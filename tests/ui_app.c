/* Minimal UI app for the sidecar contract test. */

#include <nebo/nebo.h>

static int handle_request(const nebo_http_request_t *req, nebo_http_response_t *resp) {
    (void)req;
    resp->status_code = 204;
    return 0;
}

int main(void) {
    nebo_ui_handler_t ui = {.handle_request = handle_request};
    nebo_app_t *app = nebo_app_new();
    nebo_app_register_ui(app, &ui);
    return nebo_app_run(app);
}
