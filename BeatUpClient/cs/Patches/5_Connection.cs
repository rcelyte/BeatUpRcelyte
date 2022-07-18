static partial class BeatUpClient {
	static bool enableCustomLevels = false;
	[Patch(PatchType.Prefix, typeof(MasterServerConnectionManager), "HandleConnectToServerSuccess")]
	[Patch(PatchType.Prefix, typeof(GameLiftConnectionManager), "HandleConnectToServerSuccess")]
	public static void ConnectionManager_HandleConnectToServerSuccess(object __1, BeatmapLevelSelectionMask selectionMask, GameplayServerConfiguration configuration) {
		Log.Debug("ConnectionManager_HandleConnectToServerSuccess()");
		enableCustomLevels = selectionMask.songPacks.Contains("custom_levelpack_CustomLevels") && __1 is string;
		playerData = new PlayerData(configuration.maxPlayerCount);
		lobbyDifficultyPanel.Clear();
		connectInfo = null;
		infoText.SetActive(false);
		Log.Debug("ConnectionManager_HandleConnectToServerSuccess() end");
	}

	[Patch(PatchType.Postfix, typeof(MultiplayerLevelSelectionFlowCoordinator), "get_enableCustomLevels")]
	public static void MultiplayerLevelSelectionFlowCoordinator_enableCustomLevels(ref bool __result) =>
		__result |= enableCustomLevels;

	[Patch(PatchType.Postfix, typeof(MultiplayerLevelSelectionFlowCoordinator), "get_notAllowedCharacteristics")]
	public static void MultiplayerLevelSelectionFlowCoordinator_notAllowedCharacteristics(ref BeatmapCharacteristicSO[] __result) =>
		__result = new BeatmapCharacteristicSO[0];

	[Patch(PatchType.Postfix, typeof(LiteNetLib.ReliableChannel), ".ctor")]
	public static void ReliableChannel_ctor(LiteNetLib.NetPacket ____outgoingAcks, ref LiteNetLib.ReliableChannel.PendingPacket[] ____pendingPackets, ref LiteNetLib.NetPacket[] ____receivedPackets, ref bool[] ____earlyReceived, ref int ____windowSize) {
		int windowSize = (int?)connectInfo?.windowSize ?? ____windowSize;
		Log.Debug("ReliableChannel_ctor()");
		if(windowSize == ____windowSize)
			return;
		Log.Debug($"ReliableChannel_ctor({windowSize})");
		____windowSize = windowSize;
		System.Array.Resize(ref ____pendingPackets, windowSize);
		for(int i = 1, len = ____pendingPackets.Length; i < len; ++i)
			____pendingPackets[i] = ____pendingPackets[0];
		if(____receivedPackets != null)
			System.Array.Resize(ref ____receivedPackets, windowSize);
		if(____earlyReceived != null)
			System.Array.Resize(ref ____earlyReceived, windowSize);
		typeof(LiteNetLib.NetPacket).GetConstructors()[1].Invoke(____outgoingAcks, new object[] {LiteNetLib.PacketProperty.Ack, (windowSize - 1) / 8 + 2}); // TODO: test this
		Log.Debug($"ReliableChannel_ctor({windowSize}) end");
	}
}
