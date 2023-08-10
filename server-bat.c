// 服务端代码
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sqlite3.h> // 引入SQLite头文件

// 假设使用SRAM PUF
#define CHALLENGE_LEN 8 // 质询长度
#define RESPONSE_LEN 8 // 响应长度

int server_socket; // 定义服务端套接字变量
struct sockaddr_in server_address; // 定义服务端地址结构体变量
sqlite3 *db; // 定义数据库变量

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

// 模拟认证协议
void authenticate(int device_id, int client_socket) // 添加客户端套接字参数
{
    // 假设使用挑战-应答协议

    sqlite3_stmt *stmt; // 定义SQL语句对象变量

    // 查询设备是否存在于数据库中
    sqlite3_prepare_v2(db, "SELECT * FROM device_info WHERE device_id = ?", -1, &stmt, NULL); // 准备查询语句
    sqlite3_bind_int(stmt, 1, device_id); // 绑定设备号参数

    if (sqlite3_step(stmt) == SQLITE_ROW) // 执行查询语句，并判断返回值
    {
        // 设备存在，进行认证过程
        printf("Device exists!\n");

        unsigned char *challenge = sqlite3_column_blob(stmt, 1); // 获取质询数据
        unsigned char response = sqlite3_column_int(stmt, 2); // 获取响应数据

        sqlite3_finalize(stmt); // 释放查询语句对象占用的资源

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

        // 比较响应与数据库中的响应是否匹配
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
        printf("Device does not exist!\n");

        sqlite3_finalize(stmt); // 释放查询语句对象占用的资源

        unsigned char challenge[CHALLENGE_LEN]; // 定义质询数组变量

        for (int i = 0; i < CHALLENGE_LEN; i++)
        {
            challenge[i] = rand() % 256; // 随机生成质询数据
        }

        // 发送注册请求给设备，假设使用0xFF作为注册请求标志位
        send_data(0xFF, client_socket);
        printf("Sending registration request: FF\n");
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

        // 将设备号、质询和响应数据插入到数据库中
        sqlite3_prepare_v2(db, "INSERT INTO device_info VALUES (?, ?, ?)", -1, &stmt, NULL); // 准备插入语句
        sqlite3_bind_int(stmt, 1, device_id); // 绑定设备号参数
        sqlite3_bind_blob(stmt, 2, challenge, CHALLENGE_LEN, NULL); // 绑定质询参数
        sqlite3_bind_int(stmt, 3, response); // 绑定响应参数
        sqlite3_step(stmt); // 执行插入语句
        sqlite3_finalize(stmt); // 释放插入语句对象占用的资源

        printf("Registration successful!\n");
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

    sqlite3_open("device.db", &db); // 打开或创建数据库文件

    while (1) // 使用无限循环持续监听（保持运行）
    {
        int client_socket; // 定义客户端套接字变量
        client_socket = accept(server_socket, NULL, NULL); // 接受一个连接请求，并返回一个新的套接字变量
        printf("A new device connected!\n");
        int device_id = receive_data(client_socket); // 接收设备号

        authenticate(device_id, client_socket); // 使用修改后的authenticate函数

        close(client_socket); // 关闭客户端套接字
    }

    sqlite3_close(db); // 关闭数据库文件

    close(server_socket); // 关闭服务端套接字

    return 0;
}
