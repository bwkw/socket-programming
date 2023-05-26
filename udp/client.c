#include <stdio.h>                // printfを使う用の説明書
#include <stdlib.h>               // exitを使う用の説明書
#include <string.h>               // memsetを使う用の説明書
#include <unistd.h>               // closeを使う用の説明書
#include <arpa/inet.h>            // インターネット操作を行う用の説明書
#include <sys/socket.h>           // ソケットプログラミング用の説明書
#include "../validation/validate.h"

#define MAX_BUF 65507              // 送信バッファのサイズを定義

int main(int argc, char *argv[]) {
    char* server_ip = "127.0.0.1";
    int port = 12345;
    parse_command_line_arguments(argc, argv, server_ip, &port);

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

    int sockfd; 
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == -1) {
        perror("socket fd failed");
        exit(EXIT_FAILURE);
    }

    char message[MAX_BUF];
    socklen_t server_addr_len = sizeof(server_addr);  // サーバーのアドレス構造体の大きさ:16バイト
    while (1) {
        printf("Enter message (up to %d characters): ", MAX_BUF - 2);  // 末尾に追加されるnull文字と改行文字のスペースを確保するため、2文字少なく 
        // ユーザーからの入力をmessage配列に読み込む
        // 成功するとポインタを返し、エラーまたはEOFに達した場合にはNULLを返す
        if (fgets(message, MAX_BUF, stdin) != NULL) {
            // 改行文字を検索し、見つかった時にその位置へのポインタを返す
            char* newline = strchr(message, '\n');
            if (newline == NULL) {
                // 改行文字が見つかるか、入力の終わりに達するまで実行される
                // fgets が message 配列に全ての入力を収めきれなかった場合でも、残りの入力を全て読み飛ばす
                // この操作により、次回の fgets 呼び出しで以前の入力が混入することを防ぐ
                int ch;
                while ((ch = getchar()) != '\n' && ch != EOF);

                fprintf(stderr, "Input was too long. Please enter %d characters message.\n", MAX_BUF - 2);
            } else {
                *newline = '\0';
                // 第1引数は、データを送信するためのソケットのファイルディスクリプタ
                // 第2引数は、送信するメッセージ
                // 第3引数は、送信するデータのバイト数
                // 第4引数は、オプション。設定なし。
                // 第5引数は、受信者のアドレス構造体のアドレス（struct sockaddrポインタ型が必要なのでキャスト）
                // 第6引数は、受信者のアドレス構造体のサイズ
                // 送信メッセージのバイト数(正常値), -1(エラー)を返す
                ssize_t send_size = sendto(sockfd, message, strlen(message), 0, (struct sockaddr*) &server_addr, server_addr_len);
                if (send_size == -1) {
                    perror("sendto failed");
                    exit(EXIT_FAILURE);
                }
            }
        } else {
            perror("fgets failed");
            exit(EXIT_FAILURE);
        }
    }

    close(sockfd);
    return 0;
}
