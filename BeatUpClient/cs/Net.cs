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
			ServerConnectInfo,
		}

		const byte beatUpMessageType = 101;
		const byte mpCoreMessageType = 100;

		public static event System.Action<DataFragment, IConnectedPlayer>? onDataFragment = null;
		public static event System.Action<IConnectedPlayer>? onDisconnect = null;
		internal static System.Collections.Generic.Dictionary<IConnectedPlayer, ushort> beatUpPlayers = new System.Collections.Generic.Dictionary<IConnectedPlayer, ushort>();
		const ushort blockSize = LocalBlockSize;

		public static IConnectedPlayer? GetPlayer(string userId) {
			BeatSaberConnectedPlayerManager? connectedPlayerManager = Resolve<BeatSaberMultiplayerSessionManager>()?.connectedPlayerManager;
			for(int i = 0; i < (connectedPlayerManager?.connectedPlayerCount ?? 0); ++i) {
				IConnectedPlayer player = connectedPlayerManager!.GetConnectedPlayer(i);
				if(player.userId == userId)
					return player;
			}
			return null;
		}

		public static void Send<T>(T message) where T : LiteNetLib.Utils.INetSerializable =>
			Resolve<BeatSaberMultiplayerSessionManager>()?.Send(message); // TODO: guard `!`
		public static void SendToPlayer<T>(T message, IConnectedPlayer player) where T : LiteNetLib.Utils.INetSerializable =>
			Resolve<BeatSaberMultiplayerSessionManager>()?.SendToPlayer(message, player); // TODO: guard `!`
		public static void SendUnreliableToPlayer<T>(T message, IConnectedPlayer player) where T : LiteNetLib.Utils.INetSerializable {
			BeatSaberConnectedPlayerManager? connectedPlayerManager = Resolve<BeatSaberMultiplayerSessionManager>()?.connectedPlayerManager;
			if(connectedPlayerManager?.isConnected == true && player is BeatSaberConnectedPlayer connectedPlayer)
				IConnection_SendUnreliable(connectedPlayer.connection, connectedPlayerManager.WriteOne(((BeatSaberConnectedPlayer)connectedPlayerManager.localPlayer).connectionId, connectedPlayer.remoteConnectionId, message));
			else if(message is IPoolablePacket poolable)
				poolable.Release();
		}

		public static void SetLocalProgress(LoadProgress progress) {
			playerData.UpdateLoadProgress(progress, Resolve<BeatSaberMultiplayerSessionManager>()?.connectedPlayerManager?.localPlayer);
			Send(progress.Wrap());
		}
		public static void SetLocalProgressUnreliable(LoadProgress progress) {
			BeatSaberMultiplayerSessionManager? sessionManager = Resolve<BeatSaberMultiplayerSessionManager>();
			playerData.UpdateLoadProgress(progress, sessionManager?.connectedPlayerManager?.localPlayer);
			sessionManager?.SendUnreliable(progress.Wrap());
		}

		public static void HandleConnectInfo(ConnectInfo packet, IConnectedPlayer player) {
			if(packet.blockSize != LocalBlockSize)
				return;
			beatUpPlayers[player] = packet.blockSize;
			Log.Debug($"ConnectInfo from {player}");
		}
		public static void OnConnect(IConnectedPlayer player) {
			playerData.Reset(PlayerIndex(player));
		}
		public static void OnDisconnect(IConnectedPlayer player) {
			onDisconnect?.Invoke(player);
			beatUpPlayers.Remove(player);
		}
		// static void HandleRecommendPreview(RecommendPreview packet, IConnectedPlayer player) {}

		static void HandleBeatUpPacket(LiteNetLib.Utils.NetDataReader reader, int length, IConnectedPlayer player) {
			int end = reader.Position + length;
			try {
				MessageType type = (MessageType)reader.GetByte();
				if(type != MessageType.ConnectInfo && type != MessageType.ServerConnectInfo && !beatUpPlayers.Keys.Contains(player))
					return;
				switch(type) {
					case MessageType.ConnectInfo: HandleConnectInfo(new ConnectInfo(reader), player); break;
					case MessageType.RecommendPreview: playerData.previews[PlayerIndex(player)] = new RecommendPreview(reader); break;
					case MessageType.ShareInfo: ShareTracker.OnShareInfo(new ShareInfo(reader), player); break;
					case MessageType.DataFragmentRequest: ShareProvider.OnDataFragmentRequest(new DataFragmentRequest(reader), player); break;
					case MessageType.DataFragment: onDataFragment?.Invoke(new DataFragment(reader, end), player); break;
					case MessageType.LoadProgress: playerData.UpdateLoadProgress(new LoadProgress(reader), player); break;
					case MessageType.ServerConnectInfo: {
						if((player as BeatSaberConnectedPlayer)?.remoteConnectionId != 0)
							break;
						Log.Debug("Late ServerConnectInfo received");
						connectInfo = new ServerConnectInfo(reader);
						infoText?.SetActive(true);
						RefreshModifiersHeader();
						HandleConnectInfo(connectInfo.@base, player);
						break;
					}
					default: break;
				}
			} catch(System.Exception ex) {
				Log.Critical($"Error processing BeatUpPacket: {ex}");
			} finally {
				reader.SkipBytes(end - reader.Position);
			}
		}

		static void ProcessMpPreview(PreviewBeatmapLevel preview, IConnectedPlayer player, string[]? requirements = null) {
			Log.Debug($"ProcessMpPreview(player=\"{player}\")");
			RecommendPreview current = playerData.previews[PlayerIndex(player)];
			if(preview.levelID != current.levelID || current.previewDifficultyBeatmapSets == null) // Ignore if we already have a BeatUpClient preview for this level
				playerData.previews[PlayerIndex(player)] = new RecommendPreview(preview, requirements);
		}

		internal static void HandleMpPacket(LiteNetLib.Utils.NetDataReader reader, int length, IConnectedPlayer player) {
			int end = reader.Position + length;
			try {
				if(reader.GetString() == "MpBeatmapPacket")
					ProcessMpPreview(new MpBeatmapPacket(reader, end), player);
			} catch(System.Exception ex) {
				Log.Critical($"Error processing MultiplayerCore packet: {ex}");
			}
			reader.SkipBytes(end - reader.Position);
		}

		internal static void Setup(IMenuRpcManager rpcManager, BeatSaberMultiplayerSessionManager? multiplayerSessionManager) {
			rpcManager.setIsEntitledToLevelEvent += (userId, levelId, status) =>
				playerData.UpdateLoadProgress(new LoadProgress(status), GetPlayer(userId), true);
			if(multiplayerSessionManager == null)
				return;
			multiplayerSessionManager._packetSerializer._typeRegistry[typeof(BeatUpPacket)] = beatUpMessageType;
			multiplayerSessionManager._packetSerializer._typeRegistry[typeof(MpBeatmapPacket)] = mpCoreMessageType;
			multiplayerSessionManager._packetSerializer._messsageHandlers[beatUpMessageType] = HandleBeatUpPacket;
			if(!haveMpCore)
				multiplayerSessionManager._packetSerializer._messsageHandlers[mpCoreMessageType] = HandleMpPacket;
		}
	}

	[Detour(typeof(BaseNetworkPlayerModel), nameof(BaseNetworkPlayerModel.ConnectedPlayerManagerChanged))]
	static void BaseNetworkPlayerModel_ConnectedPlayerManagerChanged(BaseNetworkPlayerModel self) {
		Base(self);
		if(self.connectedPlayerManager is BeatSaberConnectedPlayerManager connectedPlayerManager) {
			self.connectedPlayerManager.playerConnectedEvent += Net.OnConnect;
			self.connectedPlayerManager.playerDisconnectedEvent += Net.OnDisconnect;
		}
	}

	[Detour(typeof(BaseNetworkPlayerModel), nameof(BaseNetworkPlayerModel.DestroyConnectedPlayerManager))]
	static void BaseNetworkPlayerModel_DestroyConnectedPlayerManager(BaseNetworkPlayerModel self) {
		Base(self);
		if(self.connectedPlayerManager is BeatSaberConnectedPlayerManager connectedPlayerManager) {
			self.connectedPlayerManager.playerConnectedEvent -= Net.OnConnect;
			self.connectedPlayerManager.playerDisconnectedEvent -= Net.OnDisconnect;
		}
	}

	[Detour(typeof(MenuRpcManager), nameof(MenuRpcManager.SetIsEntitledToLevel))]
	static void MenuRpcManager_SetIsEntitledToLevel(MenuRpcManager self, string levelId, EntitlementsStatus entitlementStatus) {
		playerData.UpdateLoadProgress(new LoadProgress(entitlementStatus), self._multiplayerSessionManager.localPlayer, true);
		Base(self, levelId, entitlementStatus);
	}
}
