#include "../net.h"

#define PER_PLAYER_DIFFICULTY 1
#define SCENE_LOAD_TIMEOUT 15
#define SONG_LOAD_TIMEOUT 30

#define lengthof(x) (sizeof(x)/sizeof(*x))
#define indexof(a, e) (((e) - (a)) / sizeof(*(a)))
#define String_eq(a, b) ((a).length == (b).length && memcmp((a).data, (b).data, (b).length) == 0)

#define SERIALIZE_RPC(pkt, mtype, dtype, data) { \
	SERIALIZE_CUSTOM(pkt, InternalMessageType_MultiplayerSession) { \
		pkt_writeMultiplayerSessionMessageHeader(pkt, (struct MultiplayerSessionMessageHeader){ \
			.type = MultiplayerSessionMessageType_##mtype##Rpc, \
		}); \
		pkt_write##mtype##RpcHeader(pkt, (struct mtype##RpcHeader){ \
			.type = mtype##RpcType_##dtype, \
		}); \
		pkt_write##dtype(pkt, data); \
	} \
}

#define SERIALIZE_MENURPC(pkt, dtype, data) SERIALIZE_RPC(pkt, Menu, dtype, data)
#define SERIALIZE_GAMEPLAYRPC(pkt, dtype, data) SERIALIZE_RPC(pkt, Gameplay, dtype, data)

typedef uint8_t ClientState;
enum ClientState {
	ClientState_disconnected,
	ClientState_accepted,
	ClientState_connected,
};

typedef uint8_t ServerState;
enum ServerState {
	ServerState_Lobby,
	ServerState_SceneLoading,
	ServerState_SongLoading,
	ServerState_Game,
};

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
	uint16_t localSeqence, remoteSequence;
	uint16_t localWindowStart, remoteWindowStart;
	struct InstanceResendPacket resend[NET_WINDOW_SIZE];
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
	uint16_t localSeqence;
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
struct Channels {
	struct ReliableUnorderedChannel ru;
	struct ReliableOrderedChannel ro;
	struct SequencedChannel rs;
	struct IncomingFragments *incomingFragmentsList;
};

typedef void (*ChanneledHandler)(void *ctx, void *room, void *session, const uint8_t **data, const uint8_t *end, DeliveryMethod channelId);

void instance_channels_init(struct Channels *channels);
void instance_send_channeled(struct Channels *channels, uint8_t *buf, uint32_t len, DeliveryMethod method);
void handle_Ack(struct Channels *channels, const uint8_t **data);
void handle_Channeled(ChanneledHandler handler, struct NetContext *net, struct NetSession *session, struct Channels *channels, void *p_ctx, void *p_room, void *p_session, const uint8_t **data, const uint8_t *end, _Bool isFragmented);
void handle_Ping(struct NetContext *net, struct NetSession *session, struct Pong *pong, const uint8_t **data);
void handle_MtuCheck(struct NetContext *net, struct NetSession *session, const uint8_t **data);
void try_resend(struct NetContext *net, struct NetSession *session, struct InstanceResendPacket *p, uint32_t currentTime);
void flush_ack(struct NetContext *net, struct NetSession *session, struct Ack *ack);
