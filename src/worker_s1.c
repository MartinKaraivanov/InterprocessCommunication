/* 
 * Operating Systems  (2INCO)  Practical Assignment
 * Interprocess Communication
 *
 * Marwaan El Majdoub 1817078
 * Martin Karaivanov 1953826
 * Hristo Kanev 1963937
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
#include "service1.h"

static void rsleep (int t);

int main (int argc, char * argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <S1_queue_name> <Rsp_queue_name>\n", argv[0]);
        exit(1);
    }

    char *mq_name_s1 = argv[1];

    mqd_t mq_s1 = mq_open(mq_name_s1, O_RDONLY);
    if (mq_s1 == (mqd_t)-1) {
        perror("Error S1 message queue");
        exit(1);
    }
    struct mq_attr a;
    if(mq_getattr(mq_s1, &a)){
        perror("Error S1 message queue");
        exit(1);
    }

    //printf("Worker service 1 connected to queue: %s\n", mq_name_s1);

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

    //printf("Resp connected to queue: %s\n", mq_name_rsp);

    MQ_MESSAGE msg;

    while (true) {
        struct timeval right_now;
        gettimeofday(&right_now, NULL);
        struct timespec timeout;
        timeout.tv_sec = right_now.tv_sec + 10;
        timeout.tv_nsec = right_now.tv_usec * 1000;

        if (mq_timedreceive(mq_s1, (char*)&msg, sizeof(msg), NULL, &timeout) == -1) {
            //printf("A worker 1 didn't receive anything, exiting...\n");
            break;
        }

        // Simulate some random time
        rsleep(10000);

        // Process the job
        //printf("Worker 1 processing job: ID=%d, data=%d\n", msg.id, msg.data);
        int res = service(msg.data);

        // Response
        msg.data = res;

        // Send the response
        if (mq_send(mq_rsp, (char*)&msg, sizeof(msg), 0) == -1) {
            perror("Error sending response to Rsp queue");
            break;
        }

        //printf("Worker 1 sent response: %d, %d\n\n", msg.id, msg.data);
    }

    mq_close(mq_s1);
    mq_close(mq_rsp);

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
