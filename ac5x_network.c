#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include "ac5x.h"
#include "ac5x_network.h"
#include "ac5x_debug.h"

#define MAX_NETWORK 1000

typedef struct {
    U32 id;
    uv_udp_t handle;
    struct sockaddr_in hostAddr;
    struct sockaddr_in dspAddr;
    AC5X_NETWORK_RECV_CB cb;
    void *priv;
}AC5X_NETWORK;

typedef struct {
    uv_udp_send_t req;
    U32 id;
    void * msg;
    U32 len;
    AC5X_NETWORK_SEND_CB cb;
    void *priv;
}AC5X_NETWORK_REQUEST;

static AC5X_NETWORK s_network[MAX_NETWORK];

static void init(AC5X_NETWORK *interface) {
    memset(interface, 0, sizeof(AC5X_NETWORK));
    interface->id = MAX_U32;
}

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t* buf)
{
  static char buffer[1024];

  buf->base = buffer;
  buf->len = sizeof(buffer);
}

static AC5X_NETWORK_REQUEST *new_request(U32 id, void *msg, U32 len, AC5X_NETWORK_SEND_CB cb, void *priv) {
    AC5X_NETWORK_REQUEST *req = malloc(sizeof(AC5X_NETWORK_REQUEST));
    if (NULL == req) {
        return NULL;
    }

    req->id = id;
    req->msg = msg;
    req->len = len;
    req->cb = cb;
    req->priv = priv;
    return req;
}

static void delete_request(AC5X_NETWORK_REQUEST *req) {
    free(req);
}

static void on_close(uv_handle_t* handle) {
    if (NULL != handle->data)
    {
        init((AC5X_NETWORK *)handle->data);
    }
}

static void on_send(uv_udp_send_t *req, int status) {
    AC5X_NETWORK_REQUEST *realReq = (AC5X_NETWORK_REQUEST *)req;
    if (NULL != realReq->cb) {
        realReq->cb(status, realReq->priv);
    }

    delete_request(realReq);
}

static void on_read(uv_udp_t* handle,
                    ssize_t nread,
                    const uv_buf_t* buf,
                    const struct sockaddr* addr,
                    unsigned flags) {
    AC5X_NETWORK *interface = handle->data;
    interface->cb(buf->base, (U32)nread, interface->priv);
}

U32 ac5x_network_init(void) {
    for (U32 i = 0; i < MAX_NETWORK; i++) {
        init(&s_network[i]);
    }

    return AC5X_OK;
}

U32 ac5x_network_new(const char *hostIp,
                     U16 hostPort,
                     const char *dspIp,
                     U16 dspPort,
                     U32 piority,
                     AC5X_NETWORK_RECV_CB cb,
                     void *priv) {
    AC5X_NETWORK *network = NULL;
    U32 i;

    for (i = 0; i < MAX_NETWORK; i++) {
        if (MAX_U32 == s_network[i].id) {
            break;
        }
    }

    if (MAX_NETWORK <= i) {
        return MAX_U32;
    }

    network = &s_network[i];
    network->handle.data = (void *)network;

    if (0 != uv_ip4_addr(hostIp, (int)hostPort, &network->hostAddr)) {
        return MAX_U32;
    }

    if (0 != uv_ip4_addr(dspIp, (int)dspPort, &network->dspAddr)){
        return MAX_U32;
    }

    uv_udp_init(uv_default_loop(), &network->handle);
    if (0 != uv_udp_bind(&network->handle, (const struct sockaddr*) &network->hostAddr, 0)){
        uv_close((uv_handle_t*)&network->handle, on_close);
        return MAX_U32;
    }

    if (0 != uv_udp_recv_start(&network->handle, alloc_buffer, on_read)) {
        uv_close((uv_handle_t*)&network->handle, on_close);
        return MAX_U32;
    }

    network->cb = cb;
    network->priv = priv;
    network->id = i;
    return i;
}

void ac5x_network_delete(U32 id) {
    if (MAX_NETWORK <= id) {
        return;
    }

    AC5X_NETWORK *interface = &s_network[id];
    if (MAX_U32 != interface->id) {
        uv_close((uv_handle_t*)&interface->handle, on_close);
    }
}

U32 ac5x_network_send(U32 id, void *msg, U32 len, AC5X_NETWORK_SEND_CB cb, void *priv) {
    if (MAX_NETWORK <= id) {
        AC5X_DEBUG("NETWORK: Failed to send packet\n");
        return AC5X_FAIL;
    }

    AC5X_NETWORK *network = &s_network[id];
    if (MAX_U32 == network->id) {
        AC5X_DEBUG("NETWORK: Invalid socket\n");
        return AC5X_FAIL;
    }

    AC5X_NETWORK_REQUEST *req= new_request(id, msg, len, cb, priv);
    if (NULL == req) {
        AC5X_DEBUG("NETWORK: Failed to create request\n");
        return AC5X_FAIL;
    }

    uv_buf_t buf;
    buf.base = msg;
    buf.len = len;
    uv_udp_send((uv_udp_send_t *)req, &network->handle, &buf, 1, (const struct sockaddr*)&network->dspAddr, on_send);
    return AC5X_OK;
}

