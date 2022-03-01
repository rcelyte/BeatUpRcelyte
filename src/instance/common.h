#include "../log.h"
LOG_CTX("INSTANCE")

#include "../net.h"

#define PER_PLAYER_DIFFICULTY 1
#define PER_PLAYER_MODIFIERS 1
#define LOAD_TIMEOUT 15

#define lengthof(x) (sizeof(x)/sizeof(*x))
#define indexof(a, e) ((e) - (a))
#define String_eq(a, b) ((a).length == (b).length && memcmp((a).data, (b).data, (b).length) == 0)

#define SERIALIZE_RPC(ctx, pkt, mtype, dtype, data) { \
	SERIALIZE_CUSTOM(ctx, pkt, InternalMessageType_MultiplayerSession) { \
		pkt_writeMultiplayerSessionMessageHeader(ctx, pkt, (struct MultiplayerSessionMessageHeader){ \
			.type = MultiplayerSessionMessageType_##mtype##Rpc, \
		}); \
		pkt_write##mtype##RpcHeader(ctx, pkt, (struct mtype##RpcHeader){ \
			.type = mtype##RpcType_##dtype, \
		}); \
		pkt_write##dtype(ctx, pkt, data); \
	} \
}

// TODO: explicit protocolVersion
#define SERIALIZE_MENURPC(ctx, pkt, dtype, data) SERIALIZE_RPC(ctx, pkt, Menu, dtype, data)
#define SERIALIZE_GAMEPLAYRPC(ctx, pkt, dtype, data) SERIALIZE_RPC(ctx, pkt, Gameplay, dtype, data)

#define CLEAR_BEATMAP (struct BeatmapIdentifierNetSerializable){{0}, {0}, 0}
#define CLEAR_MODIFIERS (struct GameplayModifiers){EnergyType_Bar, 0, 0, 0, EnabledObstacleType_All, 0, 0, 0, 0, 0, 0, SongSpeed_Normal, 0, 0, 0, 0, 0}
#define CLEAR_COLORSCHEME (struct ColorSchemeNetSerializable){{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
#define CLEAR_AVATARDATA (struct MultiplayerAvatarData){{0}, {0, 0, 0, 1}, {0, 0, 0, 1}, {0}, {0, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 1}, {{0, 0, 0, 1}, {0, 0, 0, 1}}, {0}, {0}, {0, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 1}, {0}, {0}, {0}}
#define CLEAR_SETTINGS (struct PlayerSpecificSettingsNetSerializable){{0}, {0}, 0, 0, 0, 0, CLEAR_COLORSCHEME}

typedef uint8_t ClientState;
enum ClientState {
	ClientState_disconnected,
	ClientState_accepted,
	ClientState_connected,
};

ENUM(uint8_t, ServerState, {
	ServerState_Lobby,
	ServerState_LoadingScene,
	ServerState_LoadingSong,
	ServerState_Game,
})

struct InstancePacket {
	uint16_t len;
	_Bool isFragmented;
	uint8_t data[NET_MAX_PKT_SIZE];
};
struct InstanceResendPacket {
	uint32_t timeStamp;
	uint16_t len;
	uint8_t data[NET_MAX_PKT_SIZE];
};
struct ReliableChannel {
	struct Ack ack;
	uint16_t outboundSequence, inboundSequence;
	uint16_t outboundWindowStart;
	struct InstanceResendPacket resend[NET_WINDOW_SIZE];
	struct InstancePacketList *backlog;
	struct InstancePacketList **backlogEnd;
};
struct ReliableUnorderedChannel {
	struct ReliableChannel base;
	_Bool earlyReceived[NET_WINDOW_SIZE];
};
struct ReliableOrderedChannel {
	struct ReliableChannel base;
	struct InstancePacket receivedPackets[NET_WINDOW_SIZE];
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
	_Bool waiting;
	struct Ping ping;
	struct Pong pong;
};

typedef void (*ChanneledHandler)(void *ctx, void *room, void *session, const uint8_t **data, const uint8_t *end, DeliveryMethod channelId);

void instance_pingpong_init(struct PingPong *pingpong);
void instance_channels_init(struct Channels *channels);
void instance_channels_free(struct Channels *channels);
void instance_send_channeled(struct PacketContext version, struct Channels *channels, const uint8_t *buf, uint32_t len, DeliveryMethod method);
void handle_Ack(struct NetSession *session, struct Channels *channels, const uint8_t **data);
void handle_Channeled(ChanneledHandler handler, struct NetContext *net, struct NetSession *session, struct Channels *channels, void *p_ctx, void *p_room, void *p_session, const uint8_t **data, const uint8_t *end, _Bool isFragmented);
void handle_Ping(struct NetContext *net, struct NetSession *session, struct PingPong *pingpong, const uint8_t **data);
float handle_Pong(struct NetContext *net, struct NetSession *session, struct PingPong *pingpong, const uint8_t **data);
void handle_MtuCheck(struct NetContext *net, struct NetSession *session, const uint8_t **data);
void try_resend(struct NetContext *net, struct NetSession *session, struct InstanceResendPacket *p, uint32_t currentTime);
void flush_ack(struct NetContext *net, struct NetSession *session, struct Ack *ack);
