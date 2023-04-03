static partial class BeatUpClient {
	static ServerConnectInfo connectInfo = ServerConnectInfo.Default;

	[Detour(typeof(LiteNetLibConnectionManager), nameof(LiteNetLibConnectionManager.GetConnectionMessage))]
	static LiteNetLib.Utils.NetDataWriter LiteNetLibConnectionManager_GetConnectionMessage(LiteNetLibConnectionManager self) {
		Log.Debug("LiteNetLibConnectionManager_GetConnectionMessage()");
		LiteNetLib.Utils.NetDataWriter writer = (LiteNetLib.Utils.NetDataWriter)Base(self);
		LiteNetLib.Utils.NetDataWriter sub = new LiteNetLib.Utils.NetDataWriter(false, (int)ServerConnectInfo.Size);
		new ServerConnectInfo(LocalBlockSize, BeatUpClient_Config.Instance).Serialize(sub);
		writer.PutVarUInt((uint)sub.Length);
		writer.Put("BeatUpClient beta1");
		writer.Put(sub.CopyData());
		Log.Debug("LiteNetLibConnectionManager_GetConnectionMessage() end");
		return writer;
	}

	// `windowSize` MUST be set before LiteNetLib constructs any `ReliableChannel`s
	[Detour(typeof(LiteNetLib.NetConnectAcceptPacket), nameof(LiteNetLib.NetConnectAcceptPacket.FromData))]
	static LiteNetLib.NetConnectAcceptPacket NetConnectAcceptPacket_FromData(LiteNetLib.NetPacket packet) {
		if(packet.Size == LiteNetLib.NetConnectAcceptPacket.Size + ServerConnectInfo.Size) {
			packet.Size = LiteNetLib.NetConnectAcceptPacket.Size;
			ServerConnectInfo info = new ServerConnectInfo(new LiteNetLib.Utils.NetDataReader(packet.RawData, packet.Size));
			connectInfo = info;
			infoText?.SetActive(true);
			if(info.windowSize != 0)
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
