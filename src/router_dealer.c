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

mqd_t mq_req, mq_s1, mq_s2, mq_rep;

static void create_message_queues() {
  struct mq_attr attr;

  attr.mq_maxmsg = MQ_MAX_MESSAGES;
  attr.mq_msgsize = sizeof(MQ_REQUEST_MESSAGE);

  // Request queue
  mq_req = mq_open(client2dealer_name, O_CREAT | O_RDWR | O_EXCL | O_NONBLOCK, 0600, &attr);
  if (mq_req == (mqd_t)-1) {
      perror("mq_open() failed for Req_queue");
      exit(1);
  }

  // Service queues
  mq_s1 = mq_open(dealer2worker1_name, O_CREAT | O_RDWR | O_EXCL | O_NONBLOCK, 0600, &attr);
  if (mq_s1 == (mqd_t)-1) {
      perror("mq_open() failed for S1_queue");
      exit(1);
  }

  mq_s2 = mq_open(dealer2worker2_name, O_CREAT | O_RDWR | O_EXCL | O_NONBLOCK, 0600, &attr);
  if (mq_s2 == (mqd_t)-1) {
      perror("mq_open() failed for S2_queue");
      exit(1);
  }

  // Response queue
  attr.mq_maxmsg = MQ_MAX_MESSAGES;
  attr.mq_msgsize = sizeof(MQ_RESPONSE_MESSAGE);
  mq_rep = mq_open(worker2dealer_name, O_CREAT | O_RDWR | O_EXCL | O_NONBLOCK, 0600, &attr);
  if (mq_rep == (mqd_t)-1) {
      perror("mq_open() failed for Rep_queue");
      exit(1);
  }

  printf("Message queues created successfully.\n");
}


static void delete_message_queues() {
  if (mq_unlink(client2dealer_name) == -1) {
      perror("mq_unlink() failed for Req_queue");
  }

  if (mq_unlink(dealer2worker1_name) == -1) {
      perror("mq_unlink() failed for S1_queue");
  }

  if (mq_unlink(dealer2worker2_name) == -1) {
      perror("mq_unlink() failed for S2_queue");
  }

  if (mq_unlink(worker2dealer_name) == -1) {
      perror("mq_unlink() failed for Rep_queue");
  }

  printf("Message queues deleted successfully.\n");
}

static void routing_requests(){
  MQ_REQUEST_MESSAGE req;
  while (1) {
    // Read a request from the client
    ssize_t bytes_received = mq_receive(mq_req, (char*)&req, sizeof(req), NULL);
    if (bytes_received == -1) {
        break;
    }

    // Distribute to the corresponding service queue based on the serviceID
    if (req.c == 1) {
        if (mq_send(mq_s1, (char*)&req, sizeof(req), 0) == -1) {
            perror("mq_send() failed for S1_queue");
            break;
        }
    } else if (req.c == 2) {
        if (mq_send(mq_s2, (char*)&req, sizeof(req), 0) == -1) {
            perror("mq_send() failed for S2_queue");
            break;
        }
    } else {
        fprintf(stderr, "Invalid service ID: %d\n", req.c);
    }
  }
}

static void client_process(){
  pid_t client_pid;

  client_pid = fork();
  if(client_pid < 0){
    perror("fork failed");
    exit(1);
  }else if(client_pid == 0){
    printf("Client process started with PID: %d\n", getpid());
    execlp("./client", "client", client2dealer_name, NULL);
    perror("execlp() failed for client");
    exit(1);
  }

  waitpid(client_pid, NULL, 0);

  printf("Client child process has finished.\n");
}

static void worker_processes(){
  pid_t worker1_pid, worker2_pid;

  worker1_pid = fork();
  if (worker1_pid < 0) {
    perror("fork() failed for worker1");
    exit(1);
  } else if (worker1_pid == 0) {
    printf("Worker 1 process started (PID: %d)\n", getpid());
    execlp("./worker_s1", "worker_s1", dealer2worker1_name, worker2dealer_name, NULL);
    perror("execlp() failed for worker1");
    exit(1);
  }

  worker2_pid = fork();
  if (worker2_pid < 0) {
    perror("fork() failed for worker2");
    exit(1);
  } else if (worker2_pid == 0) {
    printf("Worker 2 process started (PID: %d)\n", getpid());
    execlp("./worker_s2", "worker_s2", dealer2worker2_name, worker2dealer_name, NULL);
    perror("execlp() failed for worker2");
    exit(1);
  }

  waitpid(worker1_pid, NULL, 0);
  waitpid(worker2_pid, NULL, 0);

  printf("All worker child processes have finished. Router-dealer exiting.\n");
}

int main (int argc, char * argv[])
{
  if (argc != 1)
  {
    fprintf (stderr, "%s: invalid arguments\n", argv[0]);
  }

  delete_message_queues();

  create_message_queues();

  client_process();

  routing_requests();

  worker_processes();

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
