static partial class BeatUpClient {
	public interface ILogger {
		void Debug(string message);
		void Info(string message);
		void Warn(string message);
		void Error(string message);
		void Critical(string message);
		void Exception(System.Exception error);
	}
	struct DefaultLogger : ILogger {
		public void Debug(string message) =>
			UnityEngine.Debug.Log("[BeatUpClient|Debug] " + message);
		public void Info(string message) =>
			UnityEngine.Debug.Log("[BeatUpClient|Info] " + message);
		public void Warn(string message) =>
			UnityEngine.Debug.Log("[BeatUpClient|Warn] " + message);
		public void Error(string message) =>
			UnityEngine.Debug.Log("[BeatUpClient|Error] " + message);
		public void Critical(string message) =>
			UnityEngine.Debug.Log("[BeatUpClient|Critical] " + message);
		public void Exception(System.Exception error) =>
			UnityEngine.Debug.Log($"[BeatUpClient|Exception] {error}");
	}
}
