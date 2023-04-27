static partial class BeatUpClient {
	class CellProgressBar : UnityEngine.MonoBehaviour { // MultiplayerLobbyGameServerConnectedPlayerTableCellProgressBarControllerController
		static (UnityEngine.Color bg, UnityEngine.Color fg)[] ColorScheme => new[] {
			/*LoadState.None*/ (UnityEngine.Color.clear, UnityEngine.Color.clear),
			/*LoadState.Failed*/ (new UnityEngine.Color(0.7173913f, 0.2608696f, 0.02173913f, 0.7490196f), UnityEngine.Color.clear),
			/*LoadState.Exporting*/ (new UnityEngine.Color(0.2608696f, 0.7173913f, 0.02173913f, 0.2509804f), new UnityEngine.Color(0.9130435f, 1, 0.1521739f, 0.5434783f)),
			/*LoadState.Downloading*/ (new UnityEngine.Color(0.2608696f, 0.7173913f, 0.02173913f, 0.2509804f), new UnityEngine.Color(0.4782609f, 0.6956522f, 0.02173913f, 0.5434783f)),
			/*LoadState.Loading*/ (new UnityEngine.Color(0.4782609f, 0.6956522f, 0.02173913f, 0.5434783f), new UnityEngine.Color(0.2608696f, 0.7173913f, 0.02173913f, 0.7490196f)),
			/*LoadState.Done*/ (new UnityEngine.Color(0.2608696f, 0.7173913f, 0.02173913f, 0.7490196f), UnityEngine.Color.clear),
		};

		UnityEngine.UI.Image background = null!, foreground = null!;
		UnityEngine.Color localPlayerColor;
		IConnectedPlayer? player = null;
		void OnLoadProgress(LoadProgress progress, IConnectedPlayer player) {
			if(player != this.player)
				return;
			UnityEngine.RectTransform tr = (UnityEngine.RectTransform)transform;
			float space = (progress.normalized - 1) * 104;
			tr!.sizeDelta = new UnityEngine.Vector2(space, tr.sizeDelta.y);
			tr.localPosition = new UnityEngine.Vector3(space * (progress.state == LoadState.Exporting ? .5f : -.5f), 0, 0);
			(UnityEngine.Color bg, UnityEngine.Color fg) = ColorScheme[(int)progress.state];
			background.color = (player.isMe && progress.state == LoadState.None) ? localPlayerColor : bg;
			foreground.color = fg;
		}
		void Awake() {
			playerData.onLoadProgress += OnLoadProgress;
			if(player != null)
				OnLoadProgress(playerData.progress[PlayerIndex(player)], player);
		}
		void OnDestroy() =>
			playerData.onLoadProgress -= OnLoadProgress;
		internal void SetData(IConnectedPlayer player) {
			this.player = player;
			foreground.enabled = background.enabled = true;
			OnLoadProgress(playerData.progress[PlayerIndex(player)], player);
		}
		internal static CellProgressBar Create(UnityEngine.UI.Image background) {
			UnityEngine.GameObject barObject = UnityEngine.Object.Instantiate(background.gameObject, background.transform);
			barObject.name = "BeatUpClient_Progress";
			CellProgressBar bar = barObject.AddComponent<CellProgressBar>();
			(bar.background, bar.foreground, bar.localPlayerColor) = (background, bar.GetComponent<HMUI.ImageView>(), background.color);
			return bar;
		}
	}

	[Patch(PatchType.Postfix, typeof(GameServerPlayerTableCell), nameof(GameServerPlayerTableCell.SetData))]
	public static void GameServerPlayerTableCell_SetData(IConnectedPlayer connectedPlayer, UnityEngine.UI.Image ____localPlayerBackgroundImage) =>
		(____localPlayerBackgroundImage.GetComponentInChildren<CellProgressBar>(true) ?? CellProgressBar.Create(____localPlayerBackgroundImage)).SetData(connectedPlayer);
}
