#include <cx.h>
#include <os.h>

#include "check_partner.h"
#include "globals.h"
#include "swap_errors.h"
#include "reply_error.h"

// This function receive signature of
// Input should be in the form of DER serialized signature
// the length should be in [MIN_DER_SIGNATURE_LENGTH, MAX_DER_SIGNATURE_LENGTH]
int check_partner(swap_app_context_t *ctx, const command_t *cmd, SendFunction send) {
    if (cmd->data.size < MIN_DER_SIGNATURE_LENGTH || cmd->data.size > MAX_DER_SIGNATURE_LENGTH) {
        PRINTF("Error: Input buffer length don't correspond to DER length\n");

        return reply_error(ctx, INCORRECT_COMMAND_DATA, send);
    }

    if (cx_ecdsa_verify(&(ctx->ledger_public_key),
                        CX_LAST,
                        CX_SHA256,
                        ctx->sha256_digest,
                        CURVE_SIZE_BYTES,
                        cmd->data.bytes,
                        cmd->data.size) == 0) {
        PRINTF("Error: Failed to verify signature of partner data\n");

        return reply_error(ctx, SIGN_VERIFICATION_FAIL, send);
    }

    unsigned char output_buffer[2] = {0x90, 0x00};

    if (send(output_buffer, 2) < 0) {
        PRINTF("Error: send error\n");

        return -1;
    }

    ctx->state = PROVIDER_CHECKED;

    return 0;
}
