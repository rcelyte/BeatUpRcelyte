static partial class BeatUpClient {
	static bool enableCustomLevels = false;
	static void HandleConnectToServerSuccess(bool enableCustomLevels, int maxPlayerCount) {
		Log.Debug($"HandleConnectToServerSuccess(enableCustomLevels={enableCustomLevels}, maxPlayerCount={maxPlayerCount})");
		BeatUpClient.enableCustomLevels = enableCustomLevels;
		playerData = new PlayerData(maxPlayerCount);
		lobbyDifficultyPanel.Clear();
		connectInfo = ServerConnectInfo.Default;
		infoText?.SetActive(false);
	}

	[Patch(PatchType.Prefix, typeof(GameLiftConnectionManager), nameof(GameLiftConnectionManager.HandleConnectToServerSuccess))]
	static void GameLiftConnectionManager_HandleConnectToServerSuccess(string playerSessionId, GameplayServerConfiguration configuration) =>
		HandleConnectToServerSuccess(!playerSessionId.StartsWith("psess-"), configuration.maxPlayerCount); // TODO: disable customs on official

	[Patch(PatchType.Prefix, "BGNet", "MasterServerConnectionManager", "HandleConnectToServerSuccess")]
	static void MasterServerConnectionManager_HandleConnectToServerSuccess(GameplayServerConfiguration configuration) =>
		HandleConnectToServerSuccess(true, configuration.maxPlayerCount);

	[Patch(PatchType.Prefix, typeof(LevelSelectionNavigationController), nameof(LevelSelectionNavigationController.Setup))]
	public static void LevelSelectionNavigationController_Setup(ref BeatmapCharacteristicSO[] notAllowedCharacteristics, string actionButtonText, ref bool enableCustomLevels) {
		if(actionButtonText != Polyglot.Localization.Get("BUTTON_SELECT"))
			return;
		enableCustomLevels |= BeatUpClient.enableCustomLevels;
		notAllowedCharacteristics = new BeatmapCharacteristicSO[0];
	}

	[Patch(PatchType.Postfix, typeof(LiteNetLib.ReliableChannel), ".ctor")]
	public static void ReliableChannel_ctor(byte id, LiteNetLib.NetPacket ____outgoingAcks, ref LiteNetLib.ReliableChannel.PendingPacket[] ____pendingPackets, ref LiteNetLib.NetPacket[] ____receivedPackets, ref bool[] ____earlyReceived, ref int ____windowSize) {
		int windowSize = (int)connectInfo.windowSize;
		if(windowSize == 0 || windowSize == ____windowSize) {
			Log.Debug("ReliableChannel_ctor(default)");
			return;
		}
		Log.Debug($"ReliableChannel_ctor({windowSize})");
		____windowSize = windowSize;
		System.Array.Resize(ref ____pendingPackets, windowSize);
		for(int i = 1, len = ____pendingPackets.Length; i < len; ++i)
			____pendingPackets[i] = ____pendingPackets[0];
		if(____receivedPackets != null)
			System.Array.Resize(ref ____receivedPackets, windowSize);
		if(____earlyReceived != null)
			System.Array.Resize(ref ____earlyReceived, windowSize);
		typeof(LiteNetLib.NetPacket).GetConstructors()[1].Invoke(____outgoingAcks, new object[] {LiteNetLib.PacketProperty.Ack, (windowSize - 1) / 8 + 2});
		____outgoingAcks.ChannelId = id;
		Log.Debug($"ReliableChannel_ctor({windowSize}) end");
	}
}
