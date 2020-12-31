#ifndef CMD_SPLITTER_H
#define CMD_SPLITTER_H

#include "chord_struct.h"

#define HELLO 1
#define HELLO_OK 2
#define HELLO_KO 3
#define GET 4
#define ANSWER 5
#define PUT 6
#define ACK 7
#define GET_RESP 8
#define ANSWER_RESP 9
#define UPDATE_TABLE 10
#define PRINT 11
#define GET_STAT 12
#define TERMINATE 13

int cmd_sort(token t);

#endif
