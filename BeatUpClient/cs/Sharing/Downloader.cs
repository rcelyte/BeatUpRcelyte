using static System.Linq.Enumerable;

static partial class BeatUpClient {
	class Downloader { // TODO: implement IDisposable + cancel in Dispose()
		static class BitOperations { // adapted from musl libc and .NET Core
			static System.ReadOnlySpan<byte> DeBruijn => new byte[64] {
				0, 1, 2, 53, 3, 7, 54, 27, 4, 38, 41, 8, 34, 55, 48, 28, 62, 5, 39, 46, 44, 42, 22, 9, 24, 35, 59, 56, 49, 18, 29, 11,
				63, 52, 6, 26, 37, 40, 33, 47, 61, 45, 43, 21, 23, 58, 17, 10, 51, 25, 36, 32, 60, 20, 57, 16, 50, 31, 19, 15, 30, 14, 13, 12,
			};
			[System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.AggressiveInlining)]
			public static byte TrailingZeroCount(ulong value) =>
				System.Runtime.CompilerServices.Unsafe.AddByteOffset(ref DeBruijn.DangerousGetPinnableReference(), (System.IntPtr)((value & (ulong)-(long)value) * 0x022fdd63cc95386dLU >> 58));
		}

		struct BitTree {
			ulong[] tree;
			uint RecurseInternal(uint count, uint layer, ulong flag, ulong blockStart, ulong skip, System.Action<ulong> callback) {
				ulong layerSkip = skip >> (int)(layer * 6);
				skip -= layerSkip << (int)(layer * 6);
				if(layerSkip != 0)
					flag |= ~0LU >> (int)(64 - layerSkip);
				flag = ~flag;
				if(layer < tree[0]) {
					ulong layerStart = tree[layer];
					while(flag != 0 && count != 0) {
						ulong i = blockStart + (ulong)BitOperations.TrailingZeroCount(flag);
						count = RecurseInternal(count, layer + 1, tree[layerStart + i], i * 64, skip, callback);
						skip = 0;
						flag &= flag - 1;
					}
				} else {
					while(flag != 0 && count != 0) {
						callback(blockStart + (ulong)BitOperations.TrailingZeroCount(flag));
						--count;
						flag &= flag - 1;
					}
				}
				return count;
			}

			ulong NewInternal(ulong length, ulong totalLength, uint layer) {
				ulong layerLength = (length + 63) >> 6;
				ulong layerStart;
				if(layerLength > 1) {
					layerStart = NewInternal(layerLength, totalLength + layerLength, layer + 1);
					tree[tree[0] - layer] = layerStart;
				} else {
					tree = new ulong[layer + totalLength + 1];
					layerStart = tree[0] = layer;
				}
				ulong pad = (layerLength << 6) - length;
				if(pad != 0)
					tree[layerStart + layerLength - 1] = ~0LU << (int)(64 - pad);
				return layerStart + layerLength;
			}

			public BitTree(ulong length) {
				tree = null!;
				ulong len = NewInternal(length, 0, 1);
			}

			public uint Recurse(ulong start, uint count, System.Action<ulong> callback) =>
				RecurseInternal(count, 1, tree[tree[0]], 0, start, callback);

			public bool Set(ulong index) {
				ulong layerStart, layer = tree[0];
				bool prev = (tree[tree[layer - 1] + (index >> 6)] & (1LU << (int)(index & 63))) != 0;
				do {
					layerStart = tree[--layer];
					ulong bit = index;
					index >>= 6;
					bit -= index << 6;
					tree[layerStart + index] |= 1LU << (int)bit;
				} while(tree[layerStart + index] == ~0LU && layer != 0);
				return prev;
			}

			[System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.AggressiveInlining)]
			public bool IsFinished() =>
				tree[tree[0]] == ~0LU;
		}

		readonly ConnectedPlayerManager connectedPlayerManager;
		readonly LiteNetLib.Utils.NetDataWriter writer;
		public readonly ShareMeta meta;
		readonly int requestLength;
		System.Action<ushort>? onProgress = delegate {};
		System.Threading.Tasks.Task<byte[]?>? bufferTask = null;
		System.Threading.CancellationTokenSource bufferCTS = new System.Threading.CancellationTokenSource();
		System.Collections.Generic.Dictionary<ConnectedPlayerManager.ConnectedPlayer, uint> sources = new System.Collections.Generic.Dictionary<ConnectedPlayerManager.ConnectedPlayer, uint>();
		public Downloader(ShareInfo info, ConnectedPlayerManager.ConnectedPlayer firstPlayer) {
			if(info.meta.byteLength == 0)
				throw new System.ArgumentException("`meta.byteLength` cannot be zero");
			if(info.meta.byteLength >= 1U << 31)
				throw new System.ArgumentException("`meta.byteLength` must be representable within a 32 bit signed integer");
			connectedPlayerManager = Resolve<MultiplayerSessionManager>()?.connectedPlayerManager ?? throw new System.InvalidOperationException("Failed to resolve `ConnectedPlayerManager`");
			writer = new LiteNetLib.Utils.NetDataWriter(false, ConnectedPlayerManager.kMaxUnreliableMessageLength);
			connectedPlayerManager.Write(writer, new DataFragmentRequest(~0U).Wrap());
			(meta, requestLength) = (info.meta, writer.Length);
			Add(firstPlayer, info.offset);
		}
		async System.Threading.Tasks.Task<byte[]?> GetInternal(System.Threading.CancellationToken cancellationToken) {
			Log.Debug($"Downloading {meta.byteLength} bytes from {sources.Count} player{((sources.Count == 1) ? "" : "s")}");
			ulong blockCount = RoundUpDivide(meta.byteLength, LocalBlockSize), progress = 0;
			BitTree segmentTree = new BitTree(blockCount);
			byte[] buffer = new byte[meta.byteLength];
			ulong pos = 0;
			uint cycle = 0, idle = 0;
			bool pendingProgress = false;
			void HandleData(DataFragment packet, IConnectedPlayer player) {
				if(player is ConnectedPlayerManager.ConnectedPlayer connectedPlayer && sources.TryGetValue(connectedPlayer, out uint blockStart)) {
					ulong block = packet.offset - blockStart;
					if(block >= blockCount)
						return;
					if(packet.data.Count == 0) {
						sources.Remove(connectedPlayer);
						return;
					}
					idle = 0;
					if(segmentTree.Set(block))
						return;
					System.Array.Copy(packet.data.Array, packet.data.Offset, buffer, (int)block * LocalBlockSize, packet.data.Count);
					++progress;
					pendingProgress = true;
				}
			}
			try {
				Net.onDataFragment += HandleData;
				while(!segmentTree.IsFinished()) {
					// TODO: handle source disconnects
					// TODO: remove sources after timeout
					if(sources.Count < 1) {
						Log.Error("Download failed: No available sources");
						return null;
					}
					if(idle >= 714) {
						Log.Error("Download failed: Timeout");
						return null;
					}
					foreach((ConnectedPlayerManager.ConnectedPlayer source, uint blockStart) in sources.Select(pair => (pair.Key, pair.Value))) {
						uint count = 7 / (uint)sources.Count + 1;
						writer.Reset();
						writer.Put(((ConnectedPlayerManager.ConnectedPlayer)connectedPlayerManager.localPlayer).connectionId);
						writer.Put(source.remoteConnectionId);
						int headerEnd = writer.Length;
						for(uint i = 0; i < 4; ++i) {
							count = segmentTree.Recurse(pos, count, block => {
								pos = block;
								if(writer.Length + requestLength >= writer.Capacity) {
									IConnection_SendUnreliable(source.connection, writer);
									writer._position = headerEnd;
								}
								connectedPlayerManager.Write(writer, new DataFragmentRequest(blockStart + (uint)block).Wrap());
							});
							if(count == 0)
								break;
							pos = 0;
						}
						IConnection_SendUnreliable(source.connection, writer);
					}
					await System.Threading.Tasks.Task.Delay(20, cancellationToken);
					cancellationToken.ThrowIfCancellationRequested();
					if((++cycle & 3) == 0 && pendingProgress) {
						onProgress?.Invoke((ushort)(progress * 65535 / blockCount));
						pendingProgress = false;
					}
					++idle;
				}
			} catch(System.Exception ex) {
				Log.Error($"Download failed: {ex}");
				return null;
			} finally {
				Net.onDataFragment -= HandleData;
				onProgress = null;
			}
			if(!(await Hash256.ComputeAsync(new(buffer))).Equals(meta.hash)) {
				Log.Error("Download failed: Hash mismatch");
				return null;
			}
			return buffer;
		}
		public void Add(ConnectedPlayerManager.ConnectedPlayer player, uint blockStart) =>
			sources[player] = blockStart;
		public int Remove(ConnectedPlayerManager.ConnectedPlayer player) {
			sources.Remove(player);
			return sources.Count;
		}
		public int Remove(ConnectedPlayerManager.ConnectedPlayer player, uint blockStart) {
			if(sources.TryGetValue(player, out uint playerBlockStart) && playerBlockStart == blockStart)
				return Remove(player);
			return sources.Count;
		}
		public System.Threading.Tasks.Task<byte[]?> Fetch(System.Action<ushort>? progress, out System.Threading.CancellationTokenSource cts) {
			if(onProgress != null && progress != null)
				onProgress += progress;
			bufferTask ??= GetInternal(bufferCTS.Token);
			cts = bufferCTS;
			return bufferTask;
		}
	}
}
