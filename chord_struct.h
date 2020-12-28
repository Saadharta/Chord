#ifndef CHORD_STRUCT_H
#define CHORD_STRUCT_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/*<idNode, ip, port> */
typedef struct s_address *address;
uint address_id(const address a);
void address_id_update(address a, uint id);
char* address_ip(const address a);
uint address_port(const address a);
address address_create(uint id, char *ip, uint port);
void address_clean(address a);

/* couple <key , value> */
typedef struct s_node_data *node_data;
node_data data_prev(const node_data data);
node_data data_next(const node_data data);
uint data_key(const node_data data);
float data_value(const node_data data);
node_data data_create(uint key, float value);
void data_add(node_data data, node_data new_data);
char *data_print(node_data data);
void data_clean(node_data data);

/* previous node, lower bound, higher bound and data couples */
typedef struct s_routing *routing;
address routing_prev(const routing r);
address routing_next(const routing r);
uint routing_lid(const routing r);
uint routing_hid(const routing r);
node_data routing_values(const routing r);

routing routing_create(address prev, address next, uint lower_id, uint higher_id, node_data values);
void routing_update(routing r, address prev, address next, uint lower_id, uint higher_id, node_data data);
void routing_clean(routing r);

/* amount of put / get / gestion received */ 
typedef struct s_statistics *statistics;


typedef struct s_list *list;
list list_prev(list l);
list list_next(list l);
char* list_value(list l);
int list_size(list l);
list list_create(char *str, int size);
list list_generate(char *chain);
void list_clean(list l);
void list_add(list l, char *str, int size);
void list_print(list l);



#endif
