using static System.Linq.Enumerable;

static partial class BeatUpClient {
	internal class MpBeatmapPacket : PreviewBeatmapLevel {
		public readonly string? characteristic = string.Empty;
		public readonly BeatmapDifficulty difficulty;
		public readonly System.Linq.ILookup<BeatmapDifficulty, string> requirements;
		public override void Serialize(LiteNetLib.Utils.NetDataWriter writer) { // TODO: implement all of this properly
			writer.Put("MpBeatmapPacket");
			base.Serialize(writer);
			writer.Put((string?)characteristic);
			writer.Put((uint)difficulty);
			writer.Put((byte)requirements.Count);
			foreach(System.Linq.IGrouping<BeatmapDifficulty, string> diff in requirements) {
				writer.Put((byte)diff.Key);
				writer.Put((byte)((System.Collections.Generic.ICollection<string>)diff).Count);
				foreach(string req in diff)
					writer.Put((string)req);
			}
			writer.Put((byte)0);
			writer.Put((byte)0);
		}
		public MpBeatmapPacket(LiteNetLib.Utils.NetDataReader reader, int end) : base(reader, true) {
			characteristic = reader.GetString();
			difficulty = (BeatmapDifficulty)reader.GetUInt();
			if(reader.Position == end) {
				requirements = System.Linq.Enumerable.Empty<BeatmapDifficulty>().ToLookup(d => d, d => string.Empty);
				return;
			}
			System.Collections.Generic.HashSet<string> requirementSet = new System.Collections.Generic.HashSet<string>();
			requirements = System.Linq.Enumerable.Range(0, reader.GetByte())
				.SelectMany(i => System.Linq.Enumerable.Repeat((BeatmapDifficulty)reader.GetByte(), reader.GetByte()))
				.ToLookup(diff => diff, diff => reader.GetString() ?? string.Empty);
			previewDifficultyBeatmapSets = new[] {
				new PreviewDifficultyBeatmapSet(SerializedCharacteristic(characteristic), requirements.Select(diff => diff.Key).ToArray()),
			}; // Fill in data for difficulty selector
			for(uint i = reader.GetByte() * 3u; i > 0; --i)
				reader.GetString();
			for(byte i = reader.GetByte(); i > 0; --i) {
				reader.GetByte();
				uint count = 0;
				for(uint colors = reader.GetByte() & 127u; colors != 0; colors >>= 1)
					count += (colors & 1);
				reader.SkipBytes((int)(count * 3 * sizeof(float)));
			}
		}
		// TODO: switching between `MpBeatmapPacket`s and `RecommendPreview`s is a lossy conversion
		public MpBeatmapPacket(RecommendPreview preview, PreviewDifficultyBeatmap beatmap) : base(beatmap.beatmapLevel, true) {
			(characteristic, difficulty) = (beatmap.beatmapCharacteristic.serializedName, beatmap.beatmapDifficulty);
			PreviewDifficultyBeatmapSet? set = beatmap.beatmapLevel.previewDifficultyBeatmapSets?.FirstOrDefault(set => set.beatmapCharacteristic == beatmap.beatmapCharacteristic);
			requirements = (set?.beatmapDifficulties ?? new BeatmapDifficulty[0])
				.SelectMany(diff => preview.requirements.Select(req => (diff, req)))
				.ToLookup(pair => pair.diff, pair => pair.req);
		}
	}
}
