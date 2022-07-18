static partial class BeatUpClient {
	public class MemorySpriteLoader : ISpriteAsyncLoader {
		byte[]? data;
		public MemorySpriteLoader(byte[]? data) =>
			this.data = data;
		public System.Threading.Tasks.Task<UnityEngine.Sprite?> LoadSpriteAsync(string path, System.Threading.CancellationToken cancellationToken) {
			if(data == null || data.LongLength < 1) {
				Log.Debug("Returning default sprite");
				return System.Threading.Tasks.Task.FromResult<UnityEngine.Sprite?>(null);
			}
			Log.Debug("Decoding sprite");
			UnityEngine.Texture2D texture = new UnityEngine.Texture2D(2, 2);
			UnityEngine.ImageConversion.LoadImage(texture, data);
			UnityEngine.Rect rect = new UnityEngine.Rect(0, 0, texture.width, texture.height);
			return System.Threading.Tasks.Task.FromResult<UnityEngine.Sprite?>(UnityEngine.Sprite.Create(texture, rect, new UnityEngine.Vector2(0, 0), 0.1f));
		}
	}
}
