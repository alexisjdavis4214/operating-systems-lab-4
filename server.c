#include <netinet/in.h>  // structure for storing address information 
#include <stdio.h> 
#include <string.h>
#include <stdlib.h> 
#include <sys/socket.h>  // for socket APIs 
#include <sys/types.h> 
#include <signal.h>
#include "list.h"

#define PORT 9001
#define ACK "ACK"

int servSockD, clientSocket;
list_t *mylist;

// Signal handler for graceful termination
void handle_signal(int sig) {
    if (clientSocket >= 0) close(clientSocket);
    if (servSockD >= 0) close(servSockD);
    if (mylist != NULL) list_free(mylist);
    printf("\nServer shutting down gracefully.\n");
    exit(0);
}

int main(int argc, char const* argv[]) {
    int n, val, idx;
    char buf[1024];
    char sbuf[1024];
    char* token;

    struct sockaddr_in servAddr;

    // Register signal handler for termination
    signal(SIGINT, handle_signal);

    // Create server socket
    servSockD = socket(AF_INET, SOCK_STREAM, 0);
    if (servSockD == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    servAddr.sin_family = AF_INET; 
    servAddr.sin_port = htons(PORT); 
    servAddr.sin_addr.s_addr = INADDR_ANY; 

    // Bind socket to specified IP and port
    if (bind(servSockD, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    // Listen for connections
    if (listen(servSockD, 1) < 0) {
        perror("Listen failed");
        exit(1);
    }

    // Accept client connection
    clientSocket = accept(servSockD, NULL, NULL);
    if (clientSocket < 0) {
        perror("Client accept failed");
        exit(1);
    }

    // Create the linked list
    mylist = list_alloc();

    while (1) {
        n = recv(clientSocket, buf, sizeof(buf), 0);
        if (n <= 0) continue;

        buf[n] = '\0';
        token = strtok(buf, " ");

        if (strcmp(token, "exit") == 0) {
            list_free(mylist);
            printf("Client requested exit.\n");
            break;
        }
        else if (strcmp(token, "get_length") == 0) {
            val = list_length(mylist);
            sprintf(sbuf, "Length = %d", val);
        }
        else if (strcmp(token, "add_front") == 0) {
            token = strtok(NULL, " ");
            if (token) {
                val = atoi(token);
                list_add_to_front(mylist, val);
                sprintf(sbuf, "%s %d", ACK, val);
            }
        }
        else if (strcmp(token, "add_back") == 0) {
            token = strtok(NULL, " ");
            if (token) {
                val = atoi(token);
                list_add_to_back(mylist, val);
                sprintf(sbuf, "%s %d", ACK, val);
            }
        }
        else if (strcmp(token, "add_position") == 0) {
            token = strtok(NULL, " ");
            if (token) {
                idx = atoi(token);
                token = strtok(NULL, " ");
                if (token) {
                    val = atoi(token);
                    list_add_at_index(mylist, idx, val);
                    sprintf(sbuf, "%s %d at %d", ACK, val, idx);
                }
            }
        }
        else if (strcmp(token, "remove_back") == 0) {
            val = list_remove_from_back(mylist);
            sprintf(sbuf, "Removed from back: %d", val);
        }
        else if (strcmp(token, "remove_front") == 0) {
            val = list_remove_from_front(mylist);
            sprintf(sbuf, "Removed from front: %d", val);
        }
        else if (strcmp(token, "remove_position") == 0) {
            token = strtok(NULL, " ");
            if (token) {
                idx = atoi(token);
                val = list_remove_at_index(mylist, idx);
                sprintf(sbuf, "Removed %d at %d", val, idx);
            }
        }
        else if (strcmp(token, "get") == 0) {
            token = strtok(NULL, " ");
            if (token) {
                idx = atoi(token);
                val = list_get_elem_at(mylist, idx);
                sprintf(sbuf, "Value at %d = %d", idx, val);
            }
        }
        else if (strcmp(token, "print") == 0) {
            sprintf(sbuf, "%s", listToString(mylist));
        }
        else {
            sprintf(sbuf, "Invalid command!");
        }

        send(clientSocket, sbuf, sizeof(sbuf), 0);
        memset(buf, '\0', sizeof(buf));
    }

    // Cleanup on exit
    handle_signal(SIGINT);
    return 0; 
}

