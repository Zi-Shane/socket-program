#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>


#define PortNumber 8888

// client addresses
struct sockaddr_in client_addr[50];
int client_addr_length = sizeof(client_addr[0]);

// 
int confd[50] = {};
int count = 0;


void *broadcast(void *indexp) {
    int index = *(int *)indexp;
    char buf_rcv[255] = {};
    char buf_snd[255] = {};

    // username
    char name[255] = {};
    int ret = recv(confd[index], name, sizeof(name), 0);
    if (ret < 0)
    {
        printf("Error recving packet\n");
        close(confd[index]);
    }
    printf("%s connect\n", name);

    while (1)
    {
        bzero(&buf_rcv, sizeof(buf_rcv));
        ret = recv(confd[index], buf_rcv, sizeof(buf_rcv), 0);
        if (ret < 0)
        {
            printf("Error recving packet\n");
            close(confd[index]);
            return;
        }   
        if (strcmp("quit\n", buf_rcv) == 0)
        {
            bzero(&buf_snd, sizeof(buf_snd));
            sprintf(buf_snd, "%s: has leaved\n", name);
            // send to everyone, and exclude sender
            for (int i = 0; i <= count; i++)
            {
                if (i == index || confd[i] == 0)
                {
                    continue;
                }
                
                send(confd[i], buf_snd, strlen(buf_snd), 0);
            }
            confd[index] = -1;
            pthread_exit(0);
            return;
        
        }

        sprintf(buf_snd, "%s: %s", name, buf_rcv);
        printf("%s", buf_snd);
        // send to everyone, and exclude sender
        for (int i = 0; i <= count; i++)
        {
            if (i == index || confd[i] == 0)
            {
                continue;
            }
            
            send(confd[i], buf_snd, sizeof(buf_snd), 0);
        }
        
    }
    
}


int main() {
    // create a socket
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Error creating socket\n");
        return -1;
    }

    // set address
    struct sockaddr_in server_addr;
    int server_addr_length = sizeof(server_addr);
    bzero(&server_addr, server_addr_length);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PortNumber);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // bind the socket to ip and port
    int ret_bind = bind(sock, (struct sockaddr *)&server_addr, server_addr_length);
    if (ret_bind == -1) {
        printf("Error binding\n");
        close(sock);
    }
    // printf("Listener on port %d \n", PortNumber);

    // try to specify maximum connections for the socket
    int ret_listen = listen(sock, 50);
    if (ret_listen == -1) {
        printf("Listen failed\n");
        close(sock);
    }

    // puts("Waiting for connections ...");

    int index = 0;
    while (count <= 50)
    {
        // accept incoming connection
        // store new connection in confd array
        confd[count] = accept(sock, (struct sockaddr *)&client_addr[count], &client_addr_length);
        ++count;
        index = count - 1;

        // use sub thread to continuously receive messages
        pthread_t tid;
        int ret = pthread_create(&tid, NULL, broadcast, &index);
        if (ret < 0)
        {
            printf("pthread create failed");
            return -1;
        }
        
    }
    
    return 0;
}
