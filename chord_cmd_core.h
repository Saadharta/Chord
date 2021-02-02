#ifndef CHORD_CMD_CORE
#define CHORD_CMD_CORE

#include "chord_struct.h"
#include "chord_comm.h"

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

/* insertion */
/**
 * Manage the reception of a hello request
 * @param t token list
 * @param r routing struct of self 
 * @param req_id id of the received request
 * @return request id 
 */
uint hello(token t, routing r, uint req_id);

/**
 * manage the acknowlegde of a correct insertion
 * @param t token list
 * @param r routing struct of self 
 */
void hello_ok(token t, routing r);

/*Node managment*/
/**
 * @param t token list
 * @param r routing struct of self 
 */
void get(token t, routing r);

/**
 * @param t token list
 * @param r routing struct of self 
 */
void answer(token t, routing r);

/**
 * @param t token list
 * @param r routing struct of self 
 */
void put(token t, routing r);

/**
 * @param t token list
 * @param r routing struct of self 
 */
uint ack(token t, routing r);

/* Routing updates */
/**
 * @param t token list
 * @param r routing struct of self 
 */
void get_resp(token t, routing r);

/**
 * @param t token list
 * @param r routing struct of self 
 */
void ans_resp(token t, routing r);

/**
 * @param t token list
 * @param r routing struct of self 
 */
void update_table(token t, routing r);

/* Stats */
/**
 * display self statistics
 * @param t token list
 * @param r routing struct of self 
 * @param s statistic struct of self
 */
void print(token t, routing r, statistics s);

/**
 * transfer to the next node a set of statistics enriched with self stats
 * @param t token list
 * @param r routing struct of self 
 * @param s statistic struct of self
 */
void get_stat(token t, routing r, statistics s);

/**
 * transfer own dictionnary to next node and update prev / next pointers before termination
 * @param r routing struct of self 
 */
void terminate(routing r);

#endif
