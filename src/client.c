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

#include "messages.h"
#include "request.h"

static void rsleep (int t){
    usleep(t * 1000);
}


int main (int argc, char * argv[])
{
    if(argc != 2){
        fprintf(stderr, "Usage: %s <message_queue_name>\n", argv[0]);
        exit(1);
    }

    char *mq_name = argv[1];

    mqd_t mq_fd = mq_open(mq_name, O_WRONLY);
    if(mq_fd == -1){
        perror("mq_open() failed");
        exit(1);
    }

    printf("Client connected to queue: %s\n", mq_name);

    Request req;
    while (getNextRequest(&req.job, &req.data, &req.service) == NO_ERR) {
        printf("Client: Sending request (ID: %d, Data: %d, Service: %d)\n", req.job, req.data, req.service);

        if (mq_send(mq_fd, (char *)&req, sizeof(req), 0) == -1) {
            perror("mq_send() failed");
            break;
        }

        rsleep(500); // Some delay between requests
    }

    mq_close(mq_fd);
    
    printf("Client: Finished sending requests. Exiting.\n");

    // TODO:
    // (see message_queue_test() in interprocess_basic.c)
    //  * open the message queue (whose name is provided in the
    //    arguments)
    //  * repeatingly:
    //      - get the next job request 
    //      - send the request to the Req message queue
    //    until there are no more requests to send
    //  * close the message queue
    
    return (0);
}
