
typedef struct {
    U32 isConfigured;
    S8 type[128]; // to-do
    S8 hostIp[128]; // to-do
    S8 dspIp[128]; // to-do
    S8 programFile[128]; // to-do
    U16 commandPort;
    U16 eventPort;
    U16 debugPort;
    U16 mirroringPort;
    U16 recordingPort;
}AC5X_CONFIG;

U32 ac5x_config_load(void);
AC5X_CONFIG *ac5x_config_get(U32 deviceId);

