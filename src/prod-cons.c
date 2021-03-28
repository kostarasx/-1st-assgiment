#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define QUEUESIZE 1000
#define LOOP 1000
#define P 20
#define Q 20

int priorityNumber = 0;
long sum = 0;

void* producer(void* args);
void* consumer(void* args);

typedef struct workFunction {
  void* (*work)(void*);
  void* arg;
}workFunction;

typedef struct {
  struct workFunction* buf[QUEUESIZE];
  struct timeval tv[QUEUESIZE];
  struct timezone tz[QUEUESIZE];


  long head, tail;
  int full, empty;
  pthread_mutex_t* mut;
  pthread_cond_t* notFull, * notEmpty;
} queue;

void printNumberLine(int number);

queue* queueInit(void);
void queueDelete(queue* q);
void queueAdd(queue* q);
void queueDel(queue* q);

int main()
{
  queue* fifo;
  pthread_t pro[P], con[Q];

  fifo = queueInit();
  if (fifo == NULL) {
    fprintf(stderr, "main: Queue Init failed.\n");
    exit(1);
  }

  int i = 0, j = 0;
  while (1) {
    if (i < P) {
      pthread_create(&pro[i], NULL, producer, fifo);
      i++;
    }
    if (j < Q) {
      pthread_create(&con[j], NULL, consumer, fifo);
      j++;
    }
    if (i == P && j == Q) {
      break;
    }
  }
  i = 0;
  j = 0;
  while (1) {
    if (i < P) {
      pthread_join(pro[i], NULL);
      i++;
    }
    if (j < Q) {
      pthread_join(con[j], NULL);
      j++;
    }

    if (i == P && j == Q) {
      break;
    }
  }

  queueDelete(fifo);
  printf("Sum = %ld\n", sum);
  int avg = sum / (P * LOOP);
  printf("Average time = %d ms\n", avg);
  return 0;
}

void* producer(void* q)
{
  queue* fifo;
  int i;

  fifo = (queue*)q;


  for (i = 0; i < LOOP; i++) {

    pthread_mutex_lock(fifo->mut);
    while (fifo->full) {
      //printf("producer: queue FULL.\n");
      pthread_cond_wait(fifo->notFull, fifo->mut);
    }
    priorityNumber++;
    queueAdd(fifo);
    pthread_mutex_unlock(fifo->mut);
    pthread_cond_broadcast(fifo->notEmpty);
  }

  return (NULL);
}

void* consumer(void* q)
{
  queue* fifo;
  fifo = (queue*)q;
  int flag = 0;
  while (1) {

    pthread_mutex_lock(fifo->mut);
    while (fifo->empty) {
      if (priorityNumber == (P * LOOP)) {
        flag = 1;
        break;
      }
      //printf("consumer: queue EMPTY.\n");
      pthread_cond_wait(fifo->notEmpty, fifo->mut);
    }
    if (flag == 1) {
      pthread_mutex_unlock(fifo->mut);
      break;
    }

    queueDel(fifo);
    pthread_mutex_unlock(fifo->mut);
    pthread_cond_broadcast(fifo->notFull);
  }
  return (NULL);
}

queue* queueInit(void)
{
  queue* q;

  q = (queue*)malloc(sizeof(queue));
  if (q == NULL) return (NULL);
  for (int i = 0; i < QUEUESIZE; i++) {
    q->buf[i] = (workFunction*)malloc(sizeof(workFunction));
    if (q->buf[i] == NULL) return (NULL);
    q->buf[i]->arg = (int*)malloc(sizeof(int));
    if (q->buf[i]->arg == NULL) return (NULL);
  }


  q->empty = 1;
  q->full = 0;
  q->head = 0;
  q->tail = 0;

  q->mut = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(q->mut, NULL);
  q->notFull = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
  pthread_cond_init(q->notFull, NULL);
  q->notEmpty = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
  pthread_cond_init(q->notEmpty, NULL);

  return (q);
}

void queueDelete(queue* q)
{
  pthread_mutex_destroy(q->mut);
  free(q->mut);
  pthread_cond_destroy(q->notFull);
  free(q->notFull);
  pthread_cond_destroy(q->notEmpty);
  free(q->notEmpty);
  for (int i = 0; i < QUEUESIZE; i++) {
    free(q->buf[i]);
  }
  free(q);
}

void queueAdd(queue* q)
{
  gettimeofday(&(q->tv[q->tail]), NULL);
  q->buf[q->tail]->work = (void*)printNumberLine;
  (q->buf[q->tail]->arg) = priorityNumber;
  printf("%d Added to the q\n", priorityNumber);
  q->tail++;
  if (q->tail == QUEUESIZE)
    q->tail = 0;
  if (q->tail == q->head)
    q->full = 1;
  q->empty = 0;
  return;
}

void queueDel(queue* q)
{
  int  sec1 = (q->tv[q->head].tv_sec);
  int  start = (q->tv[q->head].tv_usec);
  gettimeofday(&(q->tv[q->head]), NULL);
  int  end = (q->tv[q->head].tv_usec);
  int  sec2 = (q->tv[q->head].tv_sec);
  int time = (end + sec2 * 1000000) - (start + sec1 * 1000000);
  // printf("%dms passed\n", time);
  sum = sum + time;
  (q->buf[q->head]->work)(q->buf[q->head]->arg);
  q->head++;
  if (q->head == QUEUESIZE)
    q->head = 0;
  if (q->head == q->tail)
    q->empty = 1;
  q->full = 0;

  return;
}

void printNumberLine(int number) {

  printf("Finished working with %d number\n", number);
}
