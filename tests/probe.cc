/**
 * Calls ToolService.Name on <addr>, with "authorization: Bearer <token>"
 * when a token is given, and prints the resulting gRPC status code.
 * Usage: probe <ip:port> [token]
 */

#include <cstdio>
#include <grpcpp/grpcpp.h>

#include "proto/apps/v0/tool.grpc.pb.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: probe <ip:port> [token]\n");
        return 2;
    }
    auto channel = grpc::CreateChannel(argv[1], grpc::InsecureChannelCredentials());
    auto stub = apps::v0::ToolService::NewStub(channel);

    grpc::ClientContext ctx;
    if (argc > 2) ctx.AddMetadata("authorization", std::string("Bearer ") + argv[2]);
    apps::v0::Empty req;
    apps::v0::NameResponse resp;
    grpc::Status status = stub->Name(&ctx, req, &resp);
    printf("%d\n", (int)status.error_code());
    return 0;
}
