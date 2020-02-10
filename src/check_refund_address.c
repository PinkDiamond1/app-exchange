#include "check_refund_address.h"
#include "os.h"
#include "currency_lib_calls.h"
#include "user_validate_amounts.h"
#include "globals.h"
#include "swap_errors.h"
#include "reply_error.h"
#include "parse_check_address_message.h"

int check_refund_address(
    swap_app_context_t* ctx,
    unsigned char* input_buffer, int input_buffer_length,
    SendFunction send) {
    unsigned char* config;
    unsigned char config_length;
    unsigned char* der;
    unsigned char der_length;
    unsigned char* address_parameters;
    unsigned char address_parameters_length;
    unsigned char* ticker;
    unsigned char ticker_length;
    unsigned char* application_name;
    unsigned char application_name_length;
    if (parse_check_address_message(
        input_buffer, input_buffer_length,
        &config, &config_length,
        &der, &der_length,
        &address_parameters, &address_parameters_length) == 0) {
        return reply_error(&ctx, INCORRECT_COMMAND_DATA, send);
    }
    unsigned char hash[CURVE_SIZE_BYTES];
    cx_hash_sha256(config, config_length, hash, CURVE_SIZE_BYTES);
    if (cx_ecdsa_verify(&ctx->ledger_public_key, CX_LAST, CX_SHA256, hash, CURVE_SIZE_BYTES, der, der_length) == 0) {
        PRINTF("Error: Fail to verify signature of coin config");
        return reply_error(ctx, SIGN_VERIFICATION_FAIL, send);
    }
    if (parse_coin_config(config, config_length,
                          &ticker, &ticker_length,
                          &application_name, &application_name_length,
                          &config, &config_length) == 0) {
        PRINTF("Error: Can't parse refund coin config command\n");
        return reply_error(&ctx, INCORRECT_COMMAND_DATA, send);
    }
    if (ticker_length < 3 || ticker_length > 9) {
        PRINTF("Error: Ticker length should be in [3, 9]\n");
        return reply_error(&ctx, INCORRECT_COMMAND_DATA, send);
    }
    if (application_name_length < 3 || application_name_length > 15) {
        PRINTF("Error: Application name should be in [3, 15]\n");
        return reply_error(&ctx, INCORRECT_COMMAND_DATA, send);
    }
    // Check that given ticker match current context
    if (strlen(ctx->received_transaction.currency_from) != ticker_length ||
        strncmp(ctx->received_transaction.currency_from, ticker, ticker_length) != 0) {
        PRINTF("Error: Refund ticker doesn't match configuration ticker\n");
        return reply_error(&ctx, INCORRECT_COMMAND_DATA, send);
    }
    // copy payin configuration, we will need it later
    if (config_length > sizeof(ctx->payin_coin_config)) {
        PRINTF("Error: Currency config is too big");
        return reply_error(&ctx, INCORRECT_COMMAND_DATA, send);
    }
    ctx->payin_coin_config_length = config_length;
    os_memcpy(ctx->payin_coin_config, config, ctx->payin_coin_config_length);
    PRINTF("Coin config parsed OK\n");
    // creating 0-terminated application name
    char app_name[16] = {0};
    os_memcpy(app_name, application_name, application_name_length);
    // check address
    if (check_address(
        ctx->payin_coin_config,
        ctx->payin_coin_config_length,
        address_parameters,
        address_parameters_length,
        ctx->received_transaction.currency_from,
        ctx->received_transaction.refund_address,
        ctx->received_transaction.refund_extra_id) < 0) {
        PRINTF("Error: Refund address validation failed");
        return reply_error(ctx, INVALID_ADDRESS, send);
    }
    char printable_send_amount[30] = {0};
    if (get_printable_amount(
        ctx->payin_coin_config,
        ctx->payin_coin_config_length,
        ctx->received_transaction.currency_from,
        ctx->received_transaction.amount_to_provider.bytes,
        ctx->received_transaction.amount_to_provider.size,
        printable_send_amount,
        sizeof(printable_send_amount)) < 0) {
        PRINTF("Error: Failed to get source currency printable amount");
        return reply_error(ctx, INTERNAL_ERROR, send);
    }
    return user_validate_amounts(printable_send_amount, ctx->printable_get_amount, ctx->partner.name, ctx, send);
}