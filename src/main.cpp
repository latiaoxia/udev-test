#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <linux/types.h>
#include <linux/netlink.h>

int main(void)
{
    int epoll_fd = epoll_create1(0);
    struct epoll_event ep_ev;
    int ret;
    char buf[512];

    if (epoll_fd == -1) {
        fprintf(stderr, "Failed to create epoll fd\n");
        return -EINVAL;
    }

    struct sockaddr_nl nls = {0};
    nls.nl_family = AF_NETLINK;
    nls.nl_groups = NETLINK_KOBJECT_UEVENT;
    nls.nl_pid = getpid();

    ep_ev.events = EPOLLIN;
    ep_ev.data.fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    if (ep_ev.data.fd == -1) {
        fprintf(stderr, "failed to create socket: %m\n");
        ret = -1;
        goto close_ep;
    }
    ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ep_ev.data.fd, &ep_ev);
    if (ret) {
        fprintf(stderr, "epoll add failed: %m\n");
        goto close_ep;
    }

    ret = bind(ep_ev.data.fd, (sockaddr*)&nls, sizeof(struct sockaddr_nl));
    if (ret) {
        fprintf(stderr, "Bind failed: %m\n");
        goto close_ep;
    }

    do {
        ret = epoll_wait(epoll_fd, &ep_ev, 1, -1);
        if (ret == -1) {
            fprintf(stderr, "epoll wait error: %m\n");
            goto close_ep;
        }

        int len = recv(ep_ev.data.fd, buf, sizeof(buf), MSG_DONTWAIT);
        if (len == -1) {
            ret = -errno;
            fprintf(stderr, "receive msg error: %m\n");
            goto close_ep;
        }
        printf("len: %d\n", len);

        int i = 0;
        while (i < len) {
            printf("%s\n", buf + i);
            i += strlen(buf + i) + 1;
        }
    }while(1);

close_ep:
    if (close(epoll_fd)) {
        fprintf(stderr, "Failed to close epoll fd\n");
        return -EINVAL;
    }

    return ret;
}
