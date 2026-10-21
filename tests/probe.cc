/**
 * Calls one method on <addr> over a plaintext HTTP/2 channel (as Nebo
 * does), with "authorization: Bearer <token>" when a token is given, and
 * prints the resulting gRPC status code. ui-health also sends
 * "x-nebo-challenge: <challenge>" when one is given and then prints the
 * code and the x-nebo-proof it got back ("-" when none).
 * Usage: probe <ip:port> <tool-name|ui-health|ui-request> [token] [challenge]
 */

#include <cstdio>
#include <cstring>
#include <string>
#include <grpcpp/grpcpp.h>

#include "proto/apps/v0/tool.grpc.pb.h"
#include "proto/apps/v0/ui.grpc.pb.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: probe <ip:port> <tool-name|ui-health|ui-request> [token] [challenge]\n");
        return 2;
    }
    auto channel = grpc::CreateChannel(argv[1], grpc::InsecureChannelCredentials());

    grpc::ClientContext ctx;
    if (argc > 3 && argv[3][0]) ctx.AddMetadata("authorization", std::string("Bearer ") + argv[3]);

    grpc::Status status;
    if (strcmp(argv[2], "tool-name") == 0) {
        apps::v0::Empty req;
        apps::v0::NameResponse resp;
        status = apps::v0::ToolService::NewStub(channel)->Name(&ctx, req, &resp);
    } else if (strcmp(argv[2], "ui-health") == 0) {
        if (argc > 4) ctx.AddMetadata("x-nebo-challenge", argv[4]);
        apps::v0::HealthCheckRequest req;
        apps::v0::HealthCheckResponse resp;
        status = apps::v0::UIService::NewStub(channel)->HealthCheck(&ctx, req, &resp);
        if (argc > 4) {
            const auto &md = ctx.GetServerInitialMetadata();
            auto it = md.find("x-nebo-proof");
            std::string proof = it == md.end() ? "-" : std::string(it->second.data(), it->second.size());
            printf("%d %s\n", (int)status.error_code(), proof.c_str());
            return 0;
        }
    } else if (strcmp(argv[2], "ui-request") == 0) {
        apps::v0::HttpRequest req;
        apps::v0::HttpResponse resp;
        status = apps::v0::UIService::NewStub(channel)->HandleRequest(&ctx, req, &resp);
    } else {
        fprintf(stderr, "unknown method %s\n", argv[2]);
        return 2;
    }
    printf("%d\n", (int)status.error_code());
    return 0;
}
