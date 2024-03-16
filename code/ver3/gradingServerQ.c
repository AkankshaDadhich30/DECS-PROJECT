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

void error(char *msg) {
  perror(msg);
  exit(1);
}

int thread_no = 0; int pool_size;
pthread_t *threads = NULL; pthread_mutex_t mutex; pthread_cond_t cond;
int qsize=50; int *myq; int front=-1; int rear=-1;

void add_task(int sockfd) {
  pthread_mutex_lock(&mutex);					
		queue_insert(sockfd);
  pthread_mutex_unlock(&mutex);
  pthread_cond_signal(&cond);
}

void process_task(int mysockfd){

  pthread_mutex_lock(&mutex); 
    int local_thread = thread_no++;
    int newsockfd = mysockfd;
  pthread_mutex_unlock(&mutex); 

  printf("starting thread %d\n", (int) pthread_self());

  char filename[50];
    sprintf(filename, "file_%d.cpp", local_thread);
  char compile_command[100];
    sprintf(compile_command, "g++ -w %s 2> compile_%d.txt", filename, local_thread);
  char runtime_command[100];
    sprintf(runtime_command, "./a.out > out_%d.txt 2> runtime_%d.txt", local_thread, local_thread);
  char diff_command[100];
    sprintf(diff_command, "diff out_%d output-sample.txt > diff_%d.txt", local_thread, local_thread);

  recv_file(newsockfd, filename);
    //close(newsockfd);
    //continue;
  //}
  send(newsockfd, "I got your code file for grading\n", 33, 0);
  
  int n;
  if (system(compile_command) !=0 ) {
    n = send(newsockfd, "COMPILER ERROR\n", 15, 0);
	  if (n < 0)
	    error("ERROR writing to socket");	
 		close(newsockfd);	
 	} 
  else if ( system(runtime_command) !=0 ) { 			
 		n = send(newsockfd, "RUNTIME ERROR\n", 14, 0);
	  if (n < 0)
		  error("ERROR writing to socket");	
 		close(newsockfd);	
 	}
 	else { 		 
    n = send(newsockfd, "PROGRAM RAN\n", 12, 0);
	  if (n < 0)
		  error("ERROR writing to socket");	
 		close(newsockfd);	
	}
}

void *perform_task () {
  int curr_socket;
  while (true) {
		pthread_mutex_lock(&mutex);
      while(front == -1){ 			
        pthread_cond_wait(&cond, &mutex);
      }
      curr_socket = queue_delete();
		pthread_mutex_unlock(&mutex);		
    process_task(curr_socket);
	}
	//pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  int sockfd, newsockfd, portno; 

  socklen_t clilen; 
  char buffer[256]; 
  struct sockaddr_in serv_addr, cli_addr; 
  int n;

  if (argc < 3) {
    fprintf(stderr, "Usage : ./server <portno> <poolsize>");
    exit(1);
  }

  /* create socket */
  sockfd = socket(AF_INET, SOCK_STREAM, 0); 
  if (sockfd < 0)
    error("ERROR opening socket");
  bzero((char *)&serv_addr, sizeof(serv_addr)); 
  
  serv_addr.sin_family = AF_INET; 
  serv_addr.sin_addr.s_addr = INADDR_ANY;  

  portno = atoi(argv[1]);
  serv_addr.sin_port = htons(portno);  

  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  listen(sockfd, qsize);

  clilen = sizeof(cli_addr);

  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond, NULL);
  myq = (int*)malloc(qsize*sizeof(int));
  pool_size = atoi(argv[2]);
  threads = (pthread_t*)(malloc(pool_size * sizeof(pthread_t)));

  for (int i = 0; i < pool_size; i++)				// start all threads
    pthread_create(&threads[i], NULL, perform_task, NULL);

  while (1){
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    add_task(newsockfd);
    if (newsockfd < 0)
      error("ERROR on accept");
  }
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond);	
  return 0;
}
