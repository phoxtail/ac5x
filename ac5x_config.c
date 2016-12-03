#include <stdlib.h>
#include <string.h>
#include "ini.h"
#include "ac5x.h"
#include "ac5x_debug.h"
#include "ac5x_config.h"

static AC5X_CONFIG s_config[AC5X_DEVICE_MAX];

static int on_config(void* user, const char* section, const char* name,
                   const char* value) {
    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("device1", "type")) {
        strncpy(s_config[0].type, value, sizeof(s_config[0].type) - 1);
    } else if (MATCH("device1", "hostIp")) {
        strncpy(s_config[0].hostIp, value, sizeof(s_config[0].hostIp) - 1);
    } else if (MATCH("device1", "dspIp")) {
        strncpy(s_config[0].dspIp, value, sizeof(s_config[0].dspIp) - 1);
    } else if (MATCH("device1", "commandPort")) {
        s_config[0].commandPort = (U16)atoi(value);
    } else if (MATCH("device1", "eventPort")) {
        s_config[0].eventPort = (U16)atoi(value);
    } else if (MATCH("device1", "debugPort")) {
        s_config[0].debugPort = (U16)atoi(value);
    } else if (MATCH("device1", "mirroringPort")) {
        s_config[0].mirroringPort = (U16)atoi(value);
    } else if (MATCH("device1", "recordingPort")) {
        s_config[0].recordingPort = (U16)atoi(value);
    } else if (MATCH("device1", "programFile")) {
        strncpy(s_config[0].programFile, value, sizeof(s_config[0].programFile) - 1);
    } else {
        AC5X_DEBUG("CONFIG: Undefined supported item, name '%s', value '%s'",
                   name, value);
        return 0;
    }
    return 1;
}

static void init(AC5X_CONFIG *config) {
    if (NULL == config) {
        return;
    }

    memset(config, 0, sizeof(AC5X_CONFIG));
    config->commandPort = MAX_U16;
    config->isConfigured = FALSE;
}

static U32 check(AC5X_CONFIG *config) {
    if (NULL == config) {
        return AC5X_FAIL;
    }

    if (MAX_U16 == config->commandPort) {
        return AC5X_FAIL;
    }

    return AC5X_OK;
}

U32 ac5x_config_load(void) {
    int i;

    for (i = 0; i < AC5X_DEVICE_MAX; i++) {
        init(&s_config[i]);
    }

    if (ini_parse("ac5x.ini", on_config, NULL) < 0) {
        AC5X_DEBUG("Can't load 'ac5x.ini'\n");
        return AC5X_FAIL;
    }

    for (i = 0; i < AC5X_DEVICE_MAX; i++) {
        if (AC5X_OK == check(&s_config[i])) {
            s_config[i].isConfigured = TRUE;
            AC5X_DEBUG("CONFIG: deviceId '%d', hostIp '%s', dspIp '%s', commandPort '%d'.\n",
                       i,
                       s_config[i].hostIp,
                       s_config[i].dspIp,
                       s_config[i].commandPort);
        }
    }

    return AC5X_OK;
}

AC5X_CONFIG *ac5x_config_get(U32 deviceId) {
    if (AC5X_DEVICE_MAX <= deviceId) {
        return NULL;
    }

    if (TRUE != s_config[deviceId].isConfigured) {
        return NULL;
    }

    return &s_config[deviceId];
}

