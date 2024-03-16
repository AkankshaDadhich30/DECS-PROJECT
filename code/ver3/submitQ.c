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
    if (argc != 7){
        perror("Usage: ./submit  <serverIP> <port>  <sourceCodeFileTobeGraded> <loop num> <sleep time> <timeout>\n");
        return -1;
    }

    char server_ip[40], ip_port[40], file_path[256];
    int server_port; int sockfd;
    strcpy(server_ip, argv[1]);
    server_port = atoi(argv[2]);
    strcpy (file_path,argv[3]);
    int loopNum = atoi(argv[4]);
    int sleepTime = atoi(argv[5]);
    int timeout = atoi(argv[6]);

    struct timeval response_start, response_end;
    struct timeval total_start, total_end;
    double response_time = 0.0;
    double total_time = 0.0;
    int successful_responses = 0;
    
    gettimeofday(&total_start, NULL);
    for(int i=0; i<loopNum; i++){
        gettimeofday(&response_start, NULL);
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1){
            perror("Socket creation failed");
            return -1;
        }
        struct sockaddr_in serv_addr;
        bzero((char *)&serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(server_port);
        inet_pton(AF_INET, server_ip, &serv_addr.sin_addr.s_addr);

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

        if (send_file(sockfd, file_path) != 0) {
            printf ("Error sending source file\n");
            close(sockfd);
            return -1;
        }
        //printf ("Code sent for grading, waiting for response\n");
        size_t bytes_read;
        char buffer[BUFFER_SIZE];
        while (true) {
            bytes_read = recv(sockfd, buffer, sizeof(buffer), 0);
            if (bytes_read <= 0)
                break;
            write(STDOUT_FILENO, "Server Response: ", 17);
            write(STDOUT_FILENO, buffer, bytes_read);
            bzero(buffer, BUFFER_SIZE);
        }
        close(sockfd);
        gettimeofday(&response_end, NULL);
        response_time = response_time + (response_end.tv_sec-response_start.tv_sec) + (response_end.tv_usec-response_start.tv_usec)/1000000.0;
        successful_responses++;
        sleep(sleepTime);
    }
    gettimeofday(&total_end, NULL);
    total_time = (total_end.tv_sec-total_start.tv_sec) + (total_end.tv_usec-total_start.tv_usec)/1000000.0;
    printf("Successful Responses  = %d\n", successful_responses);
    printf("Average response time = %f\n", (response_time/successful_responses));
    printf("Total experiment time = %f\n", total_time);
    printf("Throughput            = %f\n", (successful_responses/total_time));
    return 0;
}