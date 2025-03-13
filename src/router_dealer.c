/* 
 * Operating Systems  (2INCO)  Practical Assignment
 * Interprocess Communication
 *
 * STUDENT_NAME_1 (STUDENT_NR_1)
 * STUDENT_NAME_2 (STUDENT_NR_2)
 *
 * Grading:
 * Your work will be evaluated based on the following criteria:
 * - Satisfaction of all the specifications
 * - Correctness of the program
 * - Coding style
 * - Report quality
 * - Deadlock analysis
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>    
#include <unistd.h>    // for execlp
#include <mqueue.h>    // for mq


#include "settings.h"  
#include "messages.h"

char client2dealer_name[30] = "/Req_queue_group98";
char dealer2worker1_name[30] = "/S1_queue_group98";
char dealer2worker2_name[30] = "/S2_queue_group98";
char worker2dealer_name[30] = "/Rsp_queue_group98";

mqd_t mq_req, mq_s1, mq_s2, mq_rsp;

pid_t client_pid, workers_pid[N_SERV1 + N_SERV2 + 1];

static void create_processes(){
  
  client_pid = fork();
  if(client_pid < 0){
    perror("fork failed");
    exit(1);
  }else if(client_pid == 0){
    printf("Client process started with PID: %d\n", getpid());
    execlp("./client", "./client", client2dealer_name, NULL);
    perror("execlp() failed for client");
    exit(1);
  }

  pid_t pid;
  for(int i = 0; i < N_SERV1; i++){
    pid = fork();
    if (pid < 0) {
      perror("fork() failed for worker1");
      exit(1);
    } else if (pid == 0) {
      printf("Worker 1 process started (PID: %d)\n", getpid());
      execlp("./worker_s1", "./worker_s1", dealer2worker1_name, worker2dealer_name, NULL);
      perror("execlp() failed for worker1");
      exit(1);
    }

    workers_pid[i] = pid;
  }

  for(int i = N_SERV1; i < N_SERV2; i++){
    pid = fork();
    if (pid < 0) {
      perror("fork() failed for worker2");
      exit(1);
    } else if (pid == 0) {
      printf("Worker 2 process started (PID: %d)\n", getpid());
      execlp("./worker_s2", "./worker_s2", dealer2worker2_name, worker2dealer_name, NULL);
      perror("execlp() failed for worker2");
      exit(1);
    }

    workers_pid[i] = pid;
  }
}

static void create_message_queues() {
  struct mq_attr attr;

  attr.mq_maxmsg = MQ_MAX_MESSAGES;
  attr.mq_msgsize = sizeof(MQ_REQUEST_MESSAGE);

  // Request queue
  mq_req = mq_open(client2dealer_name, O_CREAT | O_RDONLY | O_EXCL | O_NONBLOCK, 0600, &attr);
  if (mq_req == (mqd_t)-1) {
      perror("mq_open() failed for Req_queue");
      exit(1);
  }
  if (mq_getattr(mq_req, &attr) == -1)
  {
    fprintf(stderr, "mq_getattr(%s) failed: ", client2dealer_name);
    perror("");
    exit (1);
  }

  // Service queues
  mq_s1 = mq_open(dealer2worker1_name, O_CREAT | O_WRONLY | O_EXCL | O_NONBLOCK, 0600, &attr);
  if (mq_s1 == (mqd_t)-1) {
      perror("mq_open() failed for S1_queue");
      exit(1);
  }
  if (mq_getattr(mq_s1, &attr) == -1)
  {
    fprintf(stderr, "mq_getattr(%s) failed: ", dealer2worker1_name);
    perror("");
    exit (1);
  }

  mq_s2 = mq_open(dealer2worker2_name, O_CREAT | O_WRONLY | O_EXCL | O_NONBLOCK, 0600, &attr);
  if (mq_s2 == (mqd_t)-1) {
      perror("mq_open() failed for S2_queue");
      exit(1);
  }
  if (mq_getattr(mq_s2, &attr) == -1)
  {
    fprintf(stderr, "mq_getattr(%s) failed: ", dealer2worker2_name);
    perror("");
    exit (1);
  }

  // Response queue
  attr.mq_maxmsg = MQ_MAX_MESSAGES;
  attr.mq_msgsize = sizeof(MQ_RESPONSE_MESSAGE);
  mq_rsp = mq_open(worker2dealer_name, O_CREAT | O_RDONLY | O_EXCL | O_NONBLOCK, 0600, &attr);
  if (mq_rsp == (mqd_t)-1) {
      perror("mq_open() failed for Rsp_queue");
      exit(1);
  }
  if (mq_getattr(mq_rsp, &attr) == -1)
  {
    fprintf(stderr, "mq_getattr(%s) failed: ", worker2dealer_name);
    perror("");
    exit (1);
  }

  printf("Message queues created successfully.\n");
}


static void delete_message_queues() {
  if (mq_close(mq_req) == -1) {
    perror("mq_close() failed for Req_queue");
  }
  if (mq_unlink(client2dealer_name) == -1) {
    perror("mq_unlink() failed for Req_queue");
  }

  if (mq_close(mq_s1) == -1) {
    perror("mq_close() failed for S1_queue");
  }
  if (mq_unlink(dealer2worker1_name) == -1) {
    perror("mq_unlink() failed for S1_queue");
  }

  if (mq_close(mq_s2) == -1) {
    perror("mq_close() failed for S2_queue");
  }
  if (mq_unlink(dealer2worker2_name) == -1) {
    perror("mq_unlink() failed for S2_queue");
  }

  if (mq_close(mq_rsp) == -1) {
    perror("mq_close() failed for Rsp_queue");
  }
  if (mq_unlink(worker2dealer_name) == -1) {
    perror("mq_unlink() failed for Rsp_queue");
  }

  printf("Message queues deleted successfully.\n");
}

bool attempt_msg_receive(mqd_t queue, MQ_REQUEST_MESSAGE *msg) {
  int res = mq_receive(queue, (char*)msg, sizeof(*msg), NULL);

  if(res == -1) {
    if(errno == EAGAIN) {
      return false;
    }
    perror("Cannot receive a message");
    exit(1);
  }

  return true;
}


static void route_request(MQ_REQUEST_MESSAGE msg){
  int res = -1;
  
  if(msg.service == 1) {
    res = mq_send(mq_s1, (char*)&msg, sizeof(msg), 0);
  } else if(msg.service == 2) {
    res = mq_send(mq_s2, (char*)&msg, sizeof(msg), 0);
  } else {
    fprintf(stderr, "Invalid service id: %d\n", msg.service);
  }

  if(res == -1) {
    perror("Cannot route request");
    exit(1);
  }

  printf("Routed request %d to a worker of service %d\n", msg.id, msg.service);
}

bool output_result() {
  MQ_REQUEST_MESSAGE msg;
  bool res = attempt_msg_receive(mq_rsp, &msg);
  if(res == true) {
    fprintf(stdout, "%d -> %d\n", msg.id, msg.data);
  }

  return res;
}

static void run() {
  
  while(waitpid(client_pid, NULL, WNOHANG) == 0) {
    MQ_REQUEST_MESSAGE msg;
    if(attempt_msg_receive(mq_req, &msg)) {
      route_request(msg);
    }

    output_result();
  }

  int n = N_SERV1 + N_SERV2;
  int k = n;

  while(k > 0) {
    output_result();

    for(int i = 0; i < n; i++) {
      if(workers_pid[i] > 0) {
        if(waitpid(workers_pid[i], NULL, WNOHANG) != 0) {
          workers_pid[i] = -1;
          k--;
        }
      }
    }
  }

}

int main (int argc, char * argv[])
{
  if (argc != 1)
  {
    fprintf (stderr, "%s: invalid arguments\n", argv[0]);
  }

  delete_message_queues();

  create_message_queues();

  create_processes();

  run();

  delete_message_queues();
  
  // TODO:
    //  * create the message queues (see message_queue_test() in
    //    interprocess_basic.c)
    //  * create the child processes (see process_test() and
    //    message_queue_test())
    //  * read requests from the Req queue and transfer them to the workers
    //    with the Sx queues
    //  * read answers from workers in the Rep queue and print them
    //  * wait until the client has been stopped (see process_test())
    //  * clean up the message queues (see message_queue_test())

    // Important notice: make sure that the names of the message queues
    // contain your goup number (to ensure uniqueness during testing)
  
  return (0);
}
