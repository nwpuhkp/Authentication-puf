// 设备端代码
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> // 引入套接字头文件
#include <netinet/in.h> // 引入地址结构体头文件

// 假设使用SRAM PUF
#define CHALLENGE_LEN 8 // 质询长度
#define RESPONSE_LEN 8 // 响应长度

// 模拟SRAM PUF输出
unsigned char sram_puf_output[RESPONSE_LEN];

// 模拟响应提取器
unsigned char response_extractor(unsigned char *input, int len)
{
    // 假设使用异或运算作为模糊提取器
    unsigned char output = 0;
    for (int i = 0; i < len; i++)
    {
        output ^= input[i];
    }
    return output;
}

// 模拟通信接口
unsigned char receive_data(int client_socket) // 添加客户端套接字参数
{
    // 假设使用TCP协议接收数据
    unsigned char data;
    recv(client_socket, &data, 1, 0); // 使用recv函数接收一个字节的数据
    printf("Receiving data: %02X\n", data);
    return data;
}

void send_data(unsigned char data, int client_socket) // 添加客户端套接字参数
{
    // 假设使用TCP协议发送数据
    printf("Sending data: %02X\n", data);
    send(client_socket, &data, 1, 0); // 使用send函数发送一个字节的数据
}

int main()
{
    int client_socket; // 定义客户端套接字变量
    struct sockaddr_in server_address; // 定义服务端地址结构体变量

    client_socket = socket(AF_INET, SOCK_STREAM, 0); // 创建TCP套接字

    memset(&server_address, 0, sizeof(server_address)); // 初始化地址结构体变量
    server_address.sin_family = AF_INET; // 设置协议族为IPv4
    server_address.sin_port = htons(8888); // 设置端口号为8888，使用htons函数转换为网络字节序
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // 设置IP地址为本地回环地址，使用inet_addr函数转换为网络字节序

    connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)); // 连接服务端的套接字

    int device_id = 1; // 假设设备号为1

    send_data(htons(device_id), client_socket); // 发送设备号给服务端，并使用htons函数转换为网络字节序

    unsigned char request = receive_data(client_socket); // 接收服务端的请求

    if (request == 0xFF) // 如果请求是注册请求，假设使用0xFF作为注册请求标志位
    {
        printf("Receiving registration request\n");
        // 模拟接收质询
        unsigned char challenge[CHALLENGE_LEN];
        printf("Receiving challenge: ");
        for (int i = 0; i < CHALLENGE_LEN; i++)
        {
            challenge[i] = receive_data(client_socket); // 使用修改后的receive_data函数
            printf("%02X ", challenge[i]);
        }
        printf("\n");

        // 模拟产生响应
        unsigned char response = response_extractor(sram_puf_output, RESPONSE_LEN);
        printf("Generating response: %02X\n", response);

        // 模拟发送响应
        send_data(response, client_socket); // 使用修改后的send_data函数

        printf("Registration successful!\n");
    }
    else 
    {
        printf("Receiving authentication request\n");
        // 模拟接收质询
        unsigned char challenge[CHALLENGE_LEN];
        printf("Receiving challenge: ");
        for (int i = 0; i < CHALLENGE_LEN; i++)
        {
            challenge[i] = receive_data(client_socket); // 使用修改后的receive_data函数
            printf("%02X ", challenge[i]);
        }
        printf("\n");

        // 模拟产生响应
        unsigned char response = response_extractor(sram_puf_output, RESPONSE_LEN);
        printf("Generating response: %02X\n", response);

        // 模拟发送响应
        send_data(response, client_socket); // 使用修改后的send_data函数

        printf("Authentication successful!\n");
    }

    close(client_socket); // 关闭客户端套接字

    return 0;
}
