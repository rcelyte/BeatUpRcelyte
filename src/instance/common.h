#include "../log.h"
#include "../net.h"

#define LOAD_TIMEOUT 15

#define bitsize(e) (sizeof(e) * 8)
#define indexof(a, e) ((e) - (a))

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
#define SERIALIZE_BEATUP(pkt, end, ctx, ...) SERIALIZE_SESSION(BeatUpMessage, beatUpMessage, pkt, end, ctx, __VA_ARGS__)

#define CLEAR_BEATMAP (struct BeatmapIdentifierNetSerializable){{0}, {0}, 0}
#define CLEAR_MODIFIERS (struct GameplayModifiers){EnergyType_Bar, 0, 0, 0, EnabledObstacleType_All, 0, 0, 0, 0, 0, 0, SongSpeed_Normal, 0, 0, 0, 0, 0}
#define CLEAR_COLORSCHEME (struct ColorSchemeNetSerializable){{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
#define CLEAR_AVATARDATA (struct MultiplayerAvatarData){{0}, {0, 0, 0, 1}, {0, 0, 0, 1}, {0}, {0, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 1}, {{0, 0, 0, 1}, {0, 0, 0, 1}}, {0}, {0}, {0, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 1}, {0}, {0}, {0}}
#define CLEAR_SETTINGS (struct PlayerSpecificSettingsNetSerializable){{0}, {0}, 0, 0, 0, 0, CLEAR_COLORSCHEME}

typedef uint16_t ServerState;
enum ServerState {
	// ServerState_Handshake_Accepted,
	ServerState_Lobby_Idle = 1 << 0,
	ServerState_Lobby_Entitlement = 1 << 1,
	ServerState_Lobby_Ready = 1 << 2,
	ServerState_Lobby_Countdown = 1 << 3,
	ServerState_Lobby_Downloading = 1 << 4,
	ServerState_Selected = ServerState_Lobby_Ready | ServerState_Lobby_Countdown | ServerState_Lobby_Downloading,
	ServerState_Lobby = ServerState_Lobby_Idle | ServerState_Lobby_Entitlement | ServerState_Selected,
	ServerState_Synchronizing = 1 << 5,
	ServerState_Game_LoadingScene = 1 << 6,
	ServerState_Game_LoadingSong = 1 << 7,
	ServerState_Game_Gameplay = 1 << 8,
	ServerState_Game_Results = 1 << 9,
	ServerState_Game = ServerState_Game_LoadingScene | ServerState_Game_LoadingSong | ServerState_Game_Gameplay | ServerState_Game_Results,

	ServerState_Timeout = ServerState_Lobby_Countdown | ServerState_Game_LoadingScene | ServerState_Game_LoadingSong | ServerState_Game_Results,
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

typedef void (*ChanneledHandler)(void *ctx, void *room, void *session, const uint8_t **data, const uint8_t *end, DeliveryMethod channelId);

void instance_pingpong_init(struct PingPong *pingpong);
void instance_channels_init(struct Channels *channels);
void instance_channels_free(struct Channels *channels);
void instance_send_channeled(struct NetSession *session, struct Channels *channels, const uint8_t *buf, uint32_t len, DeliveryMethod method);
void handle_Ack(struct NetSession *session, struct Channels *channels, const struct Ack *ack);
void handle_Channeled(ChanneledHandler handler, struct NetContext *net, struct NetSession *session, struct Channels *channels, void *p_ctx, void *p_room, void *p_session, const struct NetPacketHeader *header, const uint8_t **data, const uint8_t *end);
void handle_Ping(struct NetContext *net, struct NetSession *session, struct PingPong *pingpong, struct Ping ping);
float handle_Pong(struct NetContext *net, struct NetSession *session, struct PingPong *pingpong, struct Pong pong);
void handle_MtuCheck(struct NetContext *net, struct NetSession *session, const struct MtuCheck *req);
void try_resend(struct NetContext *net, struct NetSession *session, struct InstanceResendPacket *p, uint32_t currentTime);
void flush_ack(struct NetContext *net, struct NetSession *session, struct Ack *ack);
