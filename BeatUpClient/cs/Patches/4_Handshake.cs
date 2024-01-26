static partial class BeatUpClient {
	static ServerConnectInfo connectInfo = ServerConnectInfo.Default;
	static void RefreshModifiersHeader() {
		BGLib.Polyglot.LocalizedTextMeshProUGUI? SuggestedModifiers = UnityEngine.Resources.FindObjectsOfTypeAll<GameServerPlayersTableView>()[0].transform.Find("ServerPlayersTableHeader/Labels/SuggestedModifiers")?.GetComponent<BGLib.Polyglot.LocalizedTextMeshProUGUI>();
		if(SuggestedModifiers != null)
			SuggestedModifiers.Key = connectInfo.perPlayerModifiers ? "BEATUP_SELECTED_MODIFIERS" : "SUGGESTED_MODIFIERS";
	}

	[Detour(typeof(GameLiftClientConnectionRequestHandler), nameof(GameLiftClientConnectionRequestHandler.GetConnectionMessage))]
	static void GameLiftClientConnectionRequestHandler_GetConnectionMessage(GameLiftClientConnectionRequestHandler self, LiteNetLib.Utils.NetDataWriter writer, string userId, string userName, bool isConnectionOwner) {
		Log.Debug("GameLiftClientConnectionRequestHandler_GetConnectionMessage()");
		Base(self, writer, userId, userName, isConnectionOwner);
		LiteNetLib.Utils.NetDataWriter sub = new LiteNetLib.Utils.NetDataWriter(false, (int)ServerConnectInfo.Size);
		new ServerConnectInfo(LocalBlockSize, BeatUpClient_Config.Instance).Serialize(sub);
		writer.PutVarUInt((uint)sub.Length);
		writer.Put("BeatUpClient beta1");
		writer.Put(sub.CopyData());
		Log.Debug("GameLiftClientConnectionRequestHandler_GetConnectionMessage() end");
	}

	// `windowSize` MUST be set before LiteNetLib constructs any `ReliableChannel`s
	[Detour(typeof(LiteNetLib.NetConnectAcceptPacket), nameof(LiteNetLib.NetConnectAcceptPacket.FromData))]
	static LiteNetLib.NetConnectAcceptPacket NetConnectAcceptPacket_FromData(LiteNetLib.NetPacket packet) {
		if(packet.Size == LiteNetLib.NetConnectAcceptPacket.Size + ServerConnectInfo.Size) {
			packet.Size = LiteNetLib.NetConnectAcceptPacket.Size;
			connectInfo = new ServerConnectInfo(new LiteNetLib.Utils.NetDataReader(packet.RawData, packet.Size));
			infoText?.SetActive(true);
			if(connectInfo.windowSize != 0)
				Log.Info($"Overriding window size - {connectInfo.windowSize}");
		} else if(packet.Size != LiteNetLib.NetConnectAcceptPacket.Size) {
			Log.Error($"Bad NetConnectAcceptPacket length: {packet.Size} != {LiteNetLib.NetConnectAcceptPacket.Size}");
		}
		RefreshModifiersHeader();
		return (LiteNetLib.NetConnectAcceptPacket)Base(packet);
	}

	[Detour(typeof(ConnectedPlayerManager.ConnectedPlayer), nameof(ConnectedPlayerManager.ConnectedPlayer.CreateDirectlyConnectedPlayer))]
	static ConnectedPlayerManager.ConnectedPlayer ConnectedPlayer_CreateDirectlyConnectedPlayer(ConnectedPlayerManager manager, byte connectionId, IConnection connection) {
		ConnectedPlayerManager.ConnectedPlayer result = (ConnectedPlayerManager.ConnectedPlayer)Base(manager, connectionId, connection);
		if(connectInfo.@base.protocolId != 0)
			Net.HandleConnectInfo(connectInfo.@base, result);
		return result;
	}

	[Detour(typeof(ConnectedPlayerManager), nameof(ConnectedPlayerManager.RemovePlayer))]
	static void ConnectedPlayerManager_RemovePlayer(ConnectedPlayerManager self, ConnectedPlayerManager.ConnectedPlayer player, DisconnectedReason reason) {
		Net.OnDisconnect(player);
		Base(self, player, reason);
	}
}
