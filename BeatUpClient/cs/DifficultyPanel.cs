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
		public void Clear() =>
			Update(null, null!, default, null!);
		public void Update(BeatmapLevel? beatmapLevel, BeatmapCharacteristicSO characteristic, BeatmapDifficulty difficulty, System.Action<BeatmapCharacteristicSO, BeatmapDifficulty> onChange) {
			characteristicSelector.transform.parent.gameObject.SetActive(false);
			difficultySelector.transform.parent.gameObject.SetActive(false);

			characteristicSelector.didSelectBeatmapCharacteristicEvent = delegate {};
			difficultySelector.didSelectDifficultyEvent = delegate {};
			if(beatmapLevel == null)
				return;
			difficultySelector.SetData(beatmapLevel.GetDifficulties(characteristic).DefaultIfEmpty(difficulty), difficulty, BeatmapDifficultyMask.All);
			characteristicSelector.SetData(beatmapLevel.GetCharacteristics(), characteristic, new());
			if(hideHints)
				foreach(HMUI.HoverHint hint in characteristicSelector.GetComponentsInChildren<HMUI.HoverHint>())
					hint.enabled = false;
			characteristicSelector.didSelectBeatmapCharacteristicEvent += (BeatmapCharacteristicSegmentedControlController controller, BeatmapCharacteristicSO newCharacteristic) => {
				BeatmapDifficulty closestDifficulty = beatmapLevel.GetDifficulties(newCharacteristic).First();
				foreach(BeatmapDifficulty diff in beatmapLevel.GetDifficulties(newCharacteristic)) {
					if(diff > difficulty)
						break;
					closestDifficulty = diff;
				}
				onChange(newCharacteristic, closestDifficulty);
			};
			difficultySelector.didSelectDifficultyEvent += (BeatmapDifficultySegmentedControlController controller, BeatmapDifficulty newDifficulty) =>
				onChange(characteristic, newDifficulty);
			characteristicSelector.transform.parent.gameObject.SetActive(true);
			difficultySelector.transform.parent.gameObject.SetActive(true);
		}
	}
}
