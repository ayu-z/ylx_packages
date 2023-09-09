#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define UEVENT_BUFFER_SIZE 2048

int main() {
    int sock_fd;
    struct sockaddr_nl sa;
    char buffer[UEVENT_BUFFER_SIZE];

    // 创建Netlink套接字
    sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    if (sock_fd == -1) {
        perror("无法创建Netlink套接字");
        return -1;
    }

    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = NETLINK_KOBJECT_UEVENT;

    // 绑定套接字
    if (bind(sock_fd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        perror("绑定失败");
        close(sock_fd);
        return -1;
    }

    printf("等待USB设备绑定和解绑事件...\n");

    while (1) {
        ssize_t length = recv(sock_fd, buffer, sizeof(buffer), 0);
        if (length < 0) {
            perror("接收事件失败");
            close(sock_fd);
            return -1;
        }

        // 解析Netlink消息
        int i = 0;
        while (i < length) {
            struct nlmsghdr *nlh = (struct nlmsghdr *)&buffer[i];

            if (nlh->nlmsg_type == NLMSG_ERROR) {
                // 错误消息
                fprintf(stderr, "接收到错误消息\n");
                break;
            }

            // USB设备绑定和解绑事件报文以"ACTION=bind"和"ACTION=unbind"开头
            if (strncmp((char *)NLMSG_DATA(nlh), "ACTION=bind", 11) == 0) {
                printf("USB设备绑定事件报文:\n%s\n", (char *)NLMSG_DATA(nlh));
            } else if (strncmp((char *)NLMSG_DATA(nlh), "ACTION=unbind", 13) == 0) {
                printf("USB设备解绑事件报文:\n%s\n", (char *)NLMSG_DATA(nlh));
            }

            i += NLMSG_ALIGN(nlh->nlmsg_len);
        }
    }

    // 关闭套接字
    close(sock_fd);

    return 0;
}
