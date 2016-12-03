#include <string.h>
#include "ac5x.h"
#include "ac5x_timer.h"
#include "ac5x_network.h"
#include "ac5x_program_block.h"
#include "ac5x_config.h"
#include "ac5x_debug.h"
#include "ac5x_proprietary.h"
#include "AC5xDrv_Drv.h"

typedef enum {
    AC5X_NETWOK_TYPE_COMMAND,
    AC5X_NETWOK_TYPE_END
}AC5X_NETWOK_TYPE;

typedef enum {
    AC5X_DEVICE_RUN,
    AC5X_DEVICE_STATE_COUNT
}AC5X_DEVICE_STATE;

typedef struct {
    U32 id;
    U32 state;
    U32 programBlockIdSending;
    U32 programBlockIdSent;
    BOOL lastProgramBlock;

    U32 networkId[128]; // to-do

    U32 retrans;

    U32 downloadTimer;
}AC5X_DEVICE;

static AC5X_DEVICE s_device[AC5X_DEVICE_MAX];

static U32 download_program(AC5X_DEVICE *device, U32 blockId);

static void on_send_download(U32 result, void *data) {
    if (NULL == data) {
        return;
    }

    AC5X_DEVICE *device = (AC5X_DEVICE *)data;

    if (0 == result) { //libuv uv_udp_send_cb status
        device->programBlockIdSent = device->programBlockIdSending;
    }
}

static void on_timeout_download(void *priv) {
    if (NULL == priv) {
        return;
    }

    AC5X_DEVICE *device = (AC5X_DEVICE *)priv;
    U32 ret = download_program(device, MAX_U32);
    if (AC5X_OK != ret) {
        // to-do error proof need re-design
        ac5x_timer_stop(device->downloadTimer);
        device->downloadTimer = MAX_U32;
    }

    if (1 == device->lastProgramBlock) {
        ac5x_timer_stop(device->downloadTimer);
        device->downloadTimer = MAX_U32;
    }

    if (10 <= device->retrans) {
        ac5x_timer_stop(device->downloadTimer);
        device->downloadTimer = MAX_U32;
    }
}

static void on_boot_message(AC5X_DEVICE *device, Tac5xMiiBootPacket *packet) {
    U32 result;
    U32 messageId = MergeFieldsToLong(packet->MessageId);
    switch (messageId) {
        case AC5X_MII_MESSAGE_ID__READY_FOR_NEXT_BLOCK:
            download_program(device, MergeFieldsToLong(packet->u.ReadyForNextBlock.RequestedBlockId));
            break;
        case AC5X_MII_MESSAGE_ID__BOOT_STATUS_INFO_CORE_0:
            result = MergeFieldsToLong(packet->u.BootStatusInfo.Flags);
            if (AC5X_MII_BOOT_STATUS_FLAG__OK == result) {
                device->state = AC5X_DEVICE_RUN;
            } else {
            }

            break;
        default:
            break;
    }
}

static void on_command(AC5X_DEVICE *device, Tac5xMiiProprietaryPayload *packet) {
    //it's an echo
}

static void on_recv_command(void *msg, U32 len, void *priv) {
    if (NULL == priv) {
        return;
    }

    AC5X_DEVICE *device = (AC5X_DEVICE *)priv;
    if (AC5X_DEVICE_RUN != device->state) {
        on_boot_message(device, (Tac5xMiiBootPacket *)msg);
    } else {
        on_command(device, (Tac5xMiiProprietaryPayload *)msg);
    }
}

static U32 download_program(AC5X_DEVICE *device, U32 blockId) {
    U32 chosedId;

    // MAX_U32 means it's not a re-trans request from ac5x
    if (MAX_U32 == blockId) {
        if (MAX_U32 == device->programBlockIdSending) {
            chosedId = 0;
        } else if (device->programBlockIdSending != device->programBlockIdSent) {
            //last block is not sent successfully
            device->retrans++;
            chosedId = device->programBlockIdSending;
        } else {
            //most common case, choose next sending block
            chosedId = device->programBlockIdSending + 1;
        }
    }
    else { // re-trans request
        chosedId = blockId;
    }

    void *msg = NULL;
    U32 len = 0, last = 0;
    if (0 == ac5x_program_block_get(chosedId, (void **)&msg, (U32 *)&len, &last)) {
        if (0 == ac5x_network_send(device->networkId[AC5X_NETWOK_TYPE_COMMAND], msg, len, on_send_download, (void *)device)) {
            if (MAX_U32 == blockId) {
                if (MAX_U32 == device->programBlockIdSending) {
                    device->downloadTimer = ac5x_timer_start(10, 10, on_timeout_download, (void *)device);
                    if (MAX_U32 == device->downloadTimer) {
                        //to-do error proof
                        return AC5X_FAIL;
                    }
                }

                device->programBlockIdSending = chosedId;
                device->lastProgramBlock = last;
            }

            //to-do error proof
            AC5X_DEBUG("");
        }
    }

    return AC5X_OK;
}

static void on_send_command(U32 result, void *data) {
    if (NULL == data) {
        return;
    }

    AC5X_DEVICE *device = (AC5X_DEVICE *)data;

    if (0 == result) { //libuv uv_udp_send_cb status
        device->programBlockIdSent = device->programBlockIdSending;
    }
}

U32 ac5x_device_init(U32 deviceId) {
    AC5X_CONFIG *config = ac5x_config_get(deviceId);
    if (NULL == config) {
        AC5X_DEBUG("DEVICE: Failed to get config for '%lu'", deviceId);
        return AC5X_FAIL;
    }

    AC5X_DEVICE *device = &s_device[deviceId];
    memset(device, 0, sizeof (AC5X_DEVICE));

    device->networkId[AC5X_NETWOK_TYPE_COMMAND] = ac5x_network_new(config->hostIp,
                                                                   config->commandPort,
                                                                   config->dspIp,
                                                                   config->commandPort,
                                                                   AC5X_NETWORK_PIORITY_HI,
                                                                   on_recv_command,
                                                                   (void *)device);
    if (MAX_U32 == device->networkId[AC5X_NETWOK_TYPE_COMMAND]) {
        AC5X_DEBUG("DEVICE: Failed to get create socket for deviceId '%lu', localIp '%s', dspIp '%s', commandPort '%d'",
                   deviceId,
                   config->hostIp,
                   config->dspIp,
                   config->commandPort);
        return AC5X_FAIL;
    }

    device->programBlockIdSending = MAX_U32;
    device->programBlockIdSent = MAX_U32;
    if (AC5X_OK != download_program(device, MAX_U32)) {

        return AC5X_FAIL;
    }

    ac5x_proprietary_command(deviceId, DEVICE_ID, AC5X_TX_DEVICE__MII_PORT__IPV4_HEADER);
    ac5x_proprietary_command(deviceId, DEVICE_ID, AC5X_TX_DEVICE__MII_PORT__ARP_TABLE);
    ac5x_proprietary_command(deviceId, DEVICE_ID, AC5X_TX_DEVICE__IBS__CALL_PROGRESS);
    ac5x_proprietary_command(deviceId, DEVICE_ID, AC5X_TX_DEVICE__IBS__USER_DEFINED_TONES);
    ac5x_proprietary_command(deviceId, DEVICE_ID, AC5X_TX_DEVICE__AGC);
    ac5x_proprietary_command(deviceId, DEVICE_ID, AC5X_TX_DEVICE__IBS__EXTENDED_IBS);
    ac5x_proprietary_command(deviceId, DEVICE_ID, AC5X_TX_DEVICE__OPEN);

    return AC5X_OK;
}

U32 ac5x_device_get_mac(U32 deviceId, U8 *dspMac, U8 *hostMac) {
    return AC5X_OK;
}

