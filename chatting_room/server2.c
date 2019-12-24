#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/select.h>


#define PortNumber 8888
#define MaxClients 30
#define TRUE 1


int main() {
    int opt = TRUE;
    int i, master_socket, client_socket[MaxClients], new_socket;
    int server_addr_length;
    int max_sd, sd, activity, valread;
    struct sockaddr_in server_addr;

    char buffer[1025] = {}, name[1025] = {};

    //set of socket descriptors
    fd_set readfds;

    char *message = "welcome \r\n";

    // initializing all client_socket[] to 0
    for (i = 0; i < MaxClients; i++) {
        client_socket[i] = 0;
    }

    // create a master socket
    if ((master_socket = socket(PF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // set address
    server_addr_length = sizeof(server_addr);
    bzero(&server_addr, server_addr_length);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PortNumber);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // bind the socket to ip and port
    if (bind(master_socket, (struct sockaddr *)&server_addr, server_addr_length) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    // printf("Listener on port %d \n", PortNumber);

    // try to specify maximum connections for the socket
    if (listen(master_socket, 50) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    // puts("Waiting for connections ...");

    while (TRUE) {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        //add child sockets to set
        for (i = 0; i < MaxClients; i++) {
            //socket descriptor
            sd = client_socket[i];

            //if valid socket descriptor then add to read list
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }

            //highest file descriptor number, need it for the select function
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        //wait for an activity on one of the sockets , timeout is NULL ,  
        //so wait indefinitely 
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            printf("select error");
        }

        //If something happened on the master socket ,  
        //then its an incoming connection  
        if (FD_ISSET(master_socket, &readfds)) {
            if ((new_socket = accept(master_socket,
                                     (struct sockaddr *)&server_addr, &server_addr_length)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // printf("New connection , socket fd is %d , ip is : %s , port : %d  \n" 
            //     , new_socket 
            //     , inet_ntoa(server_addr.sin_addr) 
            //     , ntohs(server_addr.sin_port));
            
            if (send(new_socket, message, strlen(message), 0) != strlen(message)) {
                perror("send");
            }

            // puts("Welcome message sent successfully");

            // receive user's name
            if (recv(new_socket, name, sizeof(name), 0) < 0) {
                printf("Error recving packet\n");
                close(new_socket);
            }
            printf("User: %s connect\n", name);
            
            //add new socket to array of sockets
            for (i = 0; i < MaxClients; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    // printf("Adding to list of sockets as %d\n", i);

                    break;
                }
            }
        }

        for (i = 0; i < MaxClients; i++) {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readfds)) {
                if ((valread = read(sd, buffer, 1024)) == 0) {
                    //Somebody disconnected , get his details and print
                    getpeername(sd, (struct sockaddr *)&server_addr, (socklen_t *)&server_addr_length);
                    printf("Host disconnected , ip %s , port %d \n",
                           inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close(sd);
                    client_socket[i] = 0;
                } else {
                    //set the string terminating NULL byte on the end
                    //of the data read
                    buffer[valread] = '\0';

                    printf("%s", buffer);

                    // send message to everyone
                    for (int j = 0; j <= MaxClients; j++)
                    {
                        if (sd == client_socket[j] || client_socket[j] == 0)
                        {
                            continue;
                        }

                        send(client_socket[j], buffer, sizeof(buffer), 0);
                    }
                }
            }
        }
    }
    
    return 0;
}
