#pragma once
#include <cstring>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fstream>

namespace Network {

static constexpr int BUFFER_BACKLOG = 50;
static constexpr int RECV_TIMEOUT_SECS = 120;

class TcpSocket {
    private:
        struct addrinfo conn_type, *res, *p;
        struct sockaddr client_info;
        struct timeval timeout;
        socklen_t client_info_sz;
        std::ofstream net_out;
        int sockfd;

    public:
        TcpSocket(char *port, bool is_client) : client_info_sz(sizeof(client_info)), 
                  net_out(std::string("network_output_").append(is_client ? "client" :
                  "server").append(".txt"), std::ios::out) {
            getAddrInfo(port, is_client);
            timeout.tv_sec = RECV_TIMEOUT_SECS;
            timeout.tv_usec = 0;
        }
        
        void getAddrInfo(char *port, bool is_client) {
            memset(&conn_type, 0, sizeof(conn_type));
            conn_type.ai_family = AF_UNSPEC;
            conn_type.ai_socktype = SOCK_STREAM;
            conn_type.ai_flags = AI_PASSIVE;
            const char *host_ip = (is_client ? "localhost" : nullptr); 

            if (auto status = getaddrinfo(host_ip, port, &conn_type, &res); status != 0) {
                net_out << "Error in getting address info, error: "
                          << gai_strerror(status) << std::endl;
                exit(1);
            }
        }

        int getSocketVal() const {
            return sockfd;
        }

        void serverStart() {
            int opt = 1;

            for (p = res; p != nullptr; p = p->ai_next) {
                if((sockfd = socket(p->ai_family, p->ai_socktype, 
                    p->ai_protocol)) < 0) {
                    net_out << "Socket creation failed" << std::endl;
                    continue;
                }

                if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(int)) < 0) {
                    net_out << "Error in setsockopt" << std::endl;
                    continue;
                }

                if(bind(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
                    close(sockfd);
                    net_out << "Socket bind failed" << std::endl;
                    continue;
                }

                break;
            }

            freeaddrinfo(res);
            if(p == nullptr) {
                net_out << "Server failed to bind" << std::endl;
                exit(1);
            }

            if(listen(sockfd, BUFFER_BACKLOG) < 0) {
                net_out << "Issue in listen" << std::endl;
                exit(1);
            }
        }

        int serverAccept() {
            int clfd = accept(sockfd, &client_info, &client_info_sz);
            return clfd;
        }

        void clientStart() {
            for(p = res; p != nullptr; p = p->ai_next) {
                if((sockfd = socket(p->ai_family, p->ai_socktype,
                    p->ai_protocol)) < 0) {
                    net_out << "Socket creation failed" << std::endl;
                    continue;
                }

                if(connect(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
                    close(sockfd);
                    net_out << "Connect to server failed" << std::endl;
                    continue;
                }

                break;
            }

            freeaddrinfo(res);
            if(p == nullptr) {
                net_out << "Client failed to connect to server port"
                          << std::endl;
                exit(1);
            }
        }

        void setRecvTimeout(int data_sock) {
            if(setsockopt(data_sock, SOL_SOCKET, SO_RCVTIMEO,
               reinterpret_cast<char*>(&timeout), sizeof(timeout)) < 0) {
                net_out << "Error in setsockopt or timeout" << std::endl;
                exit(1);
            }
        }

        void recv_data(char *buffer, int buffer_sz, int data_sock) {
            int num_bytes = 0, data_recd = 0;
            memset(buffer, 0, buffer_sz);
            while((data_recd < buffer_sz-1) && (num_bytes = recv(data_sock, 
                   buffer+data_recd, buffer_sz-1-data_recd, 0) >= 0))
                data_recd += num_bytes;
            if(num_bytes < 0) {
                net_out << "Error in recv" << std::endl;
                exit(1);
            }
            buffer[buffer_sz-1] = '\0';
        }

        void send_data(char *buffer, int buffer_sz, int data_sock) {
            int num_bytes = 0, data_sent = 0;
            while((data_sent < buffer_sz-1) && (num_bytes = send(data_sock, 
                   buffer+data_sent, buffer_sz-1-data_sent, 0) >= 0)) {
                data_sent += num_bytes;
                net_out << "Data sent: " << num_bytes << std::endl;
            }
            if(num_bytes < 0) {
                net_out << "Error in send" << std::endl;
                exit(1);
            }
        }

        ~TcpSocket() {
            close(sockfd);
        }        
};

}
