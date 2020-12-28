#ifndef CHORD_H
#define CHORD_H

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> // inet_addr, htons
#include <time.h>

#include "chord_struct.h"
#include "cmd_splitter.h"

#define MAX_NODES 16
#define MAX_NEIGHBOURS 15 //(log2 (65536))-1
#define USR_IS_DUMB 1
//networking
#define MSG_MAX_SIZE 32
#define BUF_SIZE 600
#define PORT_DEFAULT 53

#ifdef DEBUG
    #define log_debug(X, args) do { printf(X, ## __VA_ARGS__); } 
#else
    #define log_debug(X, args)    { }
#endif

/*<idNode, ip, port> */
typedef struct s_address *address;
/* couple <key , value> */
typedef struct s_node_data *node_data;
/* previous node, lower bound, higher bound and data couples */
typedef struct s_routing *routing;
/* amount of put / get / gestion received */ 
typedef struct s_statistics *statistics;

int id_req = 0;
/* Message types */
void send_get(address src, uint key);
void send_get_ans(address src, uint key, uint value, uint req_id);
void send_put(address src, uint key, float value, uint id, node_data d,routing r);
void send_ack(address src, uint req_id);
/* Insert */
void hello(list l, routing r, address self);
void send_hello(address src, address dest);
void send_hello_ok(uint id, address resp_addr, node_data d, address prev, uint req_id, address dest);
void send_hello_ko(uint id);
/* Update */
void send_get_resp(address src, uint key);
void send_ans_resp(uint key, address src);
void send_update(address src, uint lower_key, uint amount);
/* Stats */
void send_print(address src); 
void send_get_stat(address src, int msg_get, int msg_put, int msg_gest);
int end();

/* General purpose */
int is_between(uint a, uint b, uint t){
    if (a <= t && t < b)
        return 1;
    else 
        if(a > b)
            return(is_between(a,MAX_NODES,t) || is_between(0,b,t));
        else
            return 0;
        
}

int comm_out(address dest, char* buff);
char *comm_in(const address self);
#endif
