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
#include <errno.h>      // for perror()
#include <unistd.h>     // for getpid()
#include <mqueue.h>     // for mq-stuff
#include <time.h>       // for time()
#include <sys/time.h>

#include "messages.h"
#include "service2.h"

static void rsleep (int t);

int main (int argc, char * argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <S2_queue_name> <Rsp_queue_name>\n", argv[0]);
        exit(1);
    }

    char *mq_name_s2 = argv[1];

    mqd_t mq_s2 = mq_open(mq_name_s2, O_RDONLY);
    if (mq_s2 == (mqd_t)-1) {
        perror("Error S2 message queue");
        exit(1);
    }
    struct mq_attr a;
    if(mq_getattr(mq_s2, &a)){
        perror("Error S2 message queue");
        exit(1);
    }

    printf("Worker service 2 connected to queue: %s\n", mq_name_s2);

    char *mq_name_rsp = argv[2];

    mqd_t mq_rsp = mq_open(mq_name_rsp, O_WRONLY | O_NONBLOCK);
    if (mq_rsp == (mqd_t)-1) {
        perror("Error Rsp message queue");
        exit(1);
    }
    if(mq_getattr(mq_rsp, &a)){
        perror("Error Rsp message queue");
        exit(1);
    }

    printf("Resp connected to queue: %s\n", mq_name_rsp);

    MQ_MESSAGE msg;

    while (true) {
        struct timeval right_now;
        gettimeofday(&right_now, NULL);
        struct timespec timeout;
        timeout.tv_sec = right_now.tv_sec + 3;
        timeout.tv_nsec = right_now.tv_usec * 1000;

        if (mq_timedreceive(mq_s2, (char*)&msg, sizeof(msg), NULL, &timeout) == -1) {
            printf("A worker 2 didn't receive anything, exiting...\n");
            break;
        }

        // Simulate some random time
        rsleep(10000);

        // Process the job
        printf("Worker 2 processing job: ID=%d, data=%d\n", msg.id, msg.data);
        int res = service(msg.data);

        // Response
        msg.data = res;

        // Send the response
        if (mq_send(mq_rsp, (char*)&msg, sizeof(msg), 0) == -1) {
            perror("Error sending response to Rsp queue");
            break;
        }

        printf("Worker 2 sent response: %d, %d\n\n", msg.id, msg.data);
    }

    mq_close(mq_s2);
    mq_close(mq_rsp);

    // TODO:
    // (see message_queue_test() in interprocess_basic.c)
    //  * open the two message queues (whose names are provided in the
    //    arguments)
    //  * repeatedly:
    //      - read from the S1 message queue the new job to do
    //      - wait a random amount of time (e.g. rsleep(10000);)
    //      - do the job 
    //      - write the results to the Rsp message queue
    //    until there are no more tasks to do
    //  * close the message queues

    return(0);
}


/*
* rsleep(int t)
*
* The calling thread will be suspended for a random amount of time
* between 0 and t microseconds
* At the first call, the random generator is seeded with the current time
*/

static void rsleep (int t)
{
    static bool first_call = true;
    
    if (first_call == true)
    {
        srandom (time (NULL) % getpid ());
        first_call = false;
    }
    usleep (random() % t);
}
