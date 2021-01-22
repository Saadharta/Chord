#include "chord_struct.h"

struct s_address{
    uint id;
    char *ip;
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
    int len = 16;
    address a= malloc(sizeof(struct s_address)+16*sizeof(char));
    a->ip = malloc(sizeof(char)*15+1);
    a->id = id;
    memcpy(a->ip, ip, len);
    a->port = port;
    return a;
}

char *address_print(address a){
    char *msg = malloc(sizeof(char)*28+1);
    snprintf(msg,29,"%5d@%s:%-5d", a->id, a->ip, a->port);
    return msg;
}

void address_clean(address a){
    /*memset(a->ip, 0, 16); //len 15 + '\0'*/
    free(a->ip);
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
    node n = malloc(sizeof(struct s_node));
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
    float v;
    if(e->k == k){
        v = e->v;
        goto entry_val;
    }
    do{
        n_tmp=n_tmp->next;
    }while(n_tmp!=e && n_tmp->k != k);
    if(n_tmp == e){
        v = -1;
    }else{
        v = ((n_tmp->k == k) ? n_tmp->v : -1);
    }
entry_val:
    return v;
}

char *node_print(node e){
    node n_tmp = e->next;
    char *ret = malloc(100*(sizeof(uint)+sizeof(float)+3));
    char *msg = malloc(32*sizeof(char)+3);
    strcpy(ret,"");
    while(n_tmp!= e){
        snprintf(msg,32, "%d:%f,",n_tmp->k, n_tmp->v);
        strcat(ret, msg);
        n_tmp = n_tmp->next;
    }
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
    address self;
    uint lower_id;
    uint higher_id;
    uint amount;
    node n;
    address finger_table[MAX_NEIGHBOURS]; 
};

address routing_prev(const routing r){
    return r->prev;
}

address routing_next(const routing r){
    return r->next;
}

const address routing_self(const routing r){
    return r->self;
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

address routing_get_fg(routing r, int i){
    if(i<MAX_NEIGHBOURS)
        return r->finger_table[i];
    else
        return r->finger_table[MAX_NEIGHBOURS-1];
}

address routing_cls_fg(routing r, uint i){
    int idx = 0;
    if(r->finger_table[0]==NULL){
        printf("returning next");
        return routing_next(r);
    }
    while((idx<MAX_NEIGHBOURS-1) && (r->finger_table[idx]!=NULL) && (address_id(r->finger_table[idx])<i)){   
        printf("%d < %d with %d\n",address_id(r->finger_table[idx]), i, idx);
        ++idx;
    }
    return r->finger_table[idx];
}

void routing_set_fg(routing r, int k, address a){
    int i = 0;
    while( ((routing_hid(r)+(1<<i))%MAX_NODES+1) <k){
        ++i;
    }
    r->finger_table[i] = address_create(address_id(a), address_ip(a),address_port(a));
}

routing routing_create(address prev, address next, address self, uint lower_id, uint higher_id, node n){
    routing r = malloc(sizeof(struct s_routing));
    int i = 0;
    r->prev = prev;
    r->next = next;
    r->self = self;
    r->lower_id = lower_id;
    r->higher_id = higher_id;
    if(r->lower_id < r->higher_id){
        r->amount = r->higher_id - r-> lower_id;
    }else{
        r->amount = (MAX_NODES - r->lower_id)+r->higher_id;
    }
    r->n = n;
    for (i = 0; i< MAX_NEIGHBOURS; ++i){
        r->finger_table[i] = address_create(address_id(self), address_ip(self), address_port(self));
    }
    return r;
}

void routing_update(routing r, address prev, address next, int lower_id, int higher_id, node n){
    if(prev != NULL){
        address_clean(r->prev);
        r->prev = prev;
        //r->prev = address_create(address_id(prev),address_ip(prev), address_port(next));
    }
    if(next != NULL){
        address_clean(r->next);
        //r->next = address_create(address_id(next),address_ip(next), address_port(next));
        r->next = next;
    }
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

void routing_print(routing r){
    char *msg_prev = address_print(r->prev);
    char *msg_next = address_print(r->next);
    char *msg_own;
    char *msg_node= node_print(r->n);
    //int s;
    printf("-------> content of my routing table <-------\n");
    printf("next node : %s\n",msg_next);
    printf("lower_id : %d\n", r->lower_id);
    printf("higher_id : %d\n", r->higher_id);
    printf("nodes : %s\n", msg_node);
    printf("finger table:\n");
    printf("+--------+------------------------+\n");
    printf("| idNode |      owner of node     |\n");
    printf("+--------+------------------------+\n");
    printf("|  prec  | %s  |\n",msg_prev);
    for(int i =0; i <MAX_NEIGHBOURS; ++i){
        msg_own = address_print(r->finger_table[i]);
        printf("| %6d | %s  |\n",(routing_hid(r)+(1<<i))%(MAX_NODES+1),msg_own);
        free(msg_own);
    }
    printf("+--------+------------------------+\n");
    printf("<------- end of my routing table ----------->\n");
    //s = strlen(msg_prev);
    //memset(&msg_prev, 0, sizeof(msg_prev));
    free(msg_prev);
    //memset(&msg_next, 0, sizeof(msg_next));
    free(msg_node);
    //memset(&msg_node, 0, sizeof(msg_node));
    free(msg_next);
    return;
}

void routing_clean(routing r){
    int i = MAX_NEIGHBOURS;
    while(i != 0){
       address_clean(r->finger_table[--i]);
    }
    if(r->prev != NULL) 
        address_clean(r->prev);
    if(r->next != NULL) 
        address_clean(r->next);
    if(r->self != NULL)
        address_clean(r->self);
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

statistics stat_create(void){
    statistics s = malloc(sizeof(struct s_statistics));
    s->msg_get = 0;
    s->msg_put = 0;
    s->msg_gst = 0;
    return s;
}

void stat_print(statistics s){
    printf("-----Statistics-----\n");
    printf("msg_get: %d\nmsg_put: %d\nmsg_gst: %d\n", s->msg_get, s->msg_put, s->msg_gst);
    printf("--------------------\n");
}

int stat_get(statistics s){
    return s->msg_get;
}

int stat_put(statistics s){
    return s->msg_put;
}

int stat_gst(statistics s){
    return s->msg_gst;
}

void stat_iget(statistics s){
    ++s->msg_get;
}

void stat_iput(statistics s){
    ++s->msg_put;
}

void stat_igst(statistics s){
    ++s->msg_gst;
}

void stat_clean(statistics s){
    free(s);
}
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
    t->value = malloc(sizeof(char)*size+1);
    memset(t->value, 0, t->size);
    memcpy(t->value,str,t->size);
    t->prev = t;
    t->next = t;
    return t;
}

token token_generate(char *chain){
    int i, j, len;
	len = strlen(chain);
	for(i = 0; i < len; i++){
		if(chain[i] == '\"' || chain[i]=='}' || chain[i]=='{'){
			for(j = i; j < len; j++){
				chain[j] = chain[j + 1];
	        }
			len--;
			i--;	
		} 
	}	
    int size;
    const char delim[4] = ":, ";
    /* avoid the "cmd" present everytime */
    char *str = strtok(chain, delim);
    str = strtok(NULL, delim);
    token t = token_create(str, strlen(str)+1);
    /* avoid the "args" present everytime */
    str=strtok(NULL,delim);
    while (str != NULL) {
        str = strtok(NULL,delim);
        if(str !=NULL){
            size = strlen(str)+1;
            token_add(t,str,size);
        }
    }
    free(chain);
    return t;
}

void token_clean(token t){
    token t_tmp=t->prev;
    while(t_tmp!=t){
        t_tmp->prev->next = t_tmp->next;
        t_tmp->next->prev = t_tmp->prev;
        memset(t->value, 0, t->size);
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