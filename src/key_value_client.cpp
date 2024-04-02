#include <iostream>
#include <string>
#include <cstring>
#include "network_socket.h"

int main(int argc, char **argv) {
	if (argc < 4) {
		std::cerr << "./KeyValueClient <server_port> <api_type (GET/PUT/POST/DELETE)> <key_val_params (min 1 required)\n";
		return 1;
	}

    const int BUFFER_SZ = 25;

    auto server_port = argv[1];
    Network::TcpSocket tcp_sock(server_port, 1);
    tcp_sock.clientStart();

	char buffer[BUFFER_SZ];
    memset(buffer, 0, sizeof(buffer));
    int buffer_sz = 0;
    for(int i=2;i<argc;i++) {
        strncpy(buffer+buffer_sz, argv[i], strlen(argv[i]));
        buffer_sz += strlen(argv[i]);
        strcpy(buffer+buffer_sz, " ");
        buffer_sz++;
    }
    buffer[BUFFER_SZ-1] = '\0';
    tcp_sock.send_data(buffer, BUFFER_SZ, tcp_sock.getSocketVal());
    memset(buffer, 0, sizeof(buffer));
    tcp_sock.recv_data(buffer, BUFFER_SZ, tcp_sock.getSocketVal()); 
    std::cout << buffer << std::endl;
    return 0;
}
