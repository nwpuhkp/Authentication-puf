// 服务端代码
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
// 引入文件操作头文件
#include <stdio.h>
// 假设使用SRAM PUF
#define CHALLENGE_LEN 8 // 质询长度
#define RESPONSE_LEN 8 // 响应长度
// 定义CSV文件名
#define FILE_NAME "device.csv"
// 定义分隔符
#define DELIMITER ","
int server_socket; // 定义服务端套接字变量
struct sockaddr_in server_address; // 定义服务端地址结构体变量
// 查询设备是否存在于CSV文件中，如果存在，返回1，并将质询和响应数据复制到参数中；如果不存在，返回0
int query_device(int device_id, unsigned char *challenge, unsigned char *response)
{
    FILE *fp = fopen(FILE_NAME, "r"); // 以只读模式打开CSV文件
    if (fp == NULL) // 判断文件是否打开成功
    {
        printf("Error opening file!\n");
        return 0;
    }
    char line[256]; // 定义一行数据的缓冲区
    while (fgets(line, sizeof(line), fp)) // 逐行读取文件内容
    {
        int id; // 定义设备号变量
        unsigned char ch[CHALLENGE_LEN]; // 定义质询数组变量
        unsigned char res; // 定义响应变量
        sscanf(line, "%d,%02X%02X%02X%02X%02X%02X%02X%02X,%02X", &id, &ch[0], &ch[1], &ch[2], &ch[3], &ch[4], &ch[5], &ch[6], &ch[7], &res); // 使用sscanf函数解析一行数据，按照逗号分隔符分割成三个字段
        if (id == device_id) // 判断设备号是否匹配
        {
            memcpy(challenge, ch, CHALLENGE_LEN); // 复制质询数据到参数中
            *response = res; // 复制响应数据到参数中
            fclose(fp); // 关闭文件
            return 1; // 返回1表示设备存在
        }
    }
    fclose(fp); // 关闭文件
    return 0; // 返回0表示设备不存在
}
// 将设备号、质询和响应数据插入到CSV文件中，返回1表示成功，返回0表示失败
int insert_device(int device_id, unsigned char *challenge, unsigned char response)
{
    FILE *fp = fopen(FILE_NAME, "a"); // 以追加模式打开CSV文件
    if (fp == NULL) // 判断文件是否打开成功
    {
        printf("Error opening file!\n");
        return 0;
    }
    fprintf(fp, "%d,%02X%02X%02X%02X%02X%02X%02X%02X,%02X\n", device_id, challenge[0], challenge[1], challenge[2], challenge[3], challenge[4], challenge[5], challenge[6], challenge[7], response); // 使用fprintf函数将一行数据写入到文件中，按照逗号分隔符分割成三个字段
    fclose(fp); // 关闭文件
    return 1; // 返回1表示成功
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
// 模拟认证协议，使用CSV文件代替数据库操作
void authenticate(int device_id, int client_socket) // 添加客户端套接字参数
{
    // 假设使用挑战-应答协议
    unsigned char challenge[CHALLENGE_LEN]; // 定义质询数组变量
    unsigned char response; // 定义响应变量
    // 查询设备是否存在于CSV文件中，并获取质询和响应数据
    if (query_device(device_id, challenge, &response)) // 判断返回值是否为1，即设备是否存在
    {
        // 设备存在，进行认证过程
        printf("This device exsited!\n");
        // 发送质询给设备
        printf("Sending challenge: ");
        for (int i = 0; i < CHALLENGE_LEN; i++)
        {
            send_data(challenge[i], client_socket); // 使用修改后的send_data函数
        }
        printf("\n");
        // 接收设备的响应
        unsigned char device_response = receive_data(client_socket); // 使用修改后的receive_data函数
        printf("Receiving response: %02X\n", device_response);
        // 比较响应与CSV文件中的响应是否匹配
        if (device_response == response)
        {
            printf("Authentication successful!\n");
        }
        else
        {
            printf("Authentication failed!\n");
        }
    }
    else
    {
        // 设备不存在，进行注册过程
        printf("This device not exsited!\n");
        for (int i = 0; i < CHALLENGE_LEN; i++)
        {
            challenge[i] = rand() % 256; // 随机生成质询数据
        }
        // 发送注册请求给设备，假设使用0xFF作为注册请求标志位
        send_data(0xFF, client_socket);
        // 发送质询给设备
        printf("Sending challenge: ");
        for (int i = 0; i < CHALLENGE_LEN; i++)
        {
            send_data(challenge[i], client_socket);
        }
        printf("\n");
        // 接收设备的响应
        unsigned char response = receive_data(client_socket);
        printf("Receiving response: %02X\n", response);
        // 将设备号、质询和响应数据插入到CSV文件中
        if (insert_device(device_id, challenge, response)) // 判断返回值是否为1，即插入是否成功
        {
            printf("Registration successful!\n");
        }
        else
        {
            printf("Registration failed!\n");
        }
    }
}
int main()
{
    server_socket = socket(AF_INET, SOCK_STREAM, 0); // 创建TCP套接字

    memset(&server_address, 0, sizeof(server_address)); // 初始化地址结构体变量
    server_address.sin_family = AF_INET; // 设置协议族为IPv4
    server_address.sin_port = htons(8888); // 设置端口号为8888，使用htons函数转换为网络字节序
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // 设置IP地址为本地回环地址，使用htonl函数转换为网络字节序

    bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)); // 绑定套接字和地址结构体

    listen(server_socket, 10); // 监听套接字上的连接请求，设置队列长度为10

    while (1) // 使用无限循环持续监听（保持运行）
    {
        int client_socket; // 定义客户端套接字变量
        client_socket = accept(server_socket, NULL, NULL); // 接受一个连接请求，并返回一个新的套接字变量
        printf("Accept a connection!\n");
        int device_id = ntohs(receive_data(client_socket)); // 接收设备号，并使用ntohs函数转换为主机字节序
        printf("Device ID: %d\n", device_id);
        authenticate(device_id, client_socket); // 使用修改后的authenticate函数

        close(client_socket); // 关闭客户端套接字
    }

    close(server_socket); // 关闭服务端套接字

    return 0;
}
