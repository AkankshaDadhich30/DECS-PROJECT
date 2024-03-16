#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <error.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include "filecalls.h"

int main(int argc, char *argv[])
{
    char server_ip[40], ip_port[40], file_path[256];
    int server_port; int sockfd;
    // assign ip and port from arguments
    strcpy(server_ip, argv[1]);
    server_port = atoi(argv[2]);
    // declare time variables
    struct timeval total_start, total_end;
    double total_time = 0.0;
    // create half socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1){
        perror("Socket creation failed");
        return -1;
    }
    // assign server address
    struct sockaddr_in serv_addr;
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &serv_addr.sin_addr.s_addr);
    // connect to server
    int tries = 0;
    while (true) {
        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0)
            break;
        sleep(1);
        tries += 1;
        if (tries == MAX_TRIES) {
            printf ("Server not responding\n");
            return -1;
        }
    }
    // initialize buffer
    size_t bytes_read;
    char buffer[BUFFER_SIZE];
    // set request type as 0 - new and 1 - status
    int req_type = 0; 
    // if request is new
    if(strcmp(argv[3], "new") == 0){
        req_type = 0;
        // send request type
        send(sockfd, &req_type, sizeof(req_type), 0);
        strcpy (file_path,argv[4]);
        // send file
        if (send_file(sockfd, file_path) != 0) {
            printf ("Error sending source file\n");
            close(sockfd);
            return -1;
        }
        // receive request ID
        int recv_reqID = -1;
        bytes_read = recv(sockfd, &recv_reqID, sizeof(recv_reqID), 0);
        //printf("Server Response: IN QUEUE REQUEST ID = %d\n", recv_reqID);
        printf("%d\n", recv_reqID);
    } 
    // if request is status
    else if(strcmp(argv[3], "status") == 0) {
        req_type = 1;
        // send request type
        send(sockfd, &req_type, sizeof(req_type), 0);
        // send request ID
        int send_reqID = atoi(argv[4]);
        send(sockfd, &send_reqID, sizeof(send_reqID), 0);
        // receive response
        bytes_read = recv(sockfd, buffer, sizeof(buffer), 0);
        write(STDOUT_FILENO, "Server Response: ", 17);
        write(STDOUT_FILENO, buffer, bytes_read);
        bzero(buffer, BUFFER_SIZE);
    } 
    // request type not recognized
    else {
        perror("Usage: ./submit  <serverIP> <port>  <new or status> <optional sourceCodeFileTobeGraded>\n");
    }    
    // close socket
    close(sockfd);
    return 0;
}