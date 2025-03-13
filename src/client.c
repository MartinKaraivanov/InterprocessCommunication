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

    //printf("Client connected to queue: %s\n", mq_name);

    Request req;
    while (getNextRequest(&req.job, &req.data, &req.service) == NO_ERR) {
        //printf("Client: Sending request (ID: %d, Data: %d, Service: %d)\n", req.job, req.data, req.service);

        if (mq_send(mq_fd, (char *)&req, sizeof(req), 0) == -1) {
            perror("mq_send() failed");
            break;
        }

        rsleep(500); // Some delay between requests
    }

    mq_close(mq_fd);
    
    //printf("Client: Finished sending requests. Exiting.\n");
    
    return (0);
}
