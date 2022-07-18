using static System.Linq.Enumerable;

static partial class BeatUpClient {
	internal static class Net {
		public enum MessageType : byte {
			ConnectInfo,
			RecommendPreview,
			ShareInfo,
			DataFragmentRequest,
			DataFragment,
			LoadProgress,
		}

		const byte beatUpMessageType = 101;
		const byte mpMessageType = 100;

		public static event System.Action<ShareInfo, IConnectedPlayer>? onShareInfo = null;
		public static event System.Action<DataFragmentRequest, IConnectedPlayer>? onDataFragmentRequest = null;
		public static event System.Action<DataFragment, IConnectedPlayer>? onDataFragment = null;
		internal static System.Action<IConnectedPlayer>? onDisconnect = OnDisconnect;
		internal static System.Collections.Generic.Dictionary<IConnectedPlayer, ushort> beatUpPlayers = new System.Collections.Generic.Dictionary<IConnectedPlayer, ushort>();
		const ushort blockSize = LocalBlockSize;

		public static IConnectedPlayer? GetPlayer(string userId) {
			ConnectedPlayerManager? connectedPlayerManager = Resolve<MultiplayerSessionManager>()?.connectedPlayerManager;
			for(int i = 0; i < (connectedPlayerManager?.connectedPlayerCount ?? 0); ++i) {
				IConnectedPlayer player = connectedPlayerManager!.GetConnectedPlayer(i);
				if(player.userId == userId)
					return player;
			}
			return null;
		}

		public static void Send<T>(T message) where T : LiteNetLib.Utils.INetSerializable =>
			Resolve<MultiplayerSessionManager>()?.Send(message); // TODO: guard `!`
		public static void SendToPlayer<T>(T message, IConnectedPlayer player) where T : LiteNetLib.Utils.INetSerializable =>
			Resolve<MultiplayerSessionManager>()?.SendToPlayer(message, player); // TODO: guard `!`
		public static void SendUnreliableToPlayer<T>(T message, IConnectedPlayer player) where T : LiteNetLib.Utils.INetSerializable {
			ConnectedPlayerManager? connectedPlayerManager = Resolve<MultiplayerSessionManager>()?.connectedPlayerManager;
			if(connectedPlayerManager?.isConnected == true && player is ConnectedPlayerManager.ConnectedPlayer connectedPlayer)
				connectedPlayer.connection.Send(connectedPlayerManager.WriteOne(((ConnectedPlayerManager.ConnectedPlayer)connectedPlayerManager.localPlayer).connectionId, connectedPlayer.remoteConnectionId, message), LiteNetLib.DeliveryMethod.Unreliable);
			else if(message is IPoolablePacket poolable)
				poolable.Release();
		}

		static ConnectedPlayerManager? UpdateOwnProgress(LoadProgress progress) {
			ConnectedPlayerManager? connectedPlayerManager = Resolve<MultiplayerSessionManager>()?.connectedPlayerManager;
			playerData.UpdateLoadProgress(progress, connectedPlayerManager?.localPlayer);
			return connectedPlayerManager;
		}
		public static void SetLocalProgress(LoadProgress progress) =>
			UpdateOwnProgress(progress)?.Send(progress.Wrap());
		public static void SetLocalProgressUnreliable(LoadProgress progress) =>
			UpdateOwnProgress(progress)?.SendUnreliable(progress.Wrap());

		public static void HandleConnectInfo(ConnectInfo packet, IConnectedPlayer player) {
			if(packet.blockSize != LocalBlockSize)
				return;
			beatUpPlayers[player] = packet.blockSize;
			Log.Debug($"ConnectInfo from {player}");
		}
		static void OnDisconnect(IConnectedPlayer player) =>
			beatUpPlayers.Remove(player);
		// static void HandleRecommendPreview(RecommendPreview packet, IConnectedPlayer player) {}

		static void HandleBeatUpPacket(LiteNetLib.Utils.NetDataReader reader, int length, IConnectedPlayer player) {
			int end = reader.Position + length;
			try {
				MessageType type = (MessageType)reader.GetByte();
				--length;
				if(type != MessageType.ConnectInfo && !beatUpPlayers.Keys.Contains(player))
					return;
				switch(type) {
					case MessageType.ConnectInfo: HandleConnectInfo(new ConnectInfo(reader), player); break;
					case MessageType.RecommendPreview: playerData.previews[PlayerIndex(player)] = new RecommendPreview(reader); break;
					case MessageType.ShareInfo: onShareInfo?.Invoke(new ShareInfo(reader), player); break;
					case MessageType.DataFragmentRequest: onDataFragmentRequest?.Invoke(new DataFragmentRequest(reader), player); break;
					case MessageType.DataFragment: onDataFragment?.Invoke(new DataFragment(reader, length), player); break;
					case MessageType.LoadProgress: playerData.UpdateLoadProgress(new LoadProgress(reader), player); break;
					default: break;
				}
			} catch(System.Exception ex) {
				Log.Critical($"Error processing BeatUpPacket: {ex}");
			} finally {
				reader.SkipBytes(end - reader.Position);
			}
		}

		internal static void ProcessMpPreview(IPreviewBeatmapLevel preview, IConnectedPlayer player, string[]? requirements = null) {
			Log.Debug($"ProcessMpPreview(player=\"{player}\")");
			RecommendPreview current = playerData.previews[PlayerIndex(player)];
			if(preview.levelID != current.levelID || current.previewDifficultyBeatmapSets == null) // Ignore if we already have a BeatUpClient preview for this level
				playerData.previews[PlayerIndex(player)] = new RecommendPreview(preview, requirements);
		}

		static void HandleMpPacket(LiteNetLib.Utils.NetDataReader reader, int length, IConnectedPlayer player) {
			int end = reader.Position + length;
			try {
				if(reader.GetString() == "MpBeatmapPacket")
					ProcessMpPreview(new MpBeatmapPacket(reader), player);
			} catch(System.Exception ex) {
				Log.Critical($"Error processing MultiplayerCore packet: {ex}");
			}
			reader.SkipBytes(end - reader.Position);
		}

		internal static void Setup(IMenuRpcManager rpcManager) {
			rpcManager.setIsEntitledToLevelEvent += (userId, levelId, status) =>
				playerData.UpdateLoadProgress(new LoadProgress(status), GetPlayer(userId), true);
			MultiplayerSessionManager? multiplayerSessionManager = Resolve<MultiplayerSessionManager>();
			if(multiplayerSessionManager != null) {
				multiplayerSessionManager._packetSerializer._typeRegistry[typeof(BeatUpPacket)] = beatUpMessageType;
				multiplayerSessionManager._packetSerializer._typeRegistry[typeof(MpBeatmapPacket)] = mpMessageType;
				multiplayerSessionManager._packetSerializer._messsageHandlers[beatUpMessageType] = HandleBeatUpPacket;
				if(!haveMpCore)
					multiplayerSessionManager._packetSerializer._messsageHandlers[mpMessageType] = HandleMpPacket;
			}
		}
	}

	[Patch(PatchType.Prefix, typeof(MenuRpcManager), nameof(MenuRpcManager.SetIsEntitledToLevel))]
	public static void MenuRpcManager_SetIsEntitledToLevel(IMultiplayerSessionManager ____multiplayerSessionManager, EntitlementsStatus entitlementStatus) =>
		playerData.UpdateLoadProgress(new LoadProgress(entitlementStatus), ____multiplayerSessionManager.localPlayer, true);
}
