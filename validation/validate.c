#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int is_valid_ip_address(const char *ip_address)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ip_address, &(sa.sin_addr));
    return result != 0;
}

int is_valid_port(const char *port_str) {
    for (int i = 0; i < strlen(port_str); ++i) {
        if (port_str[i] < '0' || port_str[i] > '9') {
            return 0;
        }
    }
    int port = atoi(port_str);
    return port > 0 && port <= 65535;
}

void parse_command_line_arguments(int argc, char* argv[], char* server_ip, int* port) {
    int opt;

    while ((opt = getopt(argc, argv, "a:p:")) != -1) {
        switch (opt) {
            case 'a':
                server_ip = optarg;
                if (!is_valid_ip_address(server_ip)) {
                    fprintf(stderr, "Invalid IP address: %s\n", server_ip);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'p':
                if (!is_valid_port(optarg)) {
                    fprintf(stderr, "Invalid Port number. Please enter a number between 0 and 65535.\n");
                    exit(EXIT_FAILURE);
                }
                *port = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -a <IP address> -p <port number>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}
