#include "proxy.h"


int main(int argc, char** argv) {
    int connect_limit = get_connect_limit();
    _args args = convert_args(argv);

    int r_socket;
    create_proxy_socket(&r_socket, args, connect_limit);
    start_proxy_server(r_socket, connect_limit, args);

    return 0;
}
