#include "chord.h"

char *comm_in(const address self, int sock_id, struct sockaddr_in server_adr){

    struct sockaddr_in client_adr;
    socklen_t client_adr_len = sizeof(client_adr);
    char *buffer;
    buffer = malloc(sizeof(char)*BUF_SIZE);

    /*-----------------------------------------*/
    /* Réception des messages (attente active) */
    /*-----------------------------------------*/

    client_adr_len = sizeof(client_adr);
    if (recvfrom(sock_id, buffer, BUF_SIZE, SOCK_NONBLOCK, (struct sockaddr *)&client_adr, &client_adr_len) < 0) {
            perror("recvfrom error");
            exit(EXIT_FAILURE);
    }
    //printf("Message reçu : %s\n", buffer);
    return buffer;
}

int comm_out(address dest, char* buff){
    int sock_id, server_port;
    char server_ip[16]; /* address as a string */
    struct sockaddr_in server_adr;
    server_port = address_port(dest);
    strcpy(server_ip, address_ip(dest));

    /* Datagram socket creation */
    /*-------------------------------------------*/
    sock_id = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sock_id < 0 ) {
            perror("socket error");
            exit(EXIT_FAILURE);
    }

    //printf("sendto socket : %d\n", sock_id);
    /* destination address initialisation */
    /*-----------------------------------------*/
    memset(&server_adr, 0, sizeof(server_adr));
    server_adr.sin_family = AF_INET;
    server_adr.sin_port = htons(server_port); /* htons: host to net byte order (short int) */
    server_adr.sin_addr.s_addr = inet_addr(server_ip);

    /* Build request message */
    /*-----------------------*/
    int size = strlen(buff);
    char buffer[size];
    memcpy (buffer,buff,size);
    //printf("%s\n", buffer);

    /* send message to destination */
    /*-----------------------------*/
    if ( (sendto(sock_id, buffer, size, 0, (struct sockaddr*) &server_adr, sizeof(server_adr))) < 0) {
            perror("sendto error");
            exit(EXIT_FAILURE);
    }
    //printf("sending done\n");
    close(sock_id);
    return 0;
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
        routing_print(r);
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
    if(address_id(self)!=address_id(src)){
        send_get_stat(src, msg_get + stat_get(s), msg_put + stat_put(s), msg_gst + stat_gst(s),routing_prev(r), self);
    }else{
        printf("-----global Statistics-----\n");
        printf("msg_get: %d\nmsg_put: %d\nmsg_gst: %d\n", msg_get, msg_put, msg_gst);
        printf("---------------------------\n");
    }
    return;
}
void send_hello(address src, address dest, address self){
    //{\"cmd\":\"hello\",\"args\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d}}
    char msg[100];
    char *msg_src = address_print(src);
    char *msg_dest = address_print(dest);
    char *msg_self = address_print(self);
    snprintf(msg,100, "{\"cmd\":\"hello\",\"args\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d}}", address_id(src),address_ip(src),address_port(src));
    printf("%s : Sending [HELLO] from %s to %s\n",msg_self, msg_src, msg_dest);
    comm_out(dest, msg);
    free(msg_src);
    free(msg_dest);
    free(msg_self);
    return;
}

void send_hello_ok(uint id, address resp_addr, node d, address prev, uint req_id, address dest){
    //{\"cmd\":\"hello_ok\",\"args\":{\"id_requested\":%d,\"ip_port_resp\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"data\":{%s},\"prec\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"id_request\":%d}
    char msg[BUF_SIZE];
    char *node_msg = node_print(d);
    char *msg_resp = address_print(resp_addr);
    char *msg_dest = address_print(dest);
    char *msg_prev = address_print(prev);
    snprintf(msg,BUF_SIZE, "{\"cmd\":\"hello_ok\",\"args\":{\"id_requested\":%d,\"ip_port_resp\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"data\":{%s},\"prec\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"id_request\":%d}", id,address_id(resp_addr),address_ip(resp_addr),address_port(resp_addr),node_msg,address_id(prev),address_ip(prev),address_port(prev),req_id);
    printf("%s : Sending [HELLO_OK] to %s with previous %s and node %s \n", msg_resp, msg_dest, msg_prev, node_msg);
    comm_out(dest, msg);
    free(node_msg);
    free(msg_resp);
    free(msg_dest);
    free(msg_prev);
    return;
}

void send_hello_ko(uint id, address dest, address self){
    //{\"cmd\":\"hello_ko\",\"args\":{\"key\":%d}}
    char msg[100];
    char *msg_dest = address_print(dest);
    char *msg_self = address_print(self);
    snprintf(msg,100,"{\"cmd\":\"hello_ko\",\"args\":{\"key\":%d}}", id);
    printf("%s : Sending [HELLO_KO] to %s\n",msg_self, msg_dest);
    comm_out(dest, msg);
    free(msg_dest);
    free(msg_self);
    return;
}
/* Message types */
void send_get(address src, uint k, address dest, address self){
    //{\"cmd\":\"get\",\"args\":{\"host\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"key\":%d}}
    char msg[100];
    char *msg_src = address_print(src);
    char *msg_dest = address_print(dest);
    char *msg_self = address_print(self);
    snprintf(msg,100,"{\"cmd\":\"get\",\"args\":{\"host\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"key\":%d}}", address_id(src),address_ip(src),address_port(src), k);
    printf("%s: Sending [GET] <%d> from %s to %s \n",msg_self,k, msg_src, msg_dest);
    comm_out(dest, msg);
    free(msg_src);
    free(msg_dest);
    free(msg_self);
    return;
}

void send_answer(uint k, float v, char *exists, address dest, address self){
    //{\"cmd\":\"answer\",\"args\":{\"key\":%d,\"value\":%f,\"val_exists\":%s}}
    char msg[150];
    char *msg_dest = address_print(dest);
    char *msg_self = address_print(self);
    snprintf(msg,150,"{\"cmd\":\"answer\",\"args\":{\"key\":%d,\"value\":%f,\"val_exists\":%s}}", k, v, exists);
    printf("%s : Sending [ANSWER] <%d,%f> = %s to %s\n",msg_self,k,v, exists, msg_dest);
    comm_out(dest, msg);
    free(msg_dest);
    free(msg_self);
    return;
}

void send_put(address src, uint k, float v, uint req_id, address dest, address self){
    //{\"cmd\":\"put\",\"args\":{\"host\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"key\":%d,\"value\":%f,\"id\":%d}}
    char msg[150];
    char *msg_src = address_print(src);
    char *msg_dest = address_print(dest);
    char *msg_self = address_print(self);
    snprintf(msg,150,"{\"cmd\":\"put\",\"args\":{\"host\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"key\":%d,\"value\":%f,\"id\":%d}}", address_id(src), address_ip(src), address_port(src), k, v, req_id);
    printf("%s: Sending [PUT] <%d,%f> from %s to %s\n",msg_self,k,v ,msg_src, msg_dest);
    comm_out(dest, msg);
    free(msg_src);
    free(msg_dest);
    free(msg_self);
    return;
}

void send_ack(uint req_id, address dest, address self){
    //{\"cmd\":\"ack\",\"args\":{\"id\":req_id}}
    char msg[100];
    char *msg_dest = address_print(dest);
    char *msg_self = address_print(self);
    snprintf(msg,100,"{\"cmd\":\"ack\",\"args\":{\"id\":%d}}", req_id);
    printf("%s : Sending [ACK] to %s \n",msg_self, msg_dest);
    comm_out(dest, msg);
    free(msg_dest);
    free(msg_self);
    return;
}

/* Update */
void send_get_resp(address src, uint key, address dest, address self){
    //{\"cmd\":\"get_resp\",\"args\":{\"src\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"key\": %d} 
    char msg[150];
    char *msg_src = address_print(src);
    char *msg_dest = address_print(dest);
    char *msg_self = address_print(self);
    snprintf(msg,150,"{\"cmd\":\"get_resp\",\"args\":{\"src\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"key\": %d}", address_id(src), address_ip(src), address_port(src),key );
    printf("%s: Sending [GET_RESP] from %s to %s\n",msg_self, msg_src, msg_dest);
    comm_out(dest, msg);
    free(msg_src);
    free(msg_dest);
    free(msg_self);
    return;
}

void send_ans_resp(uint k, address src, address dest){
    //{\"cmd\":\"answer_resp\",\"args\":{\"key\": %d,\"src\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d}}
    char msg[150];
    char *msg_src = address_print(src);
    char *msg_dest = address_print(dest);
    snprintf(msg,150,"{\"cmd\":\"answer_resp\",\"args\":{\"key\": %d,\"src\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d}}", k, address_id(src), address_ip(src), address_port(src));
    printf("%s : Sending [ANSWER_RESP] of <%d> to %s\n", msg_src, k, msg_dest);
    comm_out(dest, msg);
    free(msg_src);
    free(msg_dest);
    return;
}

void send_update_table(address src, uint lower_key, uint amount, address dest, address self){
    //"{\"cmd\":\"update_table\",\"args\":{\"src\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"id_lower\": %d, \"amount\": %d}"
    char msg[200];
    char *msg_src = address_print(src);
    char *msg_dest = address_print(dest);
    char *msg_self = address_print(self);
    snprintf(msg,200,"{\"cmd\":\"update_table\",\"args\":{\"src\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"id_lower\": %d, \"amount\": %d}", address_id(src), address_ip(src),address_port(src), lower_key, amount);
    printf("%s : Sending [UPDATE_TABLE] <%d,%d> from %s to %s\n",msg_self, lower_key, amount, msg_src, msg_dest);
    comm_out(dest, msg);
    free(msg_src);
    free(msg_dest);
    free(msg_self);
    return;
}
/* Stats */
void send_print(address src, address dest, address self){
    //{\"cmd\":\"print\",\"args\":{\"src\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d}}}
    char msg[150];
    char *msg_src = address_print(src);
    char *msg_dest = address_print(dest);
    char *msg_self = address_print(self);
    snprintf(msg,150, "{\"cmd\":\"print\",\"args\":{\"src\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d}}}", address_id(src),address_ip(src),address_port(src));
    printf("%s : Sending [PRINT] from %s to %s\n", msg_self, msg_src, msg_dest);
    comm_out(dest, msg);
    free(msg_src);
    free(msg_dest);
    free(msg_self);
    return;
}

void send_get_stat(address src, int msg_get, int msg_put, int msg_gst, address dest, address self){
    //{\"cmd\":\"stats\",\"args\":{\"src\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d}},\"nb_get\":%d,\"nb_put\":%d,\"nb_other\":%d}
    char msg[250];
    char *msg_src = address_print(src);
    char *msg_dest = address_print(dest);
    char *msg_self = address_print(self);
    snprintf(msg,150, "{\"cmd\":\"stats\",\"args\":{\"src\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d}},\"nb_get\":%d,\"nb_put\":%d,\"nb_other\":%d}", address_id(src),address_ip(src),address_port(src), msg_get, msg_put, msg_gst);
    printf("%s : Sending [GET_STATS] from %s to %s\n", msg_self, msg_src, msg_dest);
    comm_out(dest, msg);
    free(msg_src);
    free(msg_dest);
    free(msg_self);
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

int main(int argc, char **argv){
    srand (time (NULL));
    int sock_id;
    struct sockaddr_in server_adr;
    int i, cmd;
    uint req_id = 0 ;
    uint self_id, dest_id;
    uint self_port, dest_port;
    char self_ip[16];
    char dest_ip[16];
    char *buffer;
    self_id = (rand()+1) % MAX_NODES;
    address client = NULL;
    address server = NULL;
    routing r;
    statistics s = stat_create();
    token t;
    for (i=0; i < argc; i++) {
        if (!strcmp(argv[i],"-a")) {
            dest_id = atoi(argv[i+1]);
            strcpy(dest_ip, argv[i+2]);
            dest_port= atoi(argv[i+3]);
            server = address_create(dest_id, dest_ip, dest_port);
            printf("known node : id=%d ip=%s port=%d\n",dest_id,dest_ip,dest_port);
            while( self_id == dest_id){
               self_id = (rand()+1) % MAX_NODES;
            }
        }
        if (!strcmp(argv[i],"-p")) {
            strcpy(self_ip ,"127.0.0.1");
            self_port = atoi(argv[i+1]);
        }if (!strcmp(argv[i],"-h")) {
            printf("%s -p port : create the first node of a new network\n", argv[0]);
            printf("%s -a <id_node_known> <ip_known> <port_known> -p <port>: Add a new node into an existing network\n", argv[0]);
            printf("%s -h : displays help\n", argv[0]);
            address_clean(client);
            exit(0);
        }
    }
    client= address_create(self_id, self_ip, self_port);
    printf("self node : id=%d ip=%s port=%d\n",self_id,self_ip,self_port);

    /* Creation d'une socket en mode datagramme  */
    sock_id = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sock_id < 0 ) {
            perror("socket error");
            exit(EXIT_FAILURE);
    }

    /* Initialisation adresse locale du serveur  */
    memset(&server_adr, 0, sizeof(server_adr));
    server_adr.sin_family = AF_INET;
    server_adr.sin_port = htons(address_port(client)); // htons: host to net byte order (short int)
    server_adr.sin_addr.s_addr = htonl(INADDR_ANY); // wildcard address

    if (bind(sock_id, (struct sockaddr *) &server_adr, sizeof(server_adr)) < 0) {
            perror("bind error");
            exit(EXIT_FAILURE);
    }

    if(server!=NULL){
        send_hello(client, server, client);
        stat_igst(s);
        address_clean(server);
    }
    address prev =address_create(address_id(client), address_ip(client),address_port(client));
    address next =address_create(address_id(client), address_ip(client),address_port(client));

    r = routing_create(prev,next, client, address_id(client)+1, address_id(client),node_create(address_id(client),0));
    while(1){
        
        buffer = comm_in(client, sock_id, server_adr);
        t = token_generate(buffer); 
        cmd = cmd_sort(t);
        //token_print(t);
        switch(cmd){
            case(HELLO):
                stat_igst(s);
                req_id = hello(t,r, req_id);
                break;
            case(HELLO_OK):
                stat_igst(s);
                hello_ok(t,r);
                break;
            case(HELLO_KO):
                stat_igst(s);
                srand (time (NULL));
                if( (self_id = rand() % MAX_NODES+1) == dest_id){
                    srand (time (NULL));
                    self_id = rand() % MAX_NODES+1;
                }
                address_id_update(client, self_id);
                send_hello(client, server, client);
                break;
            case(GET):
                stat_iget(s);
                get(t,r);
                break;
            case(ANSWER):
                stat_igst(s);
                answer(t,r);
                break;
            case(PUT):
                stat_iput(s);
                put(t,r);
                break;
            case(ACK):
                stat_igst(s);
                ack(t,r);
                break;
            case(GET_RESP):
                stat_iget(s);
                get_resp(t,r);
                break;
            case(ANSWER_RESP):
                stat_igst(s);
                ans_resp(t,r);
                break;
            case(UPDATE_TABLE):
                stat_igst(s);
                update_table(t, r);
                break;
            case(PRINT):
                stat_igst(s);
                print(t, r, s);
                break;
            case(GET_STAT):
                stat_igst(s);
                get_stat(t, r, s);
                break;
            case(TERMINATE):
                stat_igst(s);
                terminate(r);
                goto terminator;
                break;
            default:
                printf("Unkown code %d, self-termination \n", cmd);
                terminate(r);

                goto terminator;
                break;
        }
        token_clean(t);
    }
terminator :
    close(sock_id);
    token_clean(t);
    routing_clean(r);
    stat_clean(s);
    return 0;
}