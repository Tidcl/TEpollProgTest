#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main()
{
    int epfd = epoll_create(1);

    int listfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_port = htons(8888);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ns = bind(listfd, (sockaddr*)&server_addr, sizeof(server_addr));

    if(listen(listfd, 20) > 0) printf("list failed.");

    printf("bind: %d\n", ns);

    

    struct epoll_event ep_ev = {0, {0}};
    ep_ev.events = EPOLLIN;
    ep_ev.data.fd = listfd;
    int rtn = epoll_ctl(epfd, EPOLL_CTL_ADD, listfd, &ep_ev);

    struct epoll_event events[12];
    while(1)
    {
        int n = epoll_wait(epfd, events, 12, 1000);
        if(n < 0) 
        {
            printf("break\n");
            break;
        }

        for(int i = 0; i < n; i++)
        {
            printf("deal fd = %d\n", events[i].data.fd);

            if(events[i].data.fd == 0) continue;

            
            if(events[i].data.fd == listfd)
            {
                struct sockaddr_in client_addr;
	            socklen_t len = sizeof(client_addr);
                int fd = accept(listfd, (sockaddr*)&client_addr, &len);
                if(fd != -1)
                {
                    memset(&ep_ev, 0, sizeof(ep_ev));
                    ep_ev.events = EPOLLIN;
                    ep_ev.data.fd = fd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ep_ev);
                    printf("add socket %d\n", fd);
                }
            }else if(events[i].events & EPOLLIN)
            {
                char buffer[1024];
                int n = recv(events[i].data.fd, buffer, 1024, 0); // 
                if(n == 0)
                {
                    close(events[i].data.fd);
                }
                printf("[rece %d] %s\n", events[i].data.fd, buffer);
                ep_ev.events = EPOLLOUT;
                ep_ev.data.fd = events[i].data.fd;
                epoll_ctl(epfd, EPOLL_CTL_MOD, events[i].data.fd, &ep_ev);
            }else if(events[i].events & EPOLLOUT)
            {
                int n = send(events[i].data.fd, "qwert", 5, 0);
                if(n == 0)
                {
                    close(events[i].data.fd);
                }
                memset(&ep_ev, 0, sizeof(ep_ev));
                ep_ev.events = EPOLLIN;
                ep_ev.data.fd = events[i].data.fd;
                epoll_ctl(epfd, EPOLL_CTL_MOD, events[i].data.fd, &ep_ev);
            }
        }
    }

    return 0;    
}