#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <memory>
#include "network_socket.h"
#include "key_value_store.h"
#include "concurrent_hash_map.h"

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cerr << "./KeyValueServer <server_port> <num_buckets (optional, needs to be prime, default 19)>\n";
		return 1;
	}

    auto server_port = argv[1];
	auto num_buckets = (argc == 3) ? std::stoi(argv[2]) : 19;

	std::unique_ptr<Test::KeyValueStore<int, int>> chobj{new Test::ConcurrentHashMap<int, int>(num_buckets)};
    Network::TcpSocket tcp_sock(server_port, 0);
    tcp_sock.serverStart();
    int clfd;
    const int BUFFER_SZ = 25, DONE_SZ = 5;

    auto dataProcess = [&chobj, &tcp_sock, &BUFFER_SZ](int cl_socket) {
        char buffer[BUFFER_SZ];
        memset(buffer, 0, BUFFER_SZ);
        tcp_sock.setRecvTimeout(cl_socket);
        tcp_sock.recv_data(buffer, BUFFER_SZ, cl_socket);
        std::string api_type;
        bool ok = 0;
        std::vector<int> kv_elems;
        for (char *p = strtok(buffer, " "); p != nullptr; p = strtok(nullptr, " ")) {
            if(ok)
                kv_elems.emplace_back(atoi(p));
            else {
                ok = 1;
                api_type = p;
            }
        }
        memset(buffer, 0, BUFFER_SZ);
        strncpy(buffer, "DONE ", DONE_SZ+1);
        if(api_type == "GET") {
            int val = chobj->value_for(kv_elems.back());
            sprintf(buffer+DONE_SZ, "%d", val);
        } else if(api_type == "PUT" || api_type == "POST") {
            chobj->add_or_update_mapping(kv_elems.front(), kv_elems.back());
        } else {
            chobj->remove_mapping(kv_elems.back());
        }
        buffer[BUFFER_SZ-1] = '\0';
        std::cout << buffer << std::endl;
        tcp_sock.send_data(buffer, BUFFER_SZ, cl_socket);
    };

    while (true) {
        if((clfd = tcp_sock.serverAccept()) < 0) {
            std::cerr << "Error in accept" << std::endl;
            continue;
        }
        std::thread thobj(dataProcess, clfd);
        thobj.detach();
    }

	return 0;
}
