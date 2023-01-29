static partial class BeatUpClient {
	static ServerConnectInfo connectInfo = ServerConnectInfo.Default;

	[Detour(typeof(BasicConnectionRequestHandler), nameof(BasicConnectionRequestHandler.GetConnectionMessage))]
	static void BasicConnectionRequestHandler_GetConnectionMessage(BasicConnectionRequestHandler self, LiteNetLib.Utils.NetDataWriter writer, string userId, string userName, bool isConnectionOwner) {
		Base(self, writer, userId, userName, isConnectionOwner);
		Log.Debug("BasicConnectionRequestHandler_GetConnectionMessage()");
		LiteNetLib.Utils.NetDataWriter sub = new LiteNetLib.Utils.NetDataWriter(false, (int)ServerConnectInfo.Size);
		new ServerConnectInfo(LocalBlockSize, BeatUpClient_Config.Instance).Serialize(sub);
		writer.PutVarUInt((uint)sub.Length);
		writer.Put("BeatUpClient beta1");
		writer.Put(sub.CopyData());
		Log.Debug("BasicConnectionRequestHandler_GetConnectionMessage() end");
	}

	// `windowSize` MUST be set before LiteNetLib constructs any `ReliableChannel`s
	[Detour(typeof(LiteNetLib.NetConnectAcceptPacket), nameof(LiteNetLib.NetConnectAcceptPacket.FromData))]
	static LiteNetLib.NetConnectAcceptPacket NetConnectAcceptPacket_FromData(LiteNetLib.NetPacket packet) {
		if(packet.Size == LiteNetLib.NetConnectAcceptPacket.Size + ServerConnectInfo.Size) {
			packet.Size = LiteNetLib.NetConnectAcceptPacket.Size;
			ServerConnectInfo info = new ServerConnectInfo(new LiteNetLib.Utils.NetDataReader(packet.RawData, packet.Size));
			if(info.windowSize < 32 || info.windowSize > 512)
				return (LiteNetLib.NetConnectAcceptPacket)Base(packet);
			connectInfo = info;
			infoText.SetActive(true);
			Log.Info($"Overriding window size - {info.windowSize}");
		} else if(packet.Size != LiteNetLib.NetConnectAcceptPacket.Size) {
			Log.Error($"Bad NetConnectAcceptPacket length: {packet.Size} != {LiteNetLib.NetConnectAcceptPacket.Size}");
		}
		Polyglot.LocalizedTextMeshProUGUI? SuggestedModifiers = UnityEngine.Resources.FindObjectsOfTypeAll<GameServerPlayersTableView>()[0].transform.Find("ServerPlayersTableHeader/Labels/SuggestedModifiers")?.GetComponent<Polyglot.LocalizedTextMeshProUGUI>();
		if(SuggestedModifiers != null)
			SuggestedModifiers.Key = connectInfo.perPlayerModifiers ? "BEATUP_SELECTED_MODIFIERS" : "SUGGESTED_MODIFIERS";
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
