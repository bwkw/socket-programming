#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../validation/validate.h"

#define MAX_BUF 65507  // バッファの最大サイズを定義

int main(int argc, char *argv[]) {
    char* server_ip = "127.0.0.1";
    int port = 12345;
    parse_command_line_arguments(argc, argv, &server_ip, &port);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    int result = inet_pton(AF_INET, server_ip, &(server_addr.sin_addr));
    if (result == 0) {
        fprintf(stderr, "Invalid IP address: %s\n", server_ip);
        exit(EXIT_FAILURE);
    } else if (result == -1) {
        perror("inet_pton failed");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_port = htons(port);

    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1) {
        perror("socket fd failed");
        exit(EXIT_FAILURE);
    }

    // クライアントソケットをサーバーのソケットに接続
    // 接続成功時には0、エラー発生時には-1
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_BUF];
    while (fgets(buffer, MAX_BUF, stdin) != NULL) {
        // 成功した場合には、書き込まれたバイト数
        // エラー発生時には、-1
        ssize_t send_result = send(sockfd, buffer, strlen(buffer), 0);
        if (send_result == -1) {
            perror("write failed");
            exit(EXIT_FAILURE);
        }
    }

    close(sockfd);  
    return 0;
}
