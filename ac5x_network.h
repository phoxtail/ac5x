
typedef enum {
    AC5X_NETWORK_PIORITY_HI,
}AC5X_NETWORK_PIORITY;

typedef void (* AC5X_NETWORK_SEND_CB)(U32 result, void *priv);
typedef void (* AC5X_NETWORK_RECV_CB)(void *msg, U32 len, void *priv);
U32 ac5x_network_init(void);
U32 ac5x_network_send(U32 id, void *msg, U32 len, AC5X_NETWORK_SEND_CB cb, void *priv);
U32 ac5x_network_new(const char *hostIp,
                     U16 hostPort,
                     const char *dspIp,
                     U16 dspPort,
                     U32 piority,
                     AC5X_NETWORK_RECV_CB cb,
                     void *priv);

