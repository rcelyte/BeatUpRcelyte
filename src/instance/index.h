_Bool index_init();
void index_cleanup();
_Bool index_get_isopen(struct String *managerId_out, struct GameplayServerConfiguration *configuration);
struct NetSession *index_create_session(struct SS addr, struct String secret, struct String userId, struct String userName);
struct NetContext *index_get_net();
uint32_t index_get_port();
