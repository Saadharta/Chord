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

struct s_node{
    node prev;
    node next;
    uint k;
    float v;
};

node node_prev(const node n){
    return n->prev;
}
node node_next(const node n){
    return n->prev;
}
uint node_key(const node n){
    return n->k;
}
float node_value(const node n){
    return n->v;
}

node node_create(uint k, float v){
    node n = malloc(sizeof(node));
    n->prev = n;
    n->next = n;
    n->k = k;
    n->v = v;
    return n;
}

void node_add_node(node e, node n){
    if(e->k == n->k){
        e->v = n->v;
        node_clean(n);
    }else{
        node n_tmp=e;
        if(e->k < n->k){
            //printf("%d < %d\n", e->k, n->k);
            while(n_tmp->next->k < n->k && n_tmp->k < n_tmp->next->k){
                //printf("looping, %d < %d && %d > %d\n",n_tmp->next->k, n->k, n_tmp->k, n_tmp->next->k);
                n_tmp = n_tmp->next;
            }
            if(n_tmp->next->k == n->k){
                n_tmp->next->v = n->v;
                node_clean(n);
            }else{
                n_tmp->next->prev = n;
                n->prev  = n_tmp;
                n->next= n_tmp->next;
                n_tmp->next = n;
            }
        }else{
            //printf("%d > %d\n", e->k, n->k);
            while(n_tmp->prev->k > n->k && n_tmp->k > n_tmp->prev->k){
                //printf("looping, %d > %d && %d < %d\n",n_tmp->prev->k, n->k, n_tmp->k, n_tmp->prev->k);
                n_tmp=n_tmp->prev;
            }
            if(n_tmp->prev->k == n->k){
                n_tmp->prev->v = n->v;
                node_clean(n);
            }else{
                n_tmp->prev->next = n;
                n->next = n_tmp;
                n->prev = n_tmp->prev;
                n_tmp->prev = n;
            }
        }
    }
    return;
}

void node_add(node e, node n){
    node n_tmp=n;
    do{
        //printf("moving node %d:%f\n",n_tmp->k,n_tmp->value );
        node_add_node(e,node_create(n_tmp->k, n_tmp->v));
        n_tmp = n_tmp->next;
    }while (n_tmp!=n);
    node_clean(n);
    return;
}

void node_split(node e, node n){
    if(e->prev == e){
        goto out;
    }
    node n_tmp = e;
    node n_end;
    if(n->k > e->k){
        while(n_tmp->next->k < n->k && n_tmp->k < n_tmp->next->k){
            //printf("looping, %d < %d && %d > %d\n",n_tmp->next->k, n->k, n_tmp->k, n_tmp->next->k);
            n_tmp = n_tmp->next;
        }
        n_end = e->next;
        n_tmp->next->prev = e;
        n_end->prev = n_tmp;
        e->next = n_tmp->next;
        n_tmp->next = n_end; 
        node_add(n, n_tmp);
    }else{
        while(n_tmp->prev->k > n->k && n_tmp->k > n_tmp->prev->k){
            //printf("looping, %d < %d && %d > %d\n",n_tmp->next->k, n->k, n_tmp->k, n_tmp->next->k);
            n_tmp = n_tmp->prev;
        }
        n_tmp = n_tmp->prev;
        n_end = e->next;
        n_tmp->next->prev = e;
        n_end->prev = n_tmp;
        e->next = n_tmp->next;
        n_tmp->next = n_end; 
        node_add(n, n_tmp);
    }
    out:
        return;
}

float node_find(node e, uint k){
    node n_tmp = e;
    int key;
    do{
        n_tmp=n_tmp->next;
    }while(n_tmp!=e && n_tmp->k >= k);
    if(n_tmp == e){
        key = -1;
    }else{
        key = ((n_tmp->k == k) ? n_tmp->v : -1);
    }
    return key;
}

char *node_print(node e){
    node n_tmp = e->next;
    char *ret=malloc(MAX_NODES*(sizeof(uint)+sizeof(float)));
    char *msg;
    while(n_tmp!= e){
        msg=malloc(32*sizeof(char));
        snprintf(msg,32, "%d:%f,",n_tmp->k, n_tmp->v);
        strcat(ret, msg);
        n_tmp = n_tmp->next;
        free(msg);
    }
    msg=malloc(32*sizeof(char));
    snprintf(msg,32, "%d:%f",e->k, e->v);
    strcat(ret, msg);
    free(msg);
    return ret;
}

void node_clean(node e){
    node n_tmp=e->prev;
    while(n_tmp!=e){
        n_tmp->prev->next = n_tmp->next;
        n_tmp->next->prev = n_tmp->prev;
        n_tmp->prev = NULL;
        n_tmp->next = NULL;
        free(n_tmp);
        n_tmp=e->prev;
    }
    free(e);
    return;
}

struct s_routing{
    address prev;
    address next;
    uint lower_id;
    uint higher_id;
    uint amount;
    node n;
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
uint routing_amount(const routing r){
    return r->amount;
}
node routing_values(const routing r){
    return r->n;
}

routing routing_create(address prev, address next, uint lower_id, uint higher_id, node n){
    routing r = malloc(sizeof(struct s_routing));
    r->prev = prev;
    r->next = next;
    r->lower_id = lower_id;
    r->higher_id = higher_id;
    if(r->lower_id < r->higher_id){
        r->amount = r->higher_id - r-> lower_id;
    }else{
        r->amount = (MAX_NODES - r->lower_id)+r->higher_id;
    }
    r->n = n;
    return r;
}

void routing_update(routing r, address prev, address next, int lower_id, int higher_id, node n){
    if(prev != NULL)
        r->prev = prev;
    if(next != NULL)
        r->next = next;
    if(lower_id != -1)
        r->lower_id = (uint)lower_id;
    if(higher_id != -1)
        r->higher_id = (uint)higher_id;
    if(r->lower_id < r->higher_id){
        //printf("update lid %d, hid %d, amt %d\n", r->lower_id, r->higher_id, r->amount);
        r->amount = r->higher_id - r-> lower_id;
        //printf("update new amt %d\n",r->amount);
    }else{
        //printf("update lid %d, hid %d, amt %d\n", r->lower_id, r->higher_id, r->amount);
        r->amount = (MAX_NODES - r->lower_id)+r->higher_id;
        //printf("update new amt %d\n",r->amount);
    }
    if(n != NULL)
        node_add(r->n, n);
}

void routing_clean(routing r){
    if(r->prev != NULL)
        address_clean(r->prev);
    if(r->next != NULL)
        address_clean(r->next);
    if(r->n != NULL)
        node_clean(r->n);
    free(r);
    return;
}

struct s_statistics{
    int msg_get;
    int msg_put;
    int msg_gst;
};

struct s_token{
    token prev;
    token next;
    char *value;
    int size;
};

token token_prev(token t){
    return t->prev;
}
token token_next(token t){
    return t->next;
}
char* token_value(token t){
    return t->value;
}
int token_size(token t){
    return t->size;
}

token token_create(char *str, int size){
    token t = malloc(sizeof(struct s_token));
    t->size = size;
    t->value = malloc(sizeof(char)*size);
    memcpy(t->value,str,t->size);
    t->prev = t;
    t->next = t;
    return t;
}

token token_generate(char *chain){
    char c[strlen(chain)];
    memcpy (c,chain,strlen(chain));
    char *str;
    int size;
    const char delim[4] = ":, ";
    //printf(" input : %s\n",chain);
    /* avoid the "cmd" present everytime */
    str = strtok(c, delim);
    str = strtok(NULL, delim);
    token t = token_create(str, strlen(str));
    /* avoid the "args" present everytime */
    str=strtok(NULL,delim);
    while (str != NULL) {
        str = strtok(NULL,delim);
        if(str !=NULL){
            size = strlen(str);
            if(str[0]=='{' || str[0]=='"'){
                while(str[0]=='{' || str[0]=='"'){
                    str++;
                    size--;
                }
            }
            if(str[size-1]=='}' || str[size-1]=='"'){
                while(str[size-1]=='}' || str[size-1]=='"'){ 
                    str[--size]= 0;
                }
            }
            //printf("adding %s to %s\n",str, token_value(t));
            token_add(t,str,size);
        }
    }
    return t;
}

void token_clean(token t){
    token t_tmp=t->prev;
    while(t_tmp!=t){
        t_tmp->prev->next = t_tmp->next;
        t_tmp->next->prev = t_tmp->prev;
        free(t_tmp->value);
        t_tmp->prev = NULL;
        t_tmp->next = NULL;
        free(t_tmp);
        t_tmp=t->prev;
    }
    free(t->value);
    free(t);
    return;
}

void token_add(token t, char *str, int size){
    token t_tmp = malloc(sizeof(struct s_token));
    t_tmp->prev = t->prev;
    t_tmp->next = t;
    t->prev->next = t_tmp;
    t->prev = t_tmp;
    t_tmp->size = size;
    t_tmp->value = malloc(sizeof(char)*size);
    memcpy(t_tmp->value,str,size);  
    return;
}

void token_print(token t){
    token t_tmp;
    t_tmp = t;
    do{
        printf("<%s> ",t_tmp->value);
        t_tmp=t_tmp->next;
    }while(t_tmp!=t);
    printf("\n");
    return;
}