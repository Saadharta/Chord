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

#define USR_IS_DUMB 1
//networking
#define MSG_MAX_SIZE 32
#define BUF_SIZE 1000
#define PORT_DEFAULT 53

#ifdef DEBUG
    #define log_debug(X, args) do { printf(X, ## __VA_ARGS__); } 
#else
    #define log_debug(X, args)    { }
#endif


/*-------------------------*/
/* Reception and Treatment */
/*-------------------------*/

/* insertion */
uint hello(token t, routing r, address self, uint req_id);
void hello_ok(token t, routing r, address self);

/*Node managment*/
void get(token t, routing r, address self);
void answer(token t, routing r, address self);
void put(token t, routing r, address self);
uint ack(token t, routing r, address self);

/* Routing updates */
void get_resp(token t, routing r, address self);
void ans_resp(token t, routing r, address self);
void update_table(token t, routing r, address self);

/* Stats */
void print(token t, routing r, address self);
void get_stat(token t, routing r, address self);


/*--------------*/
/* Transmission */
/*--------------*/

/* Insertion */
void send_hello(address src, address dest);
void send_hello_ok(uint id, address resp_addr, node d, address prev, uint req_id, address dest);
void send_hello_ko(uint id, address dest, address self);

/* Node managment */
void send_get(address src, uint k, address dest, address self);
void send_answer(uint k, float v, char *exists, address dest, address self);
void send_put(address src, uint k, float v, uint req_id, address dest, address self);
void send_ack(uint req_id, address dest, address self);

/* Routing updates */
void send_get_resp(address src, uint key, address dest, address self);
void send_ans_resp(uint key, address src, address dest);
void send_update_table(address src, uint lower_key, uint amount, address dest, address self);

/* Stats */
void send_print(address src, address dest, node n); 
void send_get_stat(address src, int msg_get, int msg_put, int msg_gest, address dest);
int end();

/* General purpose */
int is_between(uint a, uint b, uint t){
    if (a <= t && t <= b)
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
