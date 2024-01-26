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
			this.requirements = CreateArray(UpperBound(reader.GetByte(), 16), i => reader.GetString());
			this.suggestions = CreateArray(UpperBound(reader.GetByte(), 16), i => reader.GetString());
		}
		public RecommendPreview(PreviewBeatmapLevel? preview, string[]? requirements) : base(preview, false) {
			labels = CreateArray((uint)(previewDifficultyBeatmapSets?.Count() ?? 0), i =>
				CreateArray((uint)previewDifficultyBeatmapSets![(int)i].difficulties.Count(), i => string.Empty));
			(this.requirements, this.suggestions) = (requirements ?? new string[0], new string[0]);
		}
		public RecommendPreview(BeatmapLevel from) : base(from, false) { // TODO: labels
			labels = CreateArray((uint)(previewDifficultyBeatmapSets?.Count() ?? 0), i =>
				CreateArray((uint)previewDifficultyBeatmapSets![(int)i].difficulties.Count(), i => string.Empty));
			(this.requirements, this.suggestions) = (new string[0], new string[0]);
		}
	}
}
