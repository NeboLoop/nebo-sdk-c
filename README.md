# Nebo C SDK

Official C SDK for building [Nebo](https://neboloop.com) apps.

## Build

```bash
mkdir build && cd build
cmake ..
make
```

## Quick Start

```c
#include <nebo/nebo.h>

static int my_execute(const char *input_json, char **output, int *is_error) {
    *output = strdup("Hello from C!");
    *is_error = 0;
    return 0;
}

static const char *actions[] = {"greet", NULL};

int main(void) {
    nebo_schema_builder_t *sb = nebo_schema_new(actions);
    nebo_schema_string(sb, "name", "Who to greet", 1);
    char *schema = nebo_schema_build(sb);
    nebo_schema_free(sb);

    nebo_tool_handler_t tool = {
        .name = "greeter",
        .description = "Says hello.",
        .schema = schema,
        .execute = my_execute,
    };

    nebo_app_t *app = nebo_app_new();
    nebo_app_register_tool(app, &tool);
    int ret = nebo_app_run(app);
    free(schema);
    return ret;
}
```

## How Nebo runs your app

Nebo starts the app binary with these environment variables:

| Variable | Accessor | Meaning |
|---|---|---|
| `NEBO_APP_ID` | `nebo_app_id()` | App id from the manifest |
| `NEBO_APP_NAME` | `nebo_app_name()` | App name |
| `NEBO_APP_VERSION` | `nebo_app_version()` | App version |
| `NEBO_APP_DIR` | `nebo_app_dir()` | The app's code folder |
| `NEBO_DATA_DIR` | `nebo_app_data_dir()` | The app's persistent data folder (also the working directory) |
| `NEBO_API_URL` | `nebo_app_api_url()` | Nebo's local API, `http://127.0.0.1:<port>` |
| `NEBO_APP_TOKEN` | `nebo_app_token()` | Per-launch secret for your calls to `NEBO_API_URL` |
| `NEBO_SIDECAR_TOKEN` | (internal) | Per-launch secret Nebo sends on its calls to your app |

`nebo_app_run()` handles the rest:

- Listens on `127.0.0.1` on a port the OS picks, and prints
  `nebo-app-listening 127.0.0.1:<port>` on stdout so Nebo can connect.
- Refuses every call without `authorization: Bearer <NEBO_SIDECAR_TOKEN>`
  (`UNAUTHENTICATED`), checked once for every service before any handler
  runs, and refuses to start when `NEBO_SIDECAR_TOKEN` is empty.
  `NEBO_APP_TOKEN` is never accepted on incoming calls.
- Exits when stdin closes (Nebo is gone) or on SIGTERM/SIGINT.

When your app calls Nebo's API at `NEBO_API_URL`, send
`Authorization: Bearer <NEBO_APP_TOKEN>`.

Everything else the app prints to stdout or stderr goes to the app's
`sidecar.log`.

## Test

```bash
cd build && ctest --output-on-failure
```

Runs the calculator example and a minimal UI app the way Nebo does, and
checks the announce line, the sidecar token check (the app token is refused),
Nebo's `UIService.HealthCheck` probe, and shutdown on stdin EOF and SIGTERM.

## Documentation

See [Creating Nebo Apps](https://neboloop.com/developers) for the full guide.
