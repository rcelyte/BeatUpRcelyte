static partial class BeatUpClient {
	[Patch(PatchType.Postfix, typeof(BasicConnectionRequestHandler), nameof(BasicConnectionRequestHandler.GetConnectionMessage))]
	public static void BasicConnectionRequestHandler_GetConnectionMessage(LiteNetLib.Utils.NetDataWriter writer) {
		Log.Debug("BasicConnectionRequestHandler_GetConnectionMessage()");
		LiteNetLib.Utils.NetDataWriter sub = new LiteNetLib.Utils.NetDataWriter(false, (int)ServerConnectInfo.Size);
		new ServerConnectInfo(LocalBlockSize, BeatUpClient_Config.Instance).Serialize(sub);
		writer.PutVarUInt((uint)sub.Length);
		writer.Put("BeatUpClient beta1");
		writer.Put(sub.CopyData());
		Log.Debug("BasicConnectionRequestHandler_GetConnectionMessage() end");
	}

	// `windowSize` MUST be set before LiteNetLib constructs any `ReliableChannel`s
	[Patch(PatchType.Prefix, typeof(LiteNetLib.NetConnectAcceptPacket), "FromData")]
	public static void NetConnectAcceptPacket_FromData(ref LiteNetLib.NetPacket packet) {
		if(packet.Size == LiteNetLib.NetConnectAcceptPacket.Size + ServerConnectInfo.Size) {
			packet.Size = LiteNetLib.NetConnectAcceptPacket.Size;
			ServerConnectInfo info = new ServerConnectInfo(new LiteNetLib.Utils.NetDataReader(packet.RawData, packet.Size));
			if(info.windowSize < 32 || info.windowSize > 512)
				return;
			connectInfo = info;
			infoText.SetActive(true);
			Log.Info($"Overriding window size - {info.windowSize}");
		} else if(packet.Size != LiteNetLib.NetConnectAcceptPacket.Size) {
			Log.Error($"Bad NetConnectAcceptPacket length: {packet.Size} != {LiteNetLib.NetConnectAcceptPacket.Size}");
		}
		Polyglot.LocalizedTextMeshProUGUI? SuggestedModifiers = UnityEngine.Resources.FindObjectsOfTypeAll<GameServerPlayersTableView>()[0].transform.Find("ServerPlayersTableHeader/Labels/SuggestedModifiers")?.GetComponent<Polyglot.LocalizedTextMeshProUGUI>();
		if(SuggestedModifiers == null)
			return;
		SuggestedModifiers.Key = (connectInfo?.perPlayerModifiers == true) ? "BEATUP_SELECTED_MODIFIERS" : "SUGGESTED_MODIFIERS";
	}

	[Patch(PatchType.Postfix, typeof(ConnectedPlayerManager.ConnectedPlayer), nameof(ConnectedPlayerManager.ConnectedPlayer.CreateDirectlyConnectedPlayer))]
	public static void ConnectedPlayer_CreateDirectlyConnectedPlayer(IConnectedPlayer __result) {
		if(connectInfo != null)
			Net.HandleConnectInfo(((ServerConnectInfo)connectInfo).@base, __result);
	}

	[Patch(PatchType.Prefix, typeof(ConnectedPlayerManager), "RemovePlayer")]
	public static void ConnectedPlayerManager_RemovePlayer(IConnectedPlayer player) =>
		Net.onDisconnect?.Invoke(player);
}
