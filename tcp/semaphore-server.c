#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include "../validation/validate.h"

#define MAX_BUF 65507  // 受信バッファのサイズを定義
sem_t *semaphore;

int main(int argc, char *argv[]) {
    char* server_ip = "127.0.0.1";
    int port = 12345;
    parse_command_line_arguments(argc, argv, &server_ip, &port);
    printf("Server IP: %s\n", server_ip);
    printf("Server Port: %d\n\n", port);

    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

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
    // printf("%d", sockfd); 3
    // ソケットをクローズしてからすぐに同じポート番号を再利用することを可能に
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    int bind_result = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bind_result == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // sockfdで指定されたソケットを接続待ち状態に
    // SOMAXCONNはバックログキューに格納できる未処理の接続要求の最大数を示す：128
    // 0(正常), 1(異常)を返す
    if (listen(sockfd, SOMAXCONN) == -1) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    // セマフォ初期化
    // 名前付きセマフォを作成し、セマフォのポインタを返す
    // 同時接続可能数1に
    sem_unlink("/semaphore");
    semaphore = sem_open("/semaphore", O_CREAT, S_IRUSR | S_IWUSR, 1);
    if (semaphore == SEM_FAILED) {
        perror("sem_open failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        socklen_t client_addr_len = sizeof(client_addr);

        // semaphore>0: 1つ減らしてすぐにリターン。リソースがすでに利用可能
        // semaphore=0: 0より大きくなる(セマフォを開放する=sem_post())のを待つ
        sem_wait(semaphore);

        int connected_fd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        // printf("%d\n", connected_fd); 4
        if (connected_fd == -1) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        // 接続毎に新しい子プロセスの作成
        // -1: fork関数の失敗
        //  0: fork関数が呼び出されたプロセスが子プロセスであることを示す
        // >0: fork関数が呼び出されたプロセスが親プロセスであることを示す
        // 親プロセスに対しては、子プロセスのIDを返す
        // 子プロセスに対しては、0を返す
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            close(sockfd);
            char buffer[MAX_BUF];
            while (1) {
                memset(buffer, 0, sizeof(buffer));
                // >0: 正常値
                //  0: 読み取るデータがない場合(clientで^C)
                // -1: 関数実行エラー
                ssize_t receive_size = recv(connected_fd, buffer, sizeof(buffer) - 1, 0);  // 接続したソケットからデータを読み込む
                if (receive_size > 0) {
                    printf("The client port is %d\n", ntohs(client_addr.sin_port));
                    printf("The client IP address is %s\n", inet_ntoa(client_addr.sin_addr));
                    buffer[receive_size] = '\0';  // 受信したメッセージの末尾にヌル文字（'\0'）を追加
                    printf("Received : %s\n", buffer);
                    if (strcmp(buffer, "finish\n") == 0) {
                        printf("Received finish command. Closing the connection.\n");
                        break;
                    }
                } else if (receive_size == 0) {
                    printf("Connection closed by the client.\n\n");
                    break;
                } else {
                    perror("recv failed");
                }
            }

            close(connected_fd);
            // 子プロセスが接続を終了するときにセマフォを増やす
            // これにより、次のプロセスがaccept可能に
            sem_post(semaphore);
            exit(EXIT_SUCCESS);
        } else {
            // 親プロセスがクライアントとの通信を行うソケットを閉じる
            // 子プロセスがこのソケットを扱ってやり取りを行うため、親プロセスでは不要に
            close(connected_fd);
        }
    }

    close(sockfd);
    sem_close(semaphore);      // プロセスから名前付きセマフォへの参照を閉じる
    sem_unlink("/semaphore");  // 名前付きセマフォをシステムから削除
    return 0;
}
