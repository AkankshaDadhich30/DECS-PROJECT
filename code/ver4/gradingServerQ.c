/* run using ./server <port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <pthread.h>
#include "filecalls.h"
#include "queuecalls.h"
#define hsize 50
// print error message
void error(char *msg) {
  perror(msg);
  exit(1);
}
// global variables
int thread_no = 0; int pool_size = 4;
pthread_t *threads = NULL; pthread_mutex_t mutex; pthread_cond_t cond;
int qsize=10; int front=-1; int rear=-1; int *myq;
int reqID=-1; int hashmap[hsize][2];
// Return Status 1-InQueue 2-InProgress 3-CompilerError 4-RuntimeError 5-ProgramRanWrong 6-ProgramRanCorrect
void returnstatus(int newsockfd, int reqID){
  // get status from hashmap
  int status = hashmap[reqID][1];
  // send status to client
  if(status == 1)       { send(newsockfd, &reqID, sizeof(reqID), 0); hashmap[reqID][1] = 2; }
  else if(status == 2)  send(newsockfd, "The request is still in queue please wait\n", 42, 0);
  else if(status == 3)  send(newsockfd, "Your request taken from queue but still in process\n", 51, 0);
  else if(status == 4)  send(newsockfd, "Your request processing is done, here are the results: COMPILER ERROR\n", 70, 0);
  else if(status == 5)  send(newsockfd, "Your request processing is done, here are the results: RUNTIME ERROR\n", 69, 0);
  else if(status == 6)  send(newsockfd, "Your request processing is done, here are the results: PROGRAM RAN WRONG OUTPUT\n", 83, 0);
  else if(status == 7)  send(newsockfd, "Your request processing is done, here are the results: PROGRAM RAN CORRECT OUTPUT\n", 83, 0);
  else send(newsockfd, "Grading request not found. Please check and resend your request ID or re-send your original grading request\n", 108, 0); 
}
// add task to queue and request ID to hshmap
void add_task(int sockfd) {
  pthread_mutex_lock(&mutex);			
    reqID++; queue_insert(reqID);
    hashmap[reqID][0] = sockfd; hashmap[reqID][1] = 1;
    // receive file
    char filename[50];          
    sprintf(filename, "file_%d.cpp", reqID);
    recv_file(sockfd, filename);
    // return status of request ID
    returnstatus(sockfd, reqID);
  // signal thread for new task
  pthread_cond_signal(&cond); 
  pthread_mutex_unlock(&mutex);
}
// complete grading the request
void process_task(int newsockfd, int reqID){
  // create filename and commands and receive file
  char filename[50];          sprintf(filename, "file_%d.cpp", reqID);
  char compile_command[100];  sprintf(compile_command, "g++ -w %s 2> compile_%d.txt", filename, reqID);
  char runtime_command[100];  sprintf(runtime_command, "./a.out > out_%d.txt 2> runtime_%d.txt", reqID, reqID);
  char diff_command[100];     sprintf(diff_command, "diff out_%d.txt output.txt > diff_%d.txt", reqID, reqID);
  // execute commands
  int n;
  if (system(compile_command) !=0 )       hashmap[reqID][1] = 4;
  else if ( system(runtime_command) !=0 ) hashmap[reqID][1] = 5;		
 	else if ( system(diff_command) !=0 )    hashmap[reqID][1] = 6;
  else                                    hashmap[reqID][1] = 7;
}
// thread starter wait till task is added
void *perform_task () {
  int currID, curr_socket;
  while (true) {
		pthread_mutex_lock(&mutex);
      // wait till task is added
      while(front == -1)		
        pthread_cond_wait(&cond, &mutex);
      // pick up task from queue and delete it
      currID = queue_delete();
      curr_socket = hashmap[currID][0];
      hashmap[currID][1] = 3;
		pthread_mutex_unlock(&mutex);		
    process_task(curr_socket, currID);
	}
}

int main(int argc, char *argv[]) {
  printf("\nserver started\n\n");
  int sockfd, newsockfd, portno, n; 
  socklen_t clilen; 
  struct sockaddr_in serv_addr, cli_addr; 
  // check arguments
  if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }
  // create half socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0); 
  if (sockfd < 0)
    error("ERROR opening socket");
  bzero((char *)&serv_addr, sizeof(serv_addr)); 
  // declare server address and port
  serv_addr.sin_family = AF_INET; 
  serv_addr.sin_addr.s_addr = INADDR_ANY;  
  portno = atoi(argv[1]);
  serv_addr.sin_port = htons(portno);  
  // bind server address to port
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");
  // listen to incoming connections
  listen(sockfd, qsize);
  clilen = sizeof(cli_addr);
  // initialize threads, mutex and cond
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond, NULL);
  threads = (pthread_t*)(malloc(pool_size * sizeof(pthread_t)));
  // initialize the queue
  myq = (int*)malloc(qsize*sizeof(int));
  // create threads
  for (int i = 0; i < pool_size; i++)
    pthread_create(&threads[i], NULL, perform_task, NULL);
  // initialize buffer
  size_t bytes_read;
  char buffer[BUFFER_SIZE];
  // initialize request type as 0-new and 1-status
  int req_type = 0; 
  // accept the connections
  while (1){
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
      error("ERROR on accept");
    // clear the buffer and receive request type
    bzero(buffer, BUFFER_SIZE);
    bytes_read = recv(newsockfd, &req_type, sizeof(req_type), 0);
    // add task to queue and hashmap if the request is new
    if( req_type == 0 ){
      add_task(newsockfd);
    }
    // send the current status of a request for status 
    else {
      int recv_reqID = 0;
      bytes_read = recv(newsockfd, &recv_reqID, sizeof(recv_reqID), 0);
      returnstatus(newsockfd, recv_reqID);
    }
    // close the socket
    close(newsockfd);
  }
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond);	
}
