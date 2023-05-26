#include <stdio.h>       // printfを使う用の説明書
#include <stdlib.h>      // exitを使う用の説明書
#include <string.h>      // memsetを使う用の説明書
#include <unistd.h>      // closeを使う用の説明書
#include <sys/types.h>   // 型定義用の説明書
#include <sys/socket.h>  // ソケットプログラミング用の説明書
#include <netinet/in.h>  // インターネットアドレス属性定義用の説明書
#include <arpa/inet.h>   // インターネット操作を行う用の説明書
#include "../validation/validate.h"

#define MAX_BUF 65507  // 受信バッファのサイズを定義

int main(int argc, char *argv[]) {
    char* server_ip = "127.0.0.1";
    int port = 12345;
    parse_command_line_arguments(argc, argv, server_ip, &port);

    // sockaddr_inは接続先のIPアドレスやポート番号の情報を保持するための構造体
    // 初期化時には値が適当に決定される
    // struct sockaddr_in {
    //     short            sin_family;   // アドレスファミリー 16bit
    //     unsigned short   sin_port;     // ポート番号 16bit
    //     struct in_addr   sin_addr;     // IPアドレス
    //     char             sin_zero[8];  // 使われていない。ゼロで埋める。異なる種類のソケットアドレスを表現するためのsockaddrとサイズを揃える必要があったため
    // };
    struct sockaddr_in server_addr, client_addr;

    // 整数値0はC言語においてはfalse、ポインタではNULLを意味する
    memset(&server_addr, 0, sizeof(server_addr));  // 指定したメモリ領域の中身を削除(初期化しないと予期せぬ問題生じることあり)
    memset(&client_addr, 0, sizeof(client_addr));  // 指定したメモリ領域の中身を削除(初期化しないと予期せぬ問題生じることあり)

    server_addr.sin_family = AF_INET;              // IPv4インターネットプロトコルのアドレスファミリー。マクロ定数で2。

    // サーバーのIPアドレスを設定
    // ネットワークバイト（ビッグエンディアン）オーダーに変換
    // -1, 0, 1(正常値)を返す
    int result = inet_pton(AF_INET, server_ip, &(server_addr.sin_addr));
    if (result == 0) {
        fprintf(stderr, "Invalid IP address: %s\n", server_ip);
        exit(EXIT_FAILURE);
    } else if (result == -1) {
        perror("inet_pton failed");
        exit(EXIT_FAILURE);
    }

    // ネットワークを介した通信でエンディアンの違いが問題を引き起こさないようにするためにこうしてる
    // 16bitのデータを8bitずつメモリに格納することを考える
    // ビッグエンディアンとリトルエンディアンでbyteが埋まる順番が違う
    server_addr.sin_port = htons(port);  // サーバーのポート設定。ネットワークバイト（ビッグエンディアン）オーダーになってないといけないので、整数をネットワークバイトオーダー(バイナリ形式)に変換する

    // IPv4, UDPソケットタイプ, UDPプロトコル
    // IPv4アドレスを利用し、UDP通信を行うソケットの作成
    // socket()は、失敗すると-1を返す
    // 関数成功時には、新しく作成されたソケットのファイルディスクリプタが返されるが、0, 1, 2は既に標準入力、標準出力、標準エラー出力に割り当てられている
    // UNIX系のOSでは、新しいプロセスが作成されたときに0, 1, 2というファイルディスクリプタが自動的に開かれる
    // 基本的にはないが、標準入力などを閉じた場合に0が返される
    // -1, 0, 1, 2, 3(正常値)を返す
    int sockfd; 
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == -1) {
        perror("socket fd failed");
        exit(EXIT_FAILURE);
    }

    // ソケットをクローズしてからすぐに同じポート番号を再利用することを可能に
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // 第1引数はバインドしたいソケットのファイルディスクリプタ、第2引数はバインドしたいアドレス構造体のアドレス、第3引数はそのアドレス構造体のサイズ
    // 第2引数は、struct sockaddrポインタ型が必要なのでキャスト
    // リスナーソケットを特定のIPアドレス, ポートにバインドし、待機状態として設定
    // -1, 0(正常値)を返す
    int bind_result = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bind_result == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    socklen_t client_addr_len = sizeof(client_addr);  // クライアントのアドレス構造体の大きさ:16バイト
    char buffer[MAX_BUF];                             // 受信データ格納のため1024文字のバッファ
    while (1) {
        // 第1引数は、バインドしたいソケットのファイルディスクリプタ
        // 第2引数は、データを受信するためのバッファ(char型のデータを指すポインタである必要があるので一応キャスト）
        // 第3引数は、受信するデータの最大サイズ
        // 第4引数は、オプション。MSG_WAITALLフラグは、要求された量のデータが完全に揃うまで待機する
        // 第5引数は、受信したデータの送信元アドレス構造体のアドレス（struct sockaddrポインタ型が必要なのでキャスト）
        // 第6引数は、受信したデータの送信元アドレス構造体のサイズを示すアドレス
        // 受信したデータはbufferに格納され、その長さ（バイト数）はnに格納
        // 受信メッセージのバイト数(正常値), -1(エラー)を返す
        ssize_t receive_size = recvfrom(sockfd, (char *)buffer, MAX_BUF-1, MSG_WAITALL, (struct sockaddr *) &client_addr, &client_addr_len);
        if (receive_size == -1) {
            perror("recvfrom failed");
            exit(EXIT_FAILURE);
        }

        buffer[receive_size] = '\0';  // 受信したメッセージの末尾にヌル文字（'\0'）を追加
        printf("The client port is %d\n", ntohs(client_addr.sin_port));
        printf("The client IP address is %s\n", inet_ntoa(client_addr.sin_addr));
        printf("Received : %s\n", buffer); 
    }

    close(sockfd);  // ソケットのclose
    return 0;       // 正常系として0を返す
}
