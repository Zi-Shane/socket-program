#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/select.h>


#define Server_PortNumber 8888
#define Server_Address "127.0.0.1"

int sock;


// void *recv_other(void *arg)
// {
//     char buf[255]= {};
//     while (1)
//     {
//         int ret = recv(sock, buf, sizeof(buf), 0);
//         if (ret < 0)
//         {
//             perror("recv");
//             return;
//         }
//         printf("%s",buf);
//     }
    
// }


int main() {
    // create username
    char name[255] = {};
    char buffer_snd[1025] = {};
    printf("username: ");
    scanf("%s",name);
    getchar();
    
    // create socket
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Error creating socket\n");
        return -1;
    }

    // set address
    struct sockaddr_in server_addr;
    int server_addr_length = sizeof(server_addr);
    bzero(&server_addr, server_addr_length);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(Server_PortNumber);
    server_addr.sin_addr.s_addr = inet_addr(Server_Address);

    // establish 3-way
    int ret_connect = connect(sock, (struct sockaddr *)&server_addr, server_addr_length);
    if (ret_connect == -1) {
        printf("connect fail\n");
        close(sock);
        return -1;
    }



    int ret_sent = send(sock, name, strlen(name), 0);
    if (ret_sent < 0) {
        printf("Error sending packet\n");
    }

    // // sub thread to receive message form other users
    // pthread_t tid;
    // int ret_pthread = pthread_create(&tid, NULL, recv_other, NULL);
    // if (ret_pthread < 0)
    // {
    //     printf("pthread create failed");
    // }

    // send message
    char buffer[255] = {}, buf[256] = {};
    int max_sd, activity, valread;
    fd_set readfds;
    max_sd = sock;
    while (1)
    {   
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        FD_SET(0, &readfds);
        
        activity = select(max_sd + 1 , &readfds , NULL , NULL , NULL);
        
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            bzero(&buffer, sizeof(buffer));
            read(0, buffer, sizeof(buffer));
            sprintf(buffer_snd, "%s: %s", name, buffer);
            int ret_send = send(sock, buffer_snd, strlen(buffer_snd), 0);
            if (ret_send < 0)
            {
                printf("Error sending packet\n");
            }
            // printf("%s", buffer_snd);

            if (strcmp("quit\n", buffer) == 0)
            {
                printf("You have leaved the chatting room\n");
                close(sock);
                return 0;
            }
            
        }
        
        if (FD_ISSET(sock, &readfds)) {
            int ret = recv(sock, buf, sizeof(buf), 0);
            if (ret < 0)
            {
                perror("recv");
            }
            // buf[valread] = '\0';
            printf("%s",buf);
            
        }
        
    }

    close(sock);
    return 0;
}
