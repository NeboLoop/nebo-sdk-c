#ifndef NEBO_H
#define NEBO_H

#include "tool.h"
#include "channel.h"
#include "gateway.h"
#include "ui.h"
#include "comm.h"
#include "schedule.h"
#include "types.h"
#include "schema.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Nebo App — the main entry point for C apps.
 *
 * Usage:
 *   nebo_app_t *app = nebo_app_new();
 *   nebo_app_register_tool(app, &my_tool);
 *   return nebo_app_run(app);  // blocks until SIGTERM
 */

typedef struct nebo_app nebo_app_t;

/** Create a new Nebo app. Reads the NEBO_* env vars Nebo sets. */
nebo_app_t *nebo_app_new(void);

/** Register a tool capability handler. */
void nebo_app_register_tool(nebo_app_t *app, const nebo_tool_handler_t *handler);

/** Register a channel capability handler. */
void nebo_app_register_channel(nebo_app_t *app, const nebo_channel_handler_t *handler);

/** Register a gateway capability handler. */
void nebo_app_register_gateway(nebo_app_t *app, const nebo_gateway_handler_t *handler);

/** Register a UI capability handler. */
void nebo_app_register_ui(nebo_app_t *app, const nebo_ui_handler_t *handler);

/** Register a comm capability handler. */
void nebo_app_register_comm(nebo_app_t *app, const nebo_comm_handler_t *handler);

/** Register a schedule capability handler. */
void nebo_app_register_schedule(nebo_app_t *app, const nebo_schedule_handler_t *handler);

/** Set a callback for settings updates from Nebo. */
void nebo_app_on_configure(nebo_app_t *app, void (*callback)(const nebo_string_map_t *settings));

/**
 * Start the gRPC server on 127.0.0.1 (an OS-picked port), announce it on
 * stdout as "nebo-app-listening 127.0.0.1:<port>", and block until stdin
 * closes or SIGTERM/SIGINT arrives. Every call must carry
 * "authorization: Bearer <NEBO_SIDECAR_TOKEN>".
 * Returns 0 on clean shutdown, non-zero on error (including an empty
 * NEBO_SIDECAR_TOKEN or NEBO_APP_TOKEN).
 */
int nebo_app_run(nebo_app_t *app);

/** Free the app (called automatically by nebo_app_run on shutdown). */
void nebo_app_free(nebo_app_t *app);

/** Get environment variable: NEBO_APP_DIR */
const char *nebo_app_dir(const nebo_app_t *app);

/** Get environment variable: NEBO_APP_ID */
const char *nebo_app_id(const nebo_app_t *app);

/** Get environment variable: NEBO_APP_NAME */
const char *nebo_app_name(const nebo_app_t *app);

/** Get environment variable: NEBO_APP_VERSION */
const char *nebo_app_version(const nebo_app_t *app);

/** Get environment variable: NEBO_DATA_DIR (also the working directory) */
const char *nebo_app_data_dir(const nebo_app_t *app);

/** Get environment variable: NEBO_APP_TOKEN (send as "Authorization: Bearer <token>" to NEBO_API_URL) */
const char *nebo_app_token(const nebo_app_t *app);

/** Get environment variable: NEBO_API_URL (Nebo's local API, http://127.0.0.1:<port>) */
const char *nebo_app_api_url(const nebo_app_t *app);

#ifdef __cplusplus
}
#endif

#endif /* NEBO_H */
