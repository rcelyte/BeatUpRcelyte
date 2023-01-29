#if MPCORE_SUPPORT
using static BeatUpClient;
using static System.Linq.Enumerable;

static partial class BeatUpClient_MpCore {
	[Detour(typeof(MainSystemInit), nameof(MainSystemInit.InstallBindings))]
	static void MainSystemInit_InstallBindings(MainSystemInit self, Zenject.DiContainer container) {
		Base(self, container);
		// TODO: learn how to zenject
		Injected<MultiplayerCore.Networking.MpPacketSerializer>.Resolve(container);
		Injected<MultiplayerCore.Beatmaps.Providers.MpBeatmapLevelProvider>.Resolve(container);
	}

	[Detour(typeof(LobbyPlayersDataModel), nameof(LobbyPlayersDataModel.Activate))]
	static void LobbyPlayersDataModel_Activate(LobbyPlayersDataModel self) {
		MultiplayerCore.Networking.MpPacketSerializer packetSerializer = Resolve<MultiplayerCore.Networking.MpPacketSerializer>()!;
		packetSerializer.registeredTypes.Add(typeof(MpBeatmapPacket));
		packetSerializer.RegisterCallback<MultiplayerCore.Beatmaps.Packets.MpBeatmapPacket>(HandleMpexBeatmapPacket);
		Base(self);
	}

	[Detour(typeof(LobbyPlayersDataModel), nameof(LobbyPlayersDataModel.Deactivate))]
	static void LobbyPlayersDataModel_Deactivate(LobbyPlayersDataModel self) {
		MultiplayerCore.Networking.MpPacketSerializer packetSerializer = Resolve<MultiplayerCore.Networking.MpPacketSerializer>()!;
		packetSerializer.UnregisterCallback<MultiplayerCore.Beatmaps.Packets.MpBeatmapPacket>();
		packetSerializer.registeredTypes.Remove(typeof(MpBeatmapPacket));
		Base(self);
	}

	static void HandleMpexBeatmapPacket(MultiplayerCore.Beatmaps.Packets.MpBeatmapPacket packet, IConnectedPlayer player) {
		IPreviewBeatmapLevel preview = Resolve<MultiplayerCore.Beatmaps.Providers.MpBeatmapLevelProvider>()!.GetBeatmapFromPacket(packet);
		if(preview is MultiplayerCore.Beatmaps.Abstractions.MpBeatmapLevel mpPreview) {
			mpPreview.previewDifficultyBeatmapSets ??= mpPreview.requirements
				.Select(set => new PreviewDifficultyBeatmapSet(SerializedCharacteristic(set.Key), set.Value.Select(diff => diff.Key).ToArray()))
				.ToArray(); // Fill in data for difficulty selector
		}
		Net.ProcessMpPreview(preview, player, packet.requirements.Values.SelectMany(set => set).ToHashSet().ToArray());
	}
}
#endif
