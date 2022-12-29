#include "../global.h"
#include "../net.h"
#ifndef PACKETS_H
#include "../packets.h"
#endif

#define LOAD_TIMEOUT 15
#define IDLE_TIMEOUT_MS 10000
#define KICK_TIMEOUT_MS 3000

#define bitsize(e) (sizeof(e) * 8)
#define indexof(a, e) ((uintptr_t)((e) - (a)))

#define SERIALIZE_SESSION(mtype, mfield, pkt, end, ctx, ...) { \
	struct InternalMessage _msg = { \
		.type = InternalMessageType_MultiplayerSession, \
		.multiplayerSession = { \
			.type = MultiplayerSessionMessageType_##mtype, \
			.mfield = __VA_ARGS__, \
		}, \
	}; \
	bool res = pkt_serialize(&_msg, pkt, end, ctx); \
	(void)res; \
}

#define SERIALIZE_MENURPC(pkt, end, ctx, ...) SERIALIZE_SESSION(MenuRpc, menuRpc, pkt, end, ctx, __VA_ARGS__)
#define SERIALIZE_GAMEPLAYRPC(pkt, end, ctx, ...) SERIALIZE_SESSION(GameplayRpc, gameplayRpc, pkt, end, ctx, __VA_ARGS__)
#define SERIALIZE_MPCORE(pkt, end, ctx, ...) SERIALIZE_SESSION(MpCore, mpCore, pkt, end, ctx, __VA_ARGS__)
#define SERIALIZE_BEATUP(pkt, end, ctx, ...) SERIALIZE_SESSION(BeatUpMessage, beatUpMessage, pkt, end, ctx, __VA_ARGS__)

#define CLEAR_STRING (struct String){0}
#define CLEAR_LONGSTRING (struct LongString){0}
#define CLEAR_BEATMAP (struct BeatmapIdentifierNetSerializable){0}
#define CLEAR_MODIFIERS (struct GameplayModifiers){0}
#define CLEAR_COLORSCHEME (struct ColorSchemeNetSerializable){0}
#define CLEAR_AVATARDATA (struct MultiplayerAvatarData){CLEAR_STRING, {0, 0, 0, 1}, {0, 0, 0, 1}, CLEAR_STRING, {0, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 1}, {{0, 0, 0, 1}, {0, 0, 0, 1}}, CLEAR_STRING, CLEAR_STRING, {0, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 1}, CLEAR_STRING, CLEAR_STRING, CLEAR_STRING}
#define CLEAR_SETTINGS (struct PlayerSpecificSettings){0}

#define REQUIRED_MODIFIER_MASK (15 << 18) // SongSpeed

enum GameplayModifierFlags {
	GameplayModifierFlags_EnergyType_Bar = 0,
	GameplayModifierFlags_EnergyType_Battery = 1,
	GameplayModifierFlags_InstaFail = 1 << 6,
	GameplayModifierFlags_FailOnSaberClash = 1 << 7,
	GameplayModifierFlags_EnabledObstacleType_All = 0,
	GameplayModifierFlags_EnabledObstacleType_FullHeightOnly = 1 << 8,
	GameplayModifierFlags_EnabledObstacleType_NoObstacles = 2 << 8,
	GameplayModifierFlags_NoBombs = 1 << 13,
	GameplayModifierFlags_FastNotes = 1 << 14,
	GameplayModifierFlags_StrictAngles = 1 << 15,
	GameplayModifierFlags_DisappearingArrows = 1 << 16,
	GameplayModifierFlags_GhostNotes = 1 << 17,
	GameplayModifierFlags_SongSpeed_Normal = 0 << 18,
	GameplayModifierFlags_SongSpeed_Faster = 1 << 18,
	GameplayModifierFlags_SongSpeed_Slower = 2 << 18,
	GameplayModifierFlags_SongSpeed_SuperFast = 3 << 18,
	GameplayModifierFlags_NoArrows = 1 << 22,
	GameplayModifierFlags_NoFailOn0Energy = 1 << 23,
	GameplayModifierFlags_ProMode = 1 << 24,
	GameplayModifierFlags_ZenMode = 1 << 25,
	GameplayModifierFlags_SmallCubes = 1 << 26,
};

typedef uint16_t ServerState;
enum ServerState {
	// ServerState_Handshake_Accepted,
	ServerState_Lobby_Idle = 1 << 0,
	ServerState_Lobby_Entitlement = 1 << 1,
	ServerState_Lobby_Ready = 1 << 2,
	ServerState_Lobby_LongCountdown = 1 << 3,
	ServerState_Lobby_ShortCountdown = 1 << 4,
	ServerState_Lobby_Downloading = 1 << 5,
	ServerState_Countdown = ServerState_Lobby_LongCountdown | ServerState_Lobby_ShortCountdown,
	ServerState_Selected = ServerState_Lobby_Ready | ServerState_Countdown | ServerState_Lobby_Downloading,
	ServerState_Lobby = ServerState_Lobby_Idle | ServerState_Lobby_Entitlement | ServerState_Selected,
	ServerState_Synchronizing = 1 << 6,
	ServerState_Game_LoadingScene = 1 << 7,
	ServerState_Game_LoadingSong = 1 << 8,
	ServerState_Game_Gameplay = 1 << 9,
	ServerState_Game_Results = 1 << 10,
	ServerState_Game = ServerState_Game_LoadingScene | ServerState_Game_LoadingSong | ServerState_Game_Gameplay | ServerState_Game_Results,

	ServerState_Timeout = ServerState_Countdown | ServerState_Game_LoadingScene | ServerState_Game_LoadingSong | ServerState_Game_Results,
	ServerState_Connected = ServerState_Lobby | ServerState_Synchronizing | ServerState_Game,
};

struct InstancePacket {
	uint16_t len;
	bool isFragmented;
	struct FragmentedHeader fragmentHeader;
	uint8_t data[NET_MAX_PKT_SIZE];
};
struct InstanceResendPacket {
	uint32_t timeStamp;
	uint16_t len;
	uint8_t data[NET_MAX_PKT_SIZE];
};
struct ReliableChannel {
	struct Ack ack;
	bool sendAck;
	uint16_t outboundSequence, inboundSequence;
	uint16_t outboundWindowStart;
	struct InstanceResendPacket resend[NET_MAX_WINDOW_SIZE];
	struct InstancePacketList *backlog;
	struct InstancePacketList **backlogEnd;
};
struct ReliableUnorderedChannel {
	struct ReliableChannel base;
	bool earlyReceived[NET_MAX_WINDOW_SIZE];
};
struct ReliableOrderedChannel {
	struct ReliableChannel base;
	struct InstancePacket receivedPackets[NET_MAX_WINDOW_SIZE];
};
struct SequencedChannel {
	struct Ack ack;
	uint16_t outboundSequence;
	struct InstanceResendPacket resend;
};
struct IncomingFragments {
	struct IncomingFragments *next;
	uint16_t fragmentId;
	DeliveryMethod channelId;
	uint16_t count, total;
	uint32_t size;
	struct InstancePacket fragments[];
};
struct InstancePacketList {
	struct InstancePacketList *next;
	struct InstancePacket pkt;
};
struct Channels {
	struct ReliableUnorderedChannel ru;
	struct ReliableOrderedChannel ro;
	struct SequencedChannel rs;
	struct IncomingFragments *incomingFragmentsList;
};
struct PingPong {
	uint64_t lastPing;
	bool waiting;
	struct Ping ping;
	struct Pong pong;
};

typedef void (*ChanneledHandler)(void *userptr, const uint8_t **data, const uint8_t *end, DeliveryMethod channelId);

void instance_channels_init(struct Channels *channels);
void instance_channels_free(struct Channels *channels);
uint32_t instance_channels_tick(struct Channels *channels, struct NetContext *net, struct NetSession *session, uint32_t currentTime);
void instance_send_channeled(struct NetSession *session, struct Channels *channels, const uint8_t *buf, uint32_t len, DeliveryMethod method);
void handle_Ack(struct NetSession *session, struct Channels *channels, const struct Ack *ack);
void handle_Channeled(ChanneledHandler handler, void *userptr, struct NetContext *net, struct NetSession *session, struct Channels *channels, const struct NetPacketHeader *header, const uint8_t **data, const uint8_t *end);
void handle_Ping(struct NetContext *net, struct NetSession *session, struct PingPong *pingpong, struct Ping ping);
float handle_Pong(struct NetContext *net, struct NetSession *session, struct PingPong *pingpong, struct Pong pong);
void handle_MtuCheck(struct NetContext *net, struct NetSession *session, const struct MtuCheck *req);
