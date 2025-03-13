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

#ifndef MESSAGES_H
#define MESSAGES_H

typedef struct {
    int id;
    int data;
    int service;
} MQ_MESSAGE;

#endif
