using static System.Linq.Enumerable;

static partial class BeatUpClient {
	internal struct PlayerData {
		public struct ModifiersWeCareAbout {
			public GameplayModifiers.SongSpeed songSpeed;
			public readonly bool disappearingArrows;
			public readonly bool ghostNotes;
			public readonly bool smallCubes;
			public ModifiersWeCareAbout(GameplayModifiers src) =>
				(songSpeed, disappearingArrows, ghostNotes, smallCubes) = (src.songSpeed, src.disappearingArrows, src.ghostNotes, src.smallCubes);
		}
		public readonly RecommendPreview[] previews;
		public readonly LoadProgress[] progress;
		public readonly ModifiersWeCareAbout[] modifiers;
		public readonly ModifiersWeCareAbout[] lockedModifiers;
		public readonly System.Collections.Generic.Dictionary<string, byte> tiers;
		public event System.Action<LoadProgress, IConnectedPlayer>? onLoadProgress;
		public PlayerData(int playerCount) {
			previews = new RecommendPreview[256];
			progress = new LoadProgress[256];
			modifiers = new ModifiersWeCareAbout[256];
			lockedModifiers = new ModifiersWeCareAbout[256];
			tiers = new System.Collections.Generic.Dictionary<string, byte>();
			onLoadProgress = null;
			System.Array.Fill(previews, new RecommendPreview(null, null));
			System.Array.Fill(modifiers, new ModifiersWeCareAbout {songSpeed = (GameplayModifiers.SongSpeed)255});
			System.Array.Fill(lockedModifiers, new ModifiersWeCareAbout {songSpeed = (GameplayModifiers.SongSpeed)255});
		}
		public void Reset(byte player) {
			progress[player] = new LoadProgress();
			lockedModifiers[player].songSpeed = modifiers[player].songSpeed = (GameplayModifiers.SongSpeed)255;
		}
		public void UpdateLoadProgress(LoadProgress newProgress, IConnectedPlayer? player, bool ignoreSequence = false) {
			if(player == null)
				return;
			byte index = PlayerIndex(player);
			if(ignoreSequence)
				newProgress.sequence = progress[index].sequence;
			else if(newProgress.sequence < progress[index].sequence)
				return;
			progress[index] = newProgress;
			onLoadProgress?.Invoke(newProgress, player);
		}
		public RecommendPreview? ResolvePreview(string levelId) =>
			previews.FirstOrDefault((RecommendPreview preview) => preview.levelID == levelId);
	}

	internal static byte PlayerIndex(IConnectedPlayer? player) { // A unique persistent index
		if(player?.isMe == true)
			return 0;
		if(player is ConnectedPlayerManager.ConnectedPlayer connectedPlayer)
			return connectedPlayer.remoteConnectionId;
		return 127;
	}
}
