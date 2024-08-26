#if MPCORE_SUPPORT
using static BeatUpClient;
using static System.Linq.Enumerable;

static partial class BeatUpClient_MpCore {
	[Patch(PatchType.Postfix, typeof(MainSystemInit), nameof(MainSystemInit.InstallBindings))]
	static void MainSystemInit_InstallBindings(Zenject.DiContainer container) {
		Injected<MultiplayerCore.Networking.MpPacketSerializer>.Resolve(container);
		Injected<MultiplayerCore.Beatmaps.Providers.MpBeatmapLevelProvider>.Resolve(container);
	}

	[Detour(typeof(LobbyPlayersDataModel), nameof(LobbyPlayersDataModel.Activate))]
	static void LobbyPlayersDataModel_Activate(LobbyPlayersDataModel self) {
		MultiplayerCore.Networking.MpPacketSerializer packetSerializer = Resolve<MultiplayerCore.Networking.MpPacketSerializer>()!;
		packetSerializer.registeredTypes.Add(typeof(MpBeatmapPacket));
		packetSerializer.RegisterCallback<MultiplayerCore.Beatmaps.Packets.MpBeatmapPacket>(HandleMpCoreBeatmapPacket);
		Base(self);
	}

	[Detour(typeof(LobbyPlayersDataModel), nameof(LobbyPlayersDataModel.Deactivate))]
	static void LobbyPlayersDataModel_Deactivate(LobbyPlayersDataModel self) {
		MultiplayerCore.Networking.MpPacketSerializer packetSerializer = Resolve<MultiplayerCore.Networking.MpPacketSerializer>()!;
		packetSerializer.UnregisterCallback<MultiplayerCore.Beatmaps.Packets.MpBeatmapPacket>();
		packetSerializer.registeredTypes.Remove(typeof(MpBeatmapPacket));
		Base(self);
	}

	static void HandleMpCoreBeatmapPacket(MultiplayerCore.Beatmaps.Packets.MpBeatmapPacket packet, IConnectedPlayer player) {
		LiteNetLib.Utils.NetDataWriter writer = new();
		writer.Put("MpBeatmapPacket");
		packet.Serialize(writer);
		Net.HandleMpPacket(new LiteNetLib.Utils.NetDataReader(writer.Data, 0, writer.Length), writer.Length, player);
	}
}
#endif
