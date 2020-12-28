#include "chord_struct.h"


struct s_address{
    uint id;
    char ip[16];
    uint port;
};
uint address_id(const address a){
    return a->id;
}

void address_id_update(address a, uint id){
    a->id = id;
    return;
}
char* address_ip(const address a){
    return a->ip;
}
uint address_port(const address a){
    return a->port;
}

address address_create(uint id, char *ip, uint port){
    address a= malloc(sizeof(struct s_address));
    a->id = id;
    strcpy(a->ip, ip);
    a->port = port;
    return a;
}

void address_clean(address a){
    free(a);
}

struct s_node_data{
    node_data prev;
    node_data next;
    uint key;
    float value;
};

node_data data_prev(const node_data data){
    return data->prev;
}
node_data data_next(const node_data data){
    return data->prev;
}
uint data_key(const node_data data){
    return data->key;
}
float data_value(const node_data data){
    return data->value;
}

node_data data_create(uint key, float value){
    node_data data = malloc(sizeof(node_data));
    data->prev = data;
    data->next = data;
    data->key = key;
    data->value = value;
    return data;
}

void data_add(node_data data, node_data new_data){
    /*node_data new_data = data_create(key, value);
    node_data curr_data = data;
    while( data->key != data->key)
   
    
    if(data->next == data){
        new_data->prev = data->prev;
        new_data->next = data;
        data->prev->next = new_data;
        data->prev = new_data;

    }
    if(curr_data->next != data){
        curr_data
    }
    data_tmp->prev = data->prev;
    data_tmp->next = data;
    data->prev->next = data_tmp;
    data->prev = data_tmp;
    data_tmp->key = key;
    data_tmp->value = value;*/
    return;
}

char *data_print(node_data data){
    return "";
}
void data_clean(node_data data){
    node_data data_tmp=data->prev;
    while(data_tmp!=data){
        data_tmp->prev->next = data_tmp->next;
        data_tmp->next->prev = data_tmp->prev;
        data_tmp->prev = NULL;
        data_tmp->next = NULL;
        free(data_tmp);
        data_tmp=data->prev;
    }
    free(data);
    return;
}

struct s_routing{
    address prev;
    address next;
    uint lower_id;
    uint higher_id;
    node_data values;
    //address finger_table[MAX_NEIGHBOURS]; V3
};
address routing_prev(const routing r){
    return r->prev;
}
address routing_next(const routing r){
    return r->next;
}
uint routing_lid(const routing r){
    return r->lower_id;
}
uint routing_hid(const routing r){
    return r->higher_id;
}
node_data routing_values(const routing r){
    return r->values;
}

routing routing_create(address prev, address next, uint lower_id, uint higher_id, node_data val){
    routing r = malloc(sizeof(struct s_routing));
    r->prev = prev;
    r->next = next;
    r->lower_id = lower_id;
    r->higher_id = higher_id;
    r->values = val;
    return r;
}
void routing_update(routing r, address prev, address next, uint lower_id, uint higher_id, node_data data){
    if(prev != NULL)
        r->prev = prev;
    if(next != NULL)
        r->next = next;
    if(lower_id != -1)
        r->lower_id = lower_id;
    if(higher_id != -1)
        r->higher_id = higher_id;
    if(data != NULL)
        data_add(r->values, data);
}
void routing_clean(routing r){
    if(r->prev != NULL)
        address_clean(r->prev);
    if(r->next != NULL)
        address_clean(r->next);
    if(r->values != NULL)
        data_clean(r->values);
    free(r);
    return;
}

struct s_statistics{
    int msg_get;
    int msg_put;
    int msg_gst;
};

struct s_list{
    list prev;
    list next;
    char *value;
    int size;
};

list list_prev(list l){
    return l->prev;
}
list list_next(list l){
    return l->next;
}
char* list_value(list l){
    return l->value;
}
int list_size(list l){
    return l->size;
}

list list_create(char *str, int size){
    list l = malloc(sizeof(struct s_list));
    l->size = size;
    l->value = malloc(sizeof(char)*size);
    memcpy(l->value,str,l->size);
    l->prev = l;
    l->next = l;
    return l;
}

list list_generate(char *chain){
    char c[strlen(chain)];
    memcpy (c,chain,strlen(chain));
    char *token;
    int size;
    const char delim[4] = ":, ";
    printf(" input : %s\n",chain);
    token = strtok(c, delim);
    token = strtok(NULL, delim);
    list l = list_create(token, strlen(token));
    token=strtok(NULL,delim);
    while (token != NULL) {
        token = strtok(NULL,delim);
        if(token !=NULL){
            size = strlen(token);
            if(token[0]=='{' || token[0]=='"'){
                while(token[0]=='{' || token[0]=='"'){
                    token++;
                    size--;
                }
            }
            if(token[size-1]=='}' || token[size-1]=='"'){
                while(token[size-1]=='}' || token[size-1]=='"'){ 
                    token[--size]= 0;
                }
            }
            printf("adding %s to %p\n",token, l);
            list_add(l,token,size);
        }
    }
    return l;
}

void list_clean(list l){
    list l_tmp=l->prev;
    while(l_tmp!=l){
        l_tmp->prev->next = l_tmp->next;
        l_tmp->next->prev = l_tmp->prev;
        free(l_tmp->value);
        l_tmp->prev = NULL;
        l_tmp->next = NULL;
        free(l_tmp);
        l_tmp=l->prev;
    }
    free(l->value);
    free(l);
    return;
}

void list_add(list l, char *str, int size){
    list l_tmp = malloc(sizeof(struct s_list));
    l_tmp->prev = l->prev;
    l_tmp->next = l;
    l->prev->next = l_tmp;
    l->prev = l_tmp;
    l_tmp->size = size;
    l_tmp->value = malloc(sizeof(char)*size);
    memcpy(l_tmp->value,str,size);  
    return;
}

void list_print(list l){
    list l_tmp;
    l_tmp = l;
    do{
        printf("<%s> ",l_tmp->value);
        l_tmp=l_tmp->next;
    }while(l_tmp!=l);
    printf("\n");
    return;
}