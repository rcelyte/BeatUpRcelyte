#if MPCORE_SUPPORT
using static BeatUpClient;
using static System.Linq.Enumerable;

static partial class BeatUpClient_MpCore {
	class PlayersDataModel : LobbyPlayersDataModel, System.IDisposable {
		[Zenject.Inject]
		readonly MultiplayerCore.Networking.MpPacketSerializer packetSerializer = null!;
		[Zenject.Inject]
		readonly MultiplayerCore.Beatmaps.Providers.MpBeatmapLevelProvider beatmapLevelProvider = null!;

		public override void Activate() {
			packetSerializer.registeredTypes.Add(typeof(MpBeatmapPacket));
			packetSerializer.RegisterCallback<MultiplayerCore.Beatmaps.Packets.MpBeatmapPacket>(HandleMpexBeatmapPacket);
			base.Activate();
		}

		public override void Deactivate() {
			packetSerializer.UnregisterCallback<MultiplayerCore.Beatmaps.Packets.MpBeatmapPacket>();
			packetSerializer.registeredTypes.Remove(typeof(MpBeatmapPacket));
			base.Deactivate();
		}

		void HandleMpexBeatmapPacket(MultiplayerCore.Beatmaps.Packets.MpBeatmapPacket packet, IConnectedPlayer player) =>
			Net.ProcessMpPreview(beatmapLevelProvider.GetBeatmapFromPacket(packet), player, packet.requirements.Values.SelectMany(set => set).ToHashSet().ToArray());
	}
}
#endif
