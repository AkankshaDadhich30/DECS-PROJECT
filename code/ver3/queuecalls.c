#include<stdio.h>
#include "queuecalls.h"

void queue_insert(int sockfd) {
  rear = (rear+1)%qsize;
  if(front == rear)
    printf("Overflow");
  else
    myq[rear] = sockfd;
  if (front == -1)
    front = 0;
}

int queue_delete(){
  if(front == -1)
    return -1;
  int item = myq[front];
  if(front == rear) {
    front = -1;
    rear = -1;
  } else {
    front = (front+1)%qsize;
  }
  return item;
}