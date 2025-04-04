static partial class BeatUpClient {
	const ushort LocalBlockSize = ConnectedPlayerManager.kMaxUnreliableMessageLength - 14;
	const ulong MaxDownloadBlocks = 100663354 / LocalBlockSize;
	const ulong MaxUnzippedSize = 134217728;
	const uint LocalProtocolId = 1;
	static readonly string[] SafeMods = new[] { // Some mods like to load AssetBundles from the level folder, leading to remote code execution
		"MappingExtensions",
	};

	internal static readonly HarmonyLib.Harmony harmony = new HarmonyLib.Harmony("BeatUpClient");
	internal static ILogger Log = new DefaultLogger();
	static string modVersion = "";
	static bool haveSiraUtil = false, haveSongCore = false, haveMpCore = false, haveMpEx = false;
	static readonly bool GameLiftRequired = typeof(GameLiftConnectionManager).Assembly.GetType("MasterServerConnectionManager", false) == null;
	static UnityEngine.Sprite defaultPackCover = null!;

	static UnityEngine.GameObject? infoText = null;
	static DifficultyPanel? lobbyDifficultyPanel = null;
	internal static PlayerData playerData = new PlayerData(0);

	static void IConnection_SendUnreliable(IConnection connection, LiteNetLib.Utils.NetDataWriter writer) { // avoids referencing `BGNet.Core.DeliveryMethod` for backwards compatibility
		if(connection is LiteNetLibConnectionManager.NetPeerConnection peerConn)
			peerConn.netPeer.Send(writer, LiteNetLib.DeliveryMethod.Unreliable);
	}
}
