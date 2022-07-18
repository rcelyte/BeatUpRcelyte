using static System.Linq.Enumerable;

static partial class BeatUpClient {
	class DifficultyPanel {
		static UnityEngine.GameObject characteristicTemplate = null!;
		static UnityEngine.GameObject difficultyTemplate = null!;
		BeatmapCharacteristicSegmentedControlController characteristicSelector;
		BeatmapDifficultySegmentedControlController difficultySelector;
		bool hideHints;
		public UnityEngine.RectTransform beatmapCharacteristic => (UnityEngine.RectTransform)characteristicSelector.transform.parent;
		public UnityEngine.RectTransform beatmapDifficulty => (UnityEngine.RectTransform)difficultySelector.transform.parent;

		static UnityEngine.GameObject Clone(UnityEngine.GameObject template) {
			UnityEngine.GameObject gameObject = UnityEngine.Object.Instantiate(template);
			UnityEngine.RectTransform tr = (UnityEngine.RectTransform)gameObject.transform;
			UnityEngine.RectTransform bg = (UnityEngine.RectTransform)tr.Find("BG");
			bg.pivot = tr.pivot = new UnityEngine.Vector2(.5f, 1);
			bg.localPosition = new UnityEngine.Vector3(0, 0, 0);
			UnityEngine.Object.DontDestroyOnLoad(gameObject);
			return gameObject;
		}
		public static void Init() {
			StandardLevelDetailView LevelDetail = UnityEngine.Resources.FindObjectsOfTypeAll<StandardLevelDetailView>()[0];
			characteristicTemplate = Clone(LevelDetail._beatmapCharacteristicSegmentedControlController.transform.parent.gameObject);
			difficultyTemplate = Clone(LevelDetail._beatmapDifficultySegmentedControlController.transform.parent.gameObject);
		}
		static void ChangeBackground(UnityEngine.RectTransform target, HMUI.ImageView newBG) {
			HMUI.ImageView bg = target.Find("BG").GetComponent<HMUI.ImageView>();
			(bg._skew, bg.color, bg.color0, bg.color1, bg._flipGradientColors) = (newBG._skew, newBG.color, newBG.color0, newBG.color1, newBG._flipGradientColors);
		}
		public DifficultyPanel(UnityEngine.Transform parent, int index, float width, HMUI.ImageView? background = null, bool hideHints = false) {
			UnityEngine.RectTransform beatmapCharacteristic = UI.CreateClone(characteristicTemplate, parent, "BeatmapCharacteristic", index);
			UnityEngine.RectTransform beatmapDifficulty = UI.CreateClone(difficultyTemplate, parent, "BeatmapDifficulty", index + 1);
			if(background != null) {
				ChangeBackground(beatmapCharacteristic, background);
				ChangeBackground(beatmapDifficulty, background);
			}
			beatmapCharacteristic.sizeDelta = new UnityEngine.Vector2(width, beatmapCharacteristic.sizeDelta.y);
			beatmapDifficulty.sizeDelta = new UnityEngine.Vector2(width, beatmapDifficulty.sizeDelta.y);
			characteristicSelector = beatmapCharacteristic.GetComponentInChildren<BeatmapCharacteristicSegmentedControlController>();
			difficultySelector = beatmapDifficulty.GetComponentInChildren<BeatmapDifficultySegmentedControlController>();
			characteristicSelector._segmentedControl._container = new Zenject.DiContainer();
			characteristicSelector._segmentedControl._container.Bind<HMUI.HoverHintController>().FromInstance(UnityEngine.Resources.FindObjectsOfTypeAll<HMUI.HoverHintController>()[0]).AsSingle();
			difficultySelector._difficultySegmentedControl._container = new Zenject.DiContainer();
			this.hideHints = hideHints;
		}
		public bool Clear() {
			characteristicSelector.transform.parent.gameObject.SetActive(false);
			difficultySelector.transform.parent.gameObject.SetActive(false);
			return false;
		}
		public bool Update(PreviewDifficultyBeatmap? beatmapLevel, System.Action<PreviewDifficultyBeatmap> onChange) {
			characteristicSelector.didSelectBeatmapCharacteristicEvent = delegate {};
			difficultySelector.didSelectDifficultyEvent = delegate {};
			if(beatmapLevel == null)
				return Clear();
			System.Collections.Generic.IReadOnlyList<PreviewDifficultyBeatmapSet>? previewDifficultyBeatmapSets = beatmapLevel.beatmapLevel.previewDifficultyBeatmapSets;
			if(previewDifficultyBeatmapSets == null)
				return Clear();
			BeatmapCharacteristicSO[] beatmapCharacteristics = PreviewDifficultyBeatmapSetExtensions.GetBeatmapCharacteristics(previewDifficultyBeatmapSets.ToArray());
			for(int i = 0; i < beatmapCharacteristics.Length; ++i)
				if(beatmapCharacteristics[i] == beatmapLevel.beatmapCharacteristic)
					difficultySelector.SetData(previewDifficultyBeatmapSets[i].beatmapDifficulties.Select(diff => (IDifficultyBeatmap)new CustomDifficultyBeatmap(null, null, diff, 0, 0, 0, 0, null, null)).ToList(), beatmapLevel.beatmapDifficulty);
			characteristicSelector.SetData(beatmapCharacteristics.Select(ch => (IDifficultyBeatmapSet)new CustomDifficultyBeatmapSet(ch)).ToList(), beatmapLevel.beatmapCharacteristic);
			if(hideHints)
				foreach(HMUI.HoverHint hint in characteristicSelector.GetComponentsInChildren<HMUI.HoverHint>())
					hint.enabled = false;
			characteristicSelector.didSelectBeatmapCharacteristicEvent += (controller, beatmapCharacteristic) => {
				PreviewDifficultyBeatmapSet set = previewDifficultyBeatmapSets.First(set => set.beatmapCharacteristic == beatmapCharacteristic);
				BeatmapDifficulty closestDifficulty = set.beatmapDifficulties[0];
				foreach(BeatmapDifficulty difficulty in set.beatmapDifficulties) {
					if(beatmapLevel.beatmapDifficulty < difficulty)
						break;
					closestDifficulty = difficulty;
				}
				onChange(new PreviewDifficultyBeatmap(beatmapLevel.beatmapLevel, beatmapCharacteristic, closestDifficulty));
			};
			difficultySelector.didSelectDifficultyEvent += (controller, difficulty) =>
				onChange(new PreviewDifficultyBeatmap(beatmapLevel.beatmapLevel, beatmapLevel.beatmapCharacteristic, difficulty));
			characteristicSelector.transform.parent.gameObject.SetActive(true);
			difficultySelector.transform.parent.gameObject.SetActive(true);
			return false;
		}
	}
}
