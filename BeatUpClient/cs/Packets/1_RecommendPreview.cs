using static System.Linq.Enumerable;

static partial class BeatUpClient {
	internal class RecommendPreview : PreviewBeatmapLevel { // TODO: unify requirement encoding between MpBeatmapPacket and RecommendPreview
		static readonly Net.MessageType messageType = (Net.MessageType)System.Enum.Parse(typeof(Net.MessageType), "RecommendPreview");
		public readonly string[][] labels;
		public readonly string[] requirements;
		public readonly string[] suggestions;
		public override void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
			writer.Put((byte)messageType);
			base.Serialize(writer);
			foreach(string[] set in labels) {
				writer.Put((byte)set.Length);
				foreach(string label in set)
					writer.Put((string)label);
			}
			writer.Put((byte)requirements.Length);
			foreach(string requirement in requirements)
				writer.Put((string)requirement);
			writer.Put((byte)suggestions.Length);
			foreach(string suggestion in suggestions)
				writer.Put((string)suggestion);
		}
		public RecommendPreview(LiteNetLib.Utils.NetDataReader reader) : base(reader, false) {
			labels = CreateArray((uint)(previewDifficultyBeatmapSets?.Count() ?? 0), i =>
				CreateArray(UpperBound(reader.GetByte(), 5), i => reader.GetString()));
			requirements = CreateArray(UpperBound(reader.GetByte(), 16), i => reader.GetString());
			suggestions = CreateArray(UpperBound(reader.GetByte(), 16), i => reader.GetString());
		}
		public RecommendPreview(IPreviewBeatmapLevel? preview, string[]? requirements) : base(preview, false) {
			labels = CreateArray((uint)(previewDifficultyBeatmapSets?.Count() ?? 0), i =>
				CreateArray((uint)previewDifficultyBeatmapSets![(int)i].beatmapDifficulties.Count(), i => string.Empty));
			this.requirements = requirements ?? new string[0];
			suggestions = new string[0];
		}
		public RecommendPreview(CustomPreviewBeatmapLevel preview) : this((IPreviewBeatmapLevel)preview, null) { // TODO: labels
			string path = System.IO.Path.Combine(preview.customLevelPath, "Info.dat");
			if(!System.IO.File.Exists(path))
				return;
			Newtonsoft.Json.Linq.JObject info = Newtonsoft.Json.Linq.JObject.Parse(System.IO.File.ReadAllText(path));
			Newtonsoft.Json.Linq.IJEnumerable<Newtonsoft.Json.Linq.JToken> customData = Newtonsoft.Json.Linq.Extensions.Children(info["_difficultyBeatmapSets"]?.Children()["_difficultyBeatmaps"] ?? new Newtonsoft.Json.Linq.JArray())["_customData"];
			requirements = Newtonsoft.Json.Linq.Extensions.Values<string>(customData["_requirements"]).ToHashSet().ToArray();
			suggestions = Newtonsoft.Json.Linq.Extensions.Values<string>(customData["_suggestions"]).ToHashSet().ToArray();
		}
	}
}
