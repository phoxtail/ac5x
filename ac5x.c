#include <string.h>
#include <uv.h>
#include "ac5x.h"
#include "ac5x_timer.h"
#include "ac5x_network.h"
#include "ac5x_program_block.h"
#include "ac5x_config.h"
#include "ac5x_device.h"
#include "ac5x_debug.h"

U32 ac5x_init(void) {
    U32 i;
    ac5x_network_init();
    ac5x_timer_init();
    ac5x_config_load();
    ac5x_program_block_init("AC5012AE3_rm.700.37");
    for (i = 0; i < AC5X_DEVICE_MAX; i++) {
        ac5x_device_init(i);
    }

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    return AC5X_OK;
}

int main(int argc, char* argv[]) {
    ac5x_init();
    return 0;
}

