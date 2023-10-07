static partial class BeatUpClient {
	struct ServerConnectInfo {
		public const uint Size = 12;
		public ConnectInfo @base;
		public uint windowSize;
		public byte countdownDuration;
		public bool directDownloads, skipResults, perPlayerDifficulty, perPlayerModifiers;
		public void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
			@base.Serialize(writer);
			writer.Put((uint)windowSize);
			writer.Put((byte)countdownDuration);
			byte bits = 0;
			bits |= directDownloads ? (byte)1 : (byte)0;
			bits |= skipResults ? (byte)2 : (byte)0;
			bits |= perPlayerDifficulty ? (byte)4 : (byte)0;
			bits |= perPlayerModifiers ? (byte)8 : (byte)0;
			writer.Put((byte)bits);
		}
		public ServerConnectInfo(LiteNetLib.Utils.NetDataReader reader) {
			@base = new ConnectInfo(reader);
			windowSize = reader.GetUInt();
			countdownDuration = reader.GetByte();
			byte bits = reader.GetByte();
			directDownloads = (bits & 1) == 1;
			skipResults = (bits & 2) == 2;
			perPlayerDifficulty = (bits & 4) == 4;
			perPlayerModifiers = (bits & 8) == 8;
		}
		public ServerConnectInfo(ushort maxBlockSize, BeatUpClient_Config config) =>
			(@base, windowSize, countdownDuration, directDownloads, skipResults, perPlayerDifficulty, perPlayerModifiers) =
				(new ConnectInfo(maxBlockSize), 0, (byte)(config.CountdownDuration * 4), config.DirectDownloads, config.SkipResults, config.PerPlayerDifficulty, config.PerPlayerModifiers);
		public static ServerConnectInfo Default => new ServerConnectInfo {
			countdownDuration = (byte)(LobbyGameStateController.kShortTimerMs / 250),
		};
	}
}
