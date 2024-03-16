#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <netinet/in.h>
#include "filecalls.h"

const int BUFFER_SIZE = 1024; 
const int MAX_FILE_SIZE_BYTES = 4;
const int MAX_TRIES = 5;

int send_file(int sockfd, char* file_path) {
    char buffer[BUFFER_SIZE]; 
    bzero(buffer, BUFFER_SIZE); 
    FILE *file = fopen(file_path, "rb"); 
    if (!file) {
        perror("Error opening file");
        return -1;
    }
		
    fseek(file, 0L, SEEK_END); 
    int file_size = ftell(file);
    //printf("File size is: %d\n", file_size);
    fseek(file, 0L, SEEK_SET);
		
    char file_size_bytes[MAX_FILE_SIZE_BYTES];
    memcpy(file_size_bytes, &file_size, sizeof(file_size));
    
    if (send(sockfd, file_size_bytes, sizeof(file_size_bytes), 0) == -1){
        perror("Error sending file size");
        fclose(file);
        return -1;
    }

    while (!feof(file)) {
        size_t bytes_read = fread(buffer, 1, BUFFER_SIZE -1, file);
        if (send(sockfd, buffer, bytes_read+1, 0) == -1) {
            perror("Error sending file data");
            fclose(file);
            return -1;
        }
        bzero(buffer, BUFFER_SIZE);
    }
    fclose(file);
    return 0;
}

int recv_file(int sockfd, char* file_path) {
  char buffer[BUFFER_SIZE]; 
  bzero(buffer, BUFFER_SIZE); 
  FILE *file = fopen(file_path, "wb");
  if (!file){
    perror("Error opening file");
    return -1;
  }
	//buffer for getting file size as bytes
  char file_size_bytes[MAX_FILE_SIZE_BYTES];
  if (recv(sockfd, file_size_bytes, sizeof(file_size_bytes), 0) == -1) {
    perror("Error receiving file size"); 
    fclose(file);
    return -1;
  }
   
  int file_size;
  memcpy(&file_size, file_size_bytes, sizeof(file_size_bytes));
  printf("File size is: %d\n", file_size);
  size_t bytes_read = 0, total_bytes_read =0;;
  while (true) {
    bytes_read = recv(sockfd, buffer, BUFFER_SIZE, 0);
    total_bytes_read += bytes_read;
    if (bytes_read <= 0) {
      perror("Error receiving file data");
      fclose(file);
      return -1;
    }
    fwrite(buffer, 1, bytes_read, file);
    bzero(buffer, BUFFER_SIZE);

    if (total_bytes_read >= file_size)
      break;
  }
  fclose(file);
  return 0;
}