#include "chord_cmd_core.h"

int cmd_sort(token t){
    char *str = token_value(t);
    int ret;
    if (!strcmp(str,"hello")) {
        ret = HELLO;
    }else if(!strcmp(str,"hello_ok")){
        ret = HELLO_OK;
    }else if(!strcmp(str,"hello_ko")){
        ret = HELLO_KO;
    }else if(!strcmp(str,"get")){
        ret = GET;
    }else if(!strcmp(str,"answer")){
        ret = ANSWER;
    }else if(!strcmp(str,"put")){
        ret = PUT;
    }else if(!strcmp(str,"ack")){
        ret = ACK;
    }else if(!strcmp(str,"get_resp")){
        ret = GET_RESP;
    }else if(!strcmp(str,"answer_resp")){
        ret = ANSWER_RESP;
    }else if(!strcmp(str,"update_table")){
        ret = UPDATE_TABLE;
    }else if(!strcmp(str,"print")){
        ret = PRINT;
    }else if(!strcmp(str,"stats")){
        ret = GET_STAT;
    }else if(!strcmp(str,"terminate")){
        ret = TERMINATE;
    }else{
        ret =  -1;
    }
    return ret;
}

/* General purpose */
int is_between(uint a, uint b, uint t){
    if(a < b) { 
        if (a <= t && t <= b)
            return 1;
        else
            return 0;
    }else{ 
        if((0 <= t && t <= b)||(MAX_NODES > t && t >= a))
            return 1;
        else
            return 0;
    }        
}

/* Insert */
uint hello(token t, routing r, uint req_id){
    //hello idNode %d ip %s port %d
    token t_curr = token_next(token_next(t));
    uint id, port;
    char ip[16];
    address src;
    address self = routing_self(r);

    id = (uint)atoi(token_value(t_curr));
    t_curr=(token_next(token_next(t_curr)));
    strcpy(ip,token_value(t_curr));
    t_curr=(token_next(token_next(t_curr)));
    port = (uint)atoi(token_value(t_curr));
    src = address_create(id,ip,port);
    printf("%d@%s:%d : Receiving [HELLO] from %d@%s:%d\n",address_id(self), address_ip(self), address_port(self), address_id(src), address_ip(src), address_port(src));
    if(id == address_id(self)){
        send_hello_ko(address_id(src), src, self);
    }else{
        if(is_between(routing_lid(r), routing_hid(r), id)){
            /* splitting the owner's nodes accordingly to the requested key */
            node n_tmp = node_create(address_id(src),0);
            node_split(routing_values(r), n_tmp);
            address prev = routing_prev(r);
            routing_update(r, src,NULL,(address_id(src)+1)%(MAX_NODES+1),-1,NULL);
            routing_print(r);
            send_hello_ok(address_id(src),self,n_tmp,prev, ++req_id, src);
            node_clean(n_tmp);
        }else{
            send_hello(src, routing_prev(r), self);
        }
    }
    //address_clean(src);
    return req_id;
}


void hello_ok(token t, routing r){
    //hello_ok id_requested %d ip_port_resp idNode %d ip %s port %d data %s prec idNode %d ip %s port %d id_request %d}
    token t_curr = token_next(token_next(t));
    uint id, port, k;
    float v;
    char ip[16];
    char *node;
    address prev,next;
    address self = routing_self(r);

    if((uint)atoi(token_value(t_curr))!= address_id(self)){
        perror("hello_ok didn't acknowledged the right id");
        exit(EXIT_FAILURE);
    }
    t_curr = token_next(t_curr);
    /* fetching our next node address*/
    t_curr = token_next(token_next(t_curr));
    id = (uint)atoi(token_value(t_curr));
    t_curr = token_next(token_next(t_curr));
    strcpy(ip,token_value(t_curr));
    t_curr = token_next(token_next(t_curr));
    port = (uint)atoi(token_value(t_curr));
    next=address_create(id,ip,port);

    /* fetching the now owned nodes */
    t_curr = token_next(token_next(t_curr));
    if(strcmp(token_value(token_next(t_curr)), "prec")){ 
        while(strcmp(token_value(t_curr), "prec")){
            k = (uint)atoi(token_value(t_curr));
            t_curr = token_next(t_curr);
            v = (float)atof(token_value(t_curr));
            t_curr = token_next(t_curr);
            node_add_node(routing_values(r),node_create(k,v));
        }
    }
    /* fetching our previous neighbour */
    t_curr = token_next(token_next(t_curr));
    id = (uint)atoi(token_value(t_curr));
    t_curr = token_next(token_next(t_curr));
    strcpy(ip,token_value(t_curr));
    t_curr = token_next(token_next(t_curr));
    port = (uint)atoi(token_value(t_curr));
    prev=address_create(id,ip,port);

    t_curr = token_next(token_next(t_curr));
    node = node_print(routing_values(r));
    printf("%d@%s:%d : Receiving [HELLO_OK] from %d@%s:%d with previous %d@%s:%d and node %s \n",address_id(self), address_ip(self), address_port(self), address_id(next), address_ip(next), address_port(next), address_id(prev), address_ip(prev), address_port(prev), node);
    /* updating our routing data */
    routing_update(r,prev,next,(address_id(prev)+1)% (MAX_NODES+1), -1, NULL);
    routing_print(r);
    /* acknowledging our next node of the reception */
    send_ack((uint)atoi(token_value(t_curr)),next, self);

    /* notifying our previous to update his routing info */
    //send_update_table(self, routing_lid(r),routing_amount(r),routing_prev(r));
    send_update_table(self, -1,-1,routing_prev(r),self);

    free(node);
    return;
}


void get(token t, routing r){
    // get host idNode %d ip %s port %d key %d
    token t_curr = token_next(t);
    uint id, port, k;
    float v;
    char ip[16];
    address src;
    address self = routing_self(r);

    /* fetching the source address */
    t_curr = token_next(token_next(t_curr));
    id = (uint)atoi(token_value(t_curr));
    t_curr = token_next(token_next(t_curr));
    strcpy(ip,token_value(t_curr));
    t_curr = token_next(token_next(t_curr));
    port = (uint)atoi(token_value(t_curr));
    src=address_create(id,ip,port);
    
    t_curr = token_next(token_next(t_curr));
    k = (uint)atoi(token_value(t_curr));

    printf("%d@%s:%d : Receiving [GET] <%d> from %d@%s:%d \n",address_id(self), address_ip(self), address_port(self),k, address_id(src), address_ip(src), address_port(src));

    if(is_between(routing_lid(r), routing_hid(r), k)){
        if((v = node_find(routing_values(r),k) )!=-1){
            send_answer(k,v,"True", src, self);
        }else{
            send_answer(k,v,"False", src, self);
        }
    }else{
        if(address_id(routing_prev(r))!=address_id(src))
            send_get(src, k, routing_prev(r), self);
        else
            send_answer(k,-1,"False", src, self);

    }
    return;
}

void answer(token t, routing r){
    //answer key %d value %f val_exists %s
    token t_curr = token_next(t);
    uint k;
    float v;
    address self = routing_self(r);

    /* fetching the source address */
    t_curr = token_next(t_curr);
    k = (uint)atoi(token_value(t_curr));
    t_curr = token_next(token_next(t_curr));
    v = atof(token_value(t_curr));
    t_curr = token_next(token_next(t_curr));
    printf("%d@%s:%d : Receiving [ANSWER] <%d:%f> = %s\n",address_id(self), address_ip(self), address_port(self),k, v, token_value(t_curr));
    return;
}

void put(token t, routing r){
    //put host idNode %d ip %s port %d key %d value %f id %d
    token t_curr = token_next(t);
    uint id, port, k, req_id;
    float v;
    char ip[16];
    address src;
    address self = routing_self(r);
    
    /* fetching our next node address*/
    t_curr = token_next(token_next(t_curr));
    id = (uint)atoi(token_value(t_curr));
    t_curr = token_next(token_next(t_curr));
    strcpy(ip,token_value(t_curr));
    t_curr = token_next(token_next(t_curr));
    port = (uint)atoi(token_value(t_curr));
    src=address_create(id,ip,port);

    t_curr = token_next(token_next(t_curr));
    k = (uint)atoi(token_value(t_curr));
    t_curr = token_next(token_next(t_curr));
    v = (uint)atof(token_value(t_curr));
    t_curr = token_next(token_next(t_curr));
    req_id = (uint)atoi(token_value(t_curr));

    printf("%d@%s:%d : Receiving [PUT] <%d,%f> from %d@%s:%d \n",address_id(self), address_ip(self), address_port(self),k,v, address_id(src), address_ip(src), address_port(src));

    if(is_between(routing_lid(r), routing_hid(r), k)){
        node_add(routing_values(r),node_create(k, v));
        //routing_print(r);
        send_ack(req_id, src, self);
    }else{
        send_put(src, k, v, req_id, routing_prev(r), self);
    }
    address_clean(src);
    return;
}

uint ack(token t, routing r){
    //ack id req_id
    token t_curr = token_next(token_next(t));
    address self = routing_self(r);

    uint req_id = (uint)atoi(token_value(t_curr));
    printf("%d@%s:%d : Receiving [ACK] %d\n",address_id(self), address_ip(self), address_port(self),req_id);
    return req_id;
}

void get_resp(token t, routing r){
    //get_resp src idNode %d ip %s port %d key %d
    token t_curr = token_next(t);
    uint id, port, k;
    char ip[16];
    address src;
    address self = routing_self(r);

    /* fetching source address*/
    t_curr = token_next(token_next(t_curr));
    id = (uint)atoi(token_value(t_curr));
    t_curr = token_next(token_next(t_curr));
    strcpy(ip,token_value(t_curr));
    t_curr = token_next(token_next(t_curr));
    port = (uint)atoi(token_value(t_curr));
    src=address_create(id,ip,port);

    /* fetching the key */
    t_curr = token_next(token_next(t_curr));
    k = (uint)atoi(token_value(t_curr));
    printf("%d@%s:%d : Receiving [GET_RESP] from %d@%s:%d with key %d\n",address_id(self), address_ip(self), address_port(self), address_id(src), address_ip(src), address_port(src),k);
    /* sending current node  answer */
    if(is_between(routing_lid(r), routing_hid(r), k)){
        send_ans_resp(k, self, src);
    }else{
        send_get_resp(src, k, routing_prev(r), self);
    }
    address_clean(src);
    return;
}

void ans_resp(token t, routing r){
    //answer_resp key %d src\ idNode %d ip %s port %d
    token t_curr = token_next(t);
    uint id, port, k;
    char ip[16];
    address src;
    address self = routing_self(r);

    /* fetching key value */
    t_curr = token_next(t_curr);
    k = (uint)atoi(token_value(t_curr));

    /* fetching source address*/
    t_curr = token_next(token_next(t_curr));
    id = (uint)atoi(token_value(t_curr));
    t_curr = token_next(token_next(t_curr));
    strcpy(ip,token_value(t_curr));
    t_curr = token_next(token_next(t_curr));
    port = (uint)atoi(token_value(t_curr));
    src=address_create(id,ip,port);
    
    printf("%d@%s:%d : Receiving [ANSWER_RESP] from %d@%s:%d for key %d\n",address_id(self), address_ip(self), address_port(self), address_id(src), address_ip(src), address_port(src),k);
    /* TODO? */

}

void update_table(token t, routing r){
    //update_table src idNode %d ip %s port %d id_lower %d amount %d
    token t_curr = token_next(t);
    uint id, port;
    int lower_key, amount;
    char ip[16];
    address src;
    address self = routing_self(r);

    t_curr=(token_next(token_next(t_curr)));
    id = (uint)atoi(token_value(t_curr));
    t_curr=(token_next(token_next(t_curr)));
    strcpy(ip,token_value(t_curr));
    t_curr=(token_next(token_next(t_curr)));
    port = (uint)atoi(token_value(t_curr));
    src = address_create(id,ip,port);

    t_curr=(token_next(token_next(t_curr)));
    lower_key = (int)atoi(token_value(t_curr));

    t_curr=(token_next(token_next(t_curr)));
    amount = (int)atoi(token_value(t_curr));

    printf("%d@%s:%d : Receiving [UPTADE_TABLE] from %d@%s:%d with values %d, %d\n",address_id(self), address_ip(self), address_port(self), address_id(src), address_ip(src), address_port(src),lower_key, amount);
    if(lower_key ==-1 && amount == -1){
        routing_update(r,NULL,src, -1, -1, NULL);
    }else if(lower_key ==-1 && amount == 0){
        routing_update(r,NULL,src, -1, -1, NULL);
    }else if(lower_key != -1 && amount == 0){
        routing_update(r,src,NULL, lower_key, -1, NULL);
    }
    routing_print(r);
    return;
}
void print(token t, routing r, statistics s){
    //print src idNode %d ip %s port %d
    token t_curr = token_next(t);
    uint id, port;
    char ip[16];
    address src;
    address self = routing_self(r);

    t_curr=(token_next(token_next(t_curr)));
    id = (uint)atoi(token_value(t_curr));
    t_curr=(token_next(token_next(t_curr)));
    strcpy(ip,token_value(t_curr));
    t_curr=(token_next(token_next(t_curr)));
    port = (uint)atoi(token_value(t_curr));
    src = address_create(id,ip,port);
    
    printf("%d@%s:%d : Receiving [PRINT] from %d@%s:%d\n",address_id(self), address_ip(self), address_port(self), address_id(src), address_ip(src), address_port(src));
    stat_print(s);
    if(address_port(self)!=address_port(src)){
        send_print(src, routing_prev(r), self);
    }
    return;
}

void get_stat(token t, routing r, statistics s){
    //stats src idNode %d ip %s port %d nb_get %d nb_put %d nb_other %d
    token t_curr = token_next(t);
    uint id, port;
    int msg_get, msg_put, msg_gst;
    char ip[16];
    address src;
    address self = routing_self(r);

    t_curr=(token_next(token_next(t_curr)));
    id = (uint)atoi(token_value(t_curr));
    t_curr=(token_next(token_next(t_curr)));
    strcpy(ip,token_value(t_curr));
    t_curr=(token_next(token_next(t_curr)));
    port = (uint)atoi(token_value(t_curr));
    src = address_create(id,ip,port);
    
    t_curr=(token_next(token_next(t_curr)));
    msg_get = (uint)atoi(token_value(t_curr));
    t_curr=(token_next(token_next(t_curr)));
    msg_put = (uint)atoi(token_value(t_curr));
    t_curr=(token_next(token_next(t_curr)));
    msg_gst = (uint)atoi(token_value(t_curr));

    printf("%d@%s:%d : Receiving [GET_STATS] from %d@%s:%d\n",address_id(self), address_ip(self), address_port(self), address_id(src), address_ip(src), address_port(src));
    if(address_id(routing_prev(r))!=address_id(src)){
        send_get_stat(src, msg_get + stat_get(s), msg_put + stat_put(s), msg_gst + stat_gst(s),routing_prev(r), self);
    }else{
        printf("-----global Statistics-----\n");
        printf("msg_get: %d\nmsg_put: %d\nmsg_gst: %d\n", msg_get, msg_put, msg_gst);
        printf("---------------------------\n");
    }
    return;
}

void terminate(routing r){
    node n_tmp = routing_values(r);
    address self = routing_self(r);
    int req_id = 0;
    do{
        send_put(self,node_key(n_tmp),node_value(n_tmp),req_id++, routing_next(r), self);
        n_tmp= node_next(n_tmp);
    }while(n_tmp != routing_values(r) );

    /* removing self from previous node routing struct */
    send_update_table(routing_next(r), -1, 0, routing_prev(r),self);
    /* removing self from next node routing struct and updating it's lower_id */
    send_update_table(routing_prev(r), routing_lid(r), 0, routing_next(r),self);

    return;
}
