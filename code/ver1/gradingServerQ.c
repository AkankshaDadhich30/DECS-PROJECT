#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <netinet/in.h>


const int BUFFER_SIZE = 1024;
const int MAX_FILE_SIZE_BYTES = 4;

void error(char *msg) {
  perror(msg);
  exit(1);
}

// Utility Function to receive a file of any size to the grading server
int recv_file(int sockfd, char* file_path) {
  // open a file to write the received file
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
  // get the file size and write the file contents
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

int main(int argc, char *argv[]) {
  int sockfd, newsockfd, portno; 
  // declare server and client address
  socklen_t clilen; 
  char buffer[256]; 
  struct sockaddr_in serv_addr, cli_addr; 
  int n;
  // check if port number is provided
  if (argc < 2) {
    fprintf(stderr, "Usage : ./server <port no>\n");
    exit(1);
  }  /* create socket */
  // open half of the socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0); 
  if (sockfd < 0)
    error("ERROR opening socket");
  bzero((char *)&serv_addr, sizeof(serv_addr)); 
  // initialize server address and port
  serv_addr.sin_family = AF_INET; 
  serv_addr.sin_addr.s_addr = INADDR_ANY;  
  portno = atoi(argv[1]);
  serv_addr.sin_port = htons(portno);  
  // bind the half socket to the server address
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");
  // listen to upto 50 incoming connections
  listen(sockfd, 50);
  // accept the incoming connections
  clilen = sizeof(cli_addr);
  // accept the incoming connections
  while (1){
    // complete the full socket
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
      error("ERROR on accept");
    // receive the file for grading
    recv_file(newsockfd, "file_.cpp");
    send(newsockfd, "I got your code file for grading\n", 33, 0);
    // execute commands on received file
    if (system("g++ file_.cpp 2> compile_.txt") !=0 )  n = send(newsockfd, "COMPILER ERROR\n", 15, 0);
    else if ( system("./a.out > out_.txt 2> runtime_.txt") !=0 ) n = send(newsockfd, "RUNTIME ERROR\n", 14, 0);
 		else if ( system("diff out_.txt output.txt > diff_.txt") )      n = send(newsockfd, "PROGRAM RAN WRONG OUTPUT\n", 25, 0);
    else                                                          n = send(newsockfd, "PROGRAM RAN CORRECT OUTPUT\n", 27, 0);
    // error in send function
    if (n < 0)
	    error("ERROR writing to socket");	
    close(newsockfd);
  }
  return 0;
}

