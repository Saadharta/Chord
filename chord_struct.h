#ifndef CHORD_STRUCT_H
#define CHORD_STRUCT_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define MAX_NEIGHBOURS 16//(log2 (65536))-1
#define MAX_NODES (1<<MAX_NEIGHBOURS)-1


/*----------------*/
/* ADDRESS struct */
/*----------------*/

typedef struct s_address *address;

/**
 * access to the id of a
 * @param a address struct
 * @return id of a
 */
uint address_id(const address a);

/**
 * access to the ip of a
 * @param a address struct
 * @return ip of a
 */
char* address_ip(const address a);

/**
 * access to the port of a
 * @param a current address
 * @return port of the provided address struct
 */
uint address_port(const address a);

/**
 * update the id of a if needed
 * @param a address struct
 * @param id new id of a
 */
void address_id_update(address a, uint id);

/**
 * create a new address
 * @param id id of the future address
 * @param ip ip of the future address
 * @param port port of the future address
 * @return complete address struct
 */
address address_create(uint id, char *ip, uint port);

/**
 * print the contents of a
 * @param a address to print from
 * @return contents of a 
 */
char *address_print(address a);
/**
 * free memory occupied by an address struct
 * @param a adress struct to be free'd
 */
void address_clean(address a);


/*-------------*/
/* NODE struct */
/*-------------*/

typedef struct s_node *node;

/**
 * access to the previous node of n
 * @param n node struct
 * @return previous node of n
 */
node node_prev(const node n);

/**
 * access to the next node of n
 * @param n node struct
 * @return next node of n
 */
node node_next(const node n);

/**
 * access to the key of n
 * @param n node struct
 * @return key of n
 */
uint node_key(const node n);

/**
 * access to the value of n
 * @param n node struct
 * @return value of n
 */
float node_value(const node n);

/**
 * allocate memory and initialize a node
 * @param k key of the new node
 * @param v value of the new node
 * @return new node
 */
node node_create(uint k, float v);

/**
 * add a node to e
 * @param e node list receiving the node n
 * @param n node to add
 */
void node_add_node(node e, node n);

/**
 * add a node list to e
 * @param e node list receiving the nodes of n
 * @param n node list to add
 * @see node_add_node(e,n)
 */
void node_add(node e, node n);

/**
 * split e in two lists
 * @param e node list to split
 * @param n node list receiving the splitted values 
 */
void node_split(node e, node n);

/**
 * search if n is in e
 * @param e node list to search
 * @param k key searched
 * @return the value of k if found else -1
 */
float node_find(node e, uint k);

/**
 * @param n node list 
 * @return string composed of the couples key:value of every node of the node list provided
 */
char *node_print(node n);

/**
 * @param n entry point of the node list 
 */
void node_clean(node n);


/*----------------*/
/* ROUTING struct */
/*----------------*/

typedef struct s_routing *routing;

/**
 * @param r routing struct 
 * @return address of previous owner node on the newtork 
 */
address routing_prev(const routing r);

/**
 * @param r routing struct 
 * @return address of next owner node on the newtork 
 */
address routing_next(const routing r);

/**
 * @param r routing struct 
 * @return address of self
 */
address routing_self(const routing r);

/**
 * @param r routing struct 
 * @return key of the smallest owned node 
 */
uint routing_lid(const routing r);

/**
 * @param r routing struct 
 * @return key of the highest owned node 
 */
uint routing_hid(const routing r);

/**
 * @param r routing struct 
 * @return amount of owned nodes 
 */
uint routing_amount(const routing r);

/**
 * @param r routing struct 
 * @return list of all owned nodes and their value
 */
node routing_values(routing r);

/**
 * return the owner of the N+2^i th node
 * @param r routing struct
 * @param i index of the N+2^i th node 
 * @return address of owner
 */
address routing_get_fg(routing r, int i);

/**
 * search for the higher inferior owner of N+2^i 
 * @param r routing struct
 * @param i index of the N+2^i th node
 * @return address of higher inferior owner of N+2^i
 */
address routing_cls_fg(routing r, uint i);

/**
 * @param r routing struct
 * @param k key which need to update it's owner
 * @param a address struct of the owner of the N+2^i node
 */
void routing_set_fg(routing r, int k, address a);

/**
 * allocate memory and fill a routing struct with the provided informations
 * @param prev address of the previous owner node; can be self if there is only one owner node in the network
 * @param next address of the next owner node; can be self if there is only one owner node in the network
 * @param self address of self
 * @param lower_id key of the lowest owned node
 * @param higher_id key of the highest owned node
 * @param n list of all owned nodes and their value
 * @return a routing struct accurately filled 
 */
routing routing_create(address prev, address next, address self, uint lower_id, uint higher_id, node n);

/**
 * update the provided routing struct
 * @param prev address of the previous owner node; set to NULL if there's nothing to do
 * @param next address of the next owner node; set to NULL if there's nothing to do
 * @param lower_id key of the lowest owned node, set to -1 if there's nothing to do
 * @param higher_id key of the highest owned node, set to -1 if there's nothing to do
 * @param n list of all owned nodes and their value
 * @return a routing struct accurately filled 
 */
void routing_update(routing r, address prev, address next, int lower_id, int higher_id, node n);

/**
 * display the content of the routing struct
 * @param r routing struct to print
 */
void routing_print(routing r);

/**
 * free everything from the provided routing struct
 * @param r routing struct to clean 
 */
void routing_clean(routing r);


/*-------------------*/
/* STATISTICS struct */
/*-------------------*/

typedef struct s_statistics *statistics;

/**
 * allocate memory and fill a new statistic
 * @return a new statistic struct 
 */
statistics stat_create();

/**
 * provide get statistics
 * @param s stat struct to access
 * @return stats for get
 */
int stat_get(statistics s);

/**
 * provide put statistics
 * @param s stat struct to access
 * @return stats for get
 */
int stat_put(statistics s);

/**
 * provide gestion statistics
 * @param s stat struct to access
 * @return stats for get
 */
int stat_gst(statistics s);

/**
 * print statistics
 * @param s stat struct to display 
 */
void stat_print(statistics s);

/**
 * increase get statistics
 * @param s stat struct to increase 
 */
void stat_iget(statistics s);

/**
 * increase put statistics
 * @param s stat struct to increase 
 */
void stat_iput(statistics s);

/**
 * increase gestion statistics
 * @param s stat struct to increase 
 */
void stat_igst(statistics s);

/**
 * free a statistic struct
 * @param s stat struct to free
 */
void stat_clean(statistics s);
/*--------------*/
/* TOKEN struct */
/*--------------*/

typedef struct s_token *token;

/**
 * @param t current token 
 * @return previous token
 */
token token_prev(token t);

/**
 * @param t current token
 * @return next token
 */
token token_next(token t);

/**
 * @param t current token
 * @return value of the current token
 */
char* token_value(token t);

/**
 * @param t
 * @return size of the value of t
 */
int token_size(token t);

/**
 * allocate memory and fill a new token
 * @param str value of the future token
 * @param size size of str
 * @return a new token
 */
token token_create(char *str, int size);

/**
 * chenerate a token chain from the provided input
 * @param chain
 * @return list of tokens
 */
token token_generate(char *chain);

/**
 * free the whole token chain
 * @param t token chain to free
 */
void token_clean(token t);

/**
 * add a new token into the prvided token chain
 * @param t existing token chain
 * @param str value of the new token
 * @param size saze of the new token
 * @see token_create(str, size)
 */
void token_add(token t, char *str, int size);

/**
 * print all the values of the provided token chain; 
 * there must be at least one existing token;
 * @param t existing token chain
 */
void token_print(token t);

#endif