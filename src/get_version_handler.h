#ifndef _GET_VERSION_HANDLER_
#define _GET_VERSION_HANDLER_

#include "swap_app_context.h"
#include "send_function.h"
#include "commands.h"

int get_version_handler(rate_e P1,
                        subcommand_e P2,
                        swap_app_context_t *ctx,
                        const buf_t *input,
                        SendFunction send);

#endif
