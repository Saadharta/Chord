#ifndef CHORD_COMM
#define CHORD_COMM

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> // inet_addr, htons
#include "chord_struct.h"

//networking
#define MSG_MAX_SIZE 32
#define BUF_SIZE 1000
#define PORT_DEFAULT 53

/*--------------*/
/* Transmission */
/*--------------*/

/**
 * send a json type message
 * @param dest address of destination node
 * @param buff message to send
 */
int comm_out(address dest, char* buff);

/**
 * receive a json type message
 * @param self address of self node
 * @param sock_id id of self socket
 */
char *comm_in(const address self, int sock_id);

/* Insertion */
/**
 * ask for insertion into the network
 * @param src address of message source
 * @param dest address of know node
 * @param self addess of self node
 */
void send_hello(address src, address dest, address self);

/**
 * answer a correct insertion of source nodeinto the network
 * @param id id of the source node in the network
 * @param resp_addr address of source node new next
 * @param d list of nodes under the responsibility of source node
 * @param prev address of source new prev
 * @param req_id id of the hello_ok request
 * @param dest address of destination node
 */
void send_hello_ok(uint id, address resp_addr, node d, address prev, uint req_id, address dest);

/**
 * answer a failed insertion of source nodeinto the network
 * @param id id of the source node in the network
 * @param dest address of destination node
 * @param self id of the node owning source node' expected identifier
 */
void send_hello_ko(uint id, address dest, address self);

/* Node managment */

/**
 * ask for a value
 * @param src address of message source
 * @param k key requested
 * @param dest address of destination node
 * @param self addess of self node
 */
void send_get(address src, uint k, address dest, address self);

/**
 * answers if value exists or not
 * @param k key requested
 * @param v value of the key
 * @param exists "true" of "false"
 * @param dest address of destination node
 * @param self addess of self node
 */
void send_answer(uint k, float v, char *exists, address dest, address self);

/**
 * ask to insert a couple key/value
 * @param src address of source node
 * @param k key requested
 * @param v value of the key
 * @param req_id request id
 * @param dest address of destination node
 * @param self addess of self node
 */
void send_put(address src, uint k, float v, uint req_id, address dest, address self);

/**
 * notify that the put has been made
 * @param req_id request id
 * @param dest address of destination node
 * @param self addess of self node
 */
void send_ack(uint req_id, address dest, address self);

/* Routing updates */

/**
 * ask for the node owning a key
 * @param src address of source node
 * @param k key requested
 * @param dest address of destination node
 * @param self addess of self node
 */
void send_get_resp(address src, uint key, address dest, address self);

/**
 * answer as owner of requested key
 * @param k key requested
 * @param src address of self node
 * @param dest address of destination node
 */
void send_ans_resp(uint key, address src, address dest);

/**
 * and a request to update destination node's table
 * @param src address of source node
 * @param lower_key lowest key possible
 * @param amount quantities of keys
 * @param dest address of destination node
 * @param self address of self node 
 */
void send_update_table(address src, uint lower_key, uint amount, address dest, address self);

/* Stats */

/**
 * answer as owner of requested key
 * @param src address of source node
 * @param dest address of destination node
 * @param self address of self node 
 */
void send_print(address src, address dest, address self); 

/**
 * answer as owner of requested key
 * @param src address of source node
 * @param msg_get amount of get messages 
 * @param msg_put amount of put messages 
 * @param msg_gst amount of others messages 
 * @param dest address of destination node
 * @param self address of self node 
 */
void send_get_stat(address src, int msg_get, int msg_put, int msg_gst, address dest, address self);

#endif
