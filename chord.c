#include "chord.h"

char *comm_in(const address self){
    int sock_id;
    struct sockaddr_in server_adr;
    struct sockaddr_in client_adr;
    socklen_t client_adr_len = sizeof(client_adr);
    char *buffer;
    buffer = malloc(sizeof(char)*BUF_SIZE);

    /* Creation d'une socket en mode datagramme  */
    /*-------------------------------------------*/
    sock_id = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sock_id < 0 ) {
            perror("socket error");
            exit(EXIT_FAILURE);
    }
    //printf("recvfrom socket : %d\n", sock_id);
    /* Initialisation adresse locale du serveur  */
    memset(&server_adr, 0, sizeof(server_adr));
    server_adr.sin_family = AF_INET;
    server_adr.sin_port = htons(address_port(self)); // htons: host to net byte order (short int)
    server_adr.sin_addr.s_addr = htonl(INADDR_ANY); // wildcard address

    if (bind(sock_id, (struct sockaddr *) &server_adr, sizeof(server_adr)) < 0) {
            perror("bind error");
            exit(EXIT_FAILURE);
    }
    /* Réception des messages (attente active)  */
    /*------------------------------------------*/
    printf("Node awaiting new message...\n");
    client_adr_len = sizeof(client_adr);
    if (recvfrom(sock_id, buffer, BUF_SIZE, SOCK_NONBLOCK, (struct sockaddr *)&client_adr, &client_adr_len) < 0) {
            perror("recvfrom error");
            exit(EXIT_FAILURE);
    }
    //printf("Message reçu : %s\n", buffer);

    close(sock_id);
    return buffer;
}

int comm_out(address dest, char* buff){
    int sock_id, server_port;
    char server_ip[16]; /* address as a string */
    struct sockaddr_in server_adr;
    server_port = address_port(dest);
    strcpy(server_ip, address_ip(dest));

    //printf("server_ip <%s>, server_port <%d>\n", server_ip, server_port);

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
uint hello(token t, routing r, address self, uint req_id){
    //hello idNode %d ip %s port %d
    token t_curr = token_next(token_next(t));
    uint id, port;
    char ip[16];
    address src;

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
        if(is_between(routing_lid(r), routing_hid(r)-1, id)){
            /* splitting the owner's nodes accordingly to the requested key */
            node n_tmp = node_create(address_id(src),0);
            node_split(routing_values(r), n_tmp);
            address prev = routing_prev(r);
            routing_update(r, src,NULL,address_id(src),-1,NULL);
            if(prev == NULL)
                send_hello_ok(address_id(src),self,n_tmp,self, ++req_id, src);
            else
                send_hello_ok(address_id(src),self,n_tmp,prev, ++req_id, src);
            
        }else{
            send_hello(src, routing_prev(r));
        }
    }
    return req_id;
}


void hello_ok(token t, routing r, address self){
    //hello_ok id_requested %d ip_port_resp idNode %d ip %s port %d data %s prec idNode %d ip %s port %d id_request %d}
    token t_curr = token_next(token_next(t));
    uint id, port, k;
    float v;
    char ip[16];
    address prev,next;
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
    
    printf("%d@%s:%d : Receiving [HELLO_OK] from %d@%s:%d with previous %d@%s:%d and node %s \n",address_id(self), address_ip(self), address_port(self), address_id(next), address_ip(next), address_port(next), address_id(prev), address_ip(prev), address_port(prev), node_print(routing_values(r)));
   
    /* updating our routing data */
    routing_update(r,prev,next,-1, -1, NULL);
    routing_update(r,NULL,NULL,address_id(routing_next(r)), -1, NULL);

    /* acknowledging our next node of the reception */
    send_ack((uint)atoi(token_value(t_curr)),next, self);

    /* notifying our previous to update his routing info */
    //send_update_table(self, routing_lid(r),routing_amount(r),routing_prev(r));
    send_update_table(self, -1,-1,routing_prev(r),self);
}


void get(token t, routing r, address self){
    // get host idNode %d ip %s port %d key %d
    token t_curr = token_next(t);
    uint id, port, k;
    float v;
    char ip[16];
    address src;

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
        send_get(src, k, routing_prev(r), self);
    }
    return;
}

void put(token t, routing r, address self){
    //put host idNode %d ip %s port %d key %d value %f id %d
    token t_curr = token_next(t);
    uint id, port, k,  req_id;
    float v;
    char ip[16];
    address src;
    
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

    if(routing_lid(r) <= k && k <= routing_hid(r)){
        node_add(routing_values(r),node_create(k, v));
        send_ack(req_id, src, self);
    }else{
        send_put(src, k, v, req_id, routing_prev(r), self);
    }
    return;
}


uint ack(token t, routing r, address self){
    //ack id req_id
    token t_curr = token_next(token_next(t));
    uint req_id = (uint)atoi(token_value(t_curr));
    printf("%d@%s:%d : Receiving [ACK] %d\n",address_id(self), address_ip(self), address_port(self),req_id);

    return req_id;
}

void update_table(token t, routing r, address self){
    //update_table src idNode %d ip %s port %d id_lower %d amount %d
    token t_curr = token_next(t);
    uint id, port;
    int lower_key, amount;
    char ip[16];
    address src;

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
    }
    return;
}

void send_hello(address src, address dest){
    //{\"cmd\":\"hello\",\"args\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d}}
    char msg[100];
    snprintf(msg,100, "{\"cmd\":\"hello\",\"args\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d}}", address_id(src),address_ip(src),address_port(src));

    printf("%d@%s:%d : Sending [HELLO] to %d@%s:%d\n",address_id(src), address_ip(src), address_port(src), address_id(dest), address_ip(dest), address_port(dest));
    comm_out(dest, msg);
    return;
}

void send_hello_ok(uint id, address resp_addr, node d, address prev, uint req_id, address dest){
    //{\"cmd\":\"hello_ok\",\"args\":{\"id_requested\":%d,\"ip_port_resp\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"data\":{%s},\"prec\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"id_request\":%d}
    char msg[BUF_SIZE];
    char *node_msg = node_print(d);
    snprintf(msg,BUF_SIZE, "{\"cmd\":\"hello_ok\",\"args\":{\"id_requested\":%d,\"ip_port_resp\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"data\":{%s},\"prec\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"id_request\":%d}", id,address_id(resp_addr),address_ip(resp_addr),address_port(resp_addr),node_msg,address_id(prev),address_ip(prev),address_port(prev),req_id);

    printf("%d@%s:%d : Sending [HELLO_OK] to %d@%s:%d with previous %d@%s:%d and node %s \n",address_id(resp_addr), address_ip(resp_addr), address_port(resp_addr), address_id(dest), address_ip(dest), address_port(dest), address_id(prev), address_ip(prev), address_port(prev), node_msg);
    comm_out(dest, msg);
    free(node_msg);
    return;
}

void send_hello_ko(uint id, address dest, address self){
    //{\"cmd\":\"hello_ko\",\"args\":{\"key\":%d}}
    char msg[100];
    snprintf(msg,100,"{\"cmd\":\"hello_ko\",\"args\":{\"key\":%d}}", id);

    printf("%d@%s:%d : Sending [HELLO_KO] to %d@%s:%d\n",address_id(self), address_ip(self), address_port(self), address_id(dest), address_ip(dest), address_port(dest));
    comm_out(dest, msg);
    return;
}
/* Message types */
void send_get(address src, uint k, address dest, address self){
    //{\"cmd\":\"get\",\"args\":{\"host\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"key\":%d}}
    char msg[100];
    snprintf(msg,100,"{\"cmd\":\"get\",\"args\":{\"host\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"key\":%d}}", address_id(src),address_ip(src),address_port(src), k);

    printf("%d@%s:%d : Sending [GET] <%d> of%d@%s:%d to %d@%s:%d \n",address_id(self), address_ip(self), address_port(self),k, address_id(src), address_ip(src), address_port(src), address_id(dest),address_ip(dest),address_port(dest));
    comm_out(dest, msg);
    return;
}

void send_answer(uint k, float v, char *exists, address dest, address self){
    //{\"cmd\":\"answer\",\"args\":{\"key\":%d,\"value\":%f,\"val_exists\":%s}}
    char msg[150];
    snprintf(msg,150,"{\"cmd\":\"answer\",\"args\":{\"key\":%d,\"value\":%f,\"val_exists\":%s}}", k, v, exists);

    printf("%d@%s:%d : Sending [ANSWER] <%d,%f> = %s to %d@%s:%d \n",address_id(self), address_ip(self), address_port(self),k,v, exists, address_id(dest),address_ip(dest),address_port(dest));
    comm_out(dest, msg);
    return;
}

void send_put(address src, uint k, float v, uint req_id, address dest, address self){
    //{\"cmd\":\"put\",\"args\":{\"host\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"key\":%d,\"value\":%f,\"id\":%d}}
    char msg[150];
    snprintf(msg,150,"{\"cmd\":\"put\",\"args\":{\"host\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"key\":%d,\"value\":%f,\"id\":%d}}", address_id(src), address_ip(src), address_port(src), k, v, req_id);

    printf("%d@%s:%d : Sending [PUT] <%d,%f> from %d@%s:%d to %d@%s:%d \n",address_id(self), address_ip(self), address_port(self),k,v ,address_id(src), address_ip(src), address_port(src), address_id(dest),address_ip(dest),address_port(dest));
    comm_out(dest, msg);
    return;
}

void send_ack(uint req_id, address dest, address self){
    //{\"cmd\":\"ack\",\"args\":{\"id\":req_id}}
    char msg[100];
    snprintf(msg,100,"{\"cmd\":\"ack\",\"args\":{\"id\":%d}}", req_id);

    printf("%d@%s:%d : Sending [ACK] to %d@%s:%d \n",address_id(self), address_ip(self), address_port(self), address_id(dest),address_ip(dest),address_port(dest));
    comm_out(dest, msg);
    return;
}
/* Update */
void send_get_resp(address src, uint key, address dest, address self){
    //{\"cmd\":\"get_resp\",\"args\":{\"src\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"key\": %d} 
    char msg[150];
    snprintf(msg,150,"{\"cmd\":\"get_resp\",\"args\":{\"src\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"key\": %d}", address_id(src), address_ip(src), address_port(src),key );

    printf("%d@%s:%d : Sending [GET_RESP] from %d@%s:%d to %d@%s:%d \n",address_id(self), address_ip(self), address_port(self), address_id(src), address_ip(src), address_port(src), address_id(dest),address_ip(dest),address_port(dest));
    comm_out(dest, msg);
    return;
}
void send_ans_resp(uint k, address src, address dest){
    char msg[150];
    snprintf(msg,150,"{\"cmd\":\"answer_resp\",\"args\":{\"key\": %d,\"src\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d}}", k, address_id(src), address_ip(src), address_port(src));

    printf("%d@%s:%d : Sending [ANSWER_RESP] of <%d> to %d@%s:%d \n", address_id(src), address_ip(src), address_port(src), k,  address_id(dest),address_ip(dest),address_port(dest));
    comm_out(dest, msg);
    return;
}

void send_update_table(address src, uint lower_key, uint amount, address dest, address self){
    //"{\"cmd\":\"update_table\",\"args\":{\"src\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"id_lower\": %d, \"amount\": %d}"
    char msg[200];
    snprintf(msg,200,"{\"cmd\":\"update_table\",\"args\":{\"src\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"id_lower\": %d, \"amount\": %d}", address_id(src), address_ip(src),address_port(src), lower_key, amount);

    printf("%d@%s:%d : Sending [UPDATE_TABLE] <%d,%d> from %d@%s:%d to %d@%s:%d \n",address_id(self), address_ip(self), address_port(self), lower_key, amount, address_id(src), address_ip(src), address_port(src), address_id(dest),address_ip(dest),address_port(dest));
    comm_out(dest, msg);
    return;
}
/* Stats */
void send_print(address src, address dest, node n){
    return;
}
void send_get_stat(address src, int msg_get, int msg_put, int msg_gest, address dest){
    return;
}

int end(){
    return 0;
}

int main(int argc, char **argv){
    int i, cmd;
    uint req_id = 0 ;
    uint self_id, dest_id;
    uint self_port, dest_port;
    char self_ip[16];
    char dest_ip[16];
    address client = NULL;
    address server = NULL;
    routing r;
    //statistics s;
    token t;
    for (i=0; i < argc; i++) {
        if (!strcmp(argv[i],"-a")) {
            dest_id = atoi(argv[i+1]);
            strcpy(dest_ip, argv[i+2]);
            dest_port= atoi(argv[i+3]);
            server = address_create(dest_id, dest_ip, dest_port);
            printf("known node : id=%d ip=%s port=%d\n",dest_id,dest_ip,dest_port);
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
    srand (time (NULL));
    if( (self_id = rand() % MAX_NODES) == dest_id){
        srand (time (NULL));
        self_id = rand() % MAX_NODES;
    }
    client= address_create(self_id, self_ip, self_port);
    printf("self node : id=%d ip=%s port=%d\n",self_id,self_ip,self_port);
    if(server!=NULL){
        send_hello(client, server);
        address_clean(server);
    }
    r = routing_create(client,client, address_id(client)+1, address_id(client),node_create(address_id(client),0));
    while(1){
        char *msg = comm_in(client);
        //printf("received %s\n", msg);
        t = token_generate(msg); 
        cmd = cmd_sort(t);
        //token_print(t);
        switch(cmd){
            case(HELLO):
                req_id = hello(t,r,client, req_id);
                break;
            case(HELLO_OK):
                hello_ok(t,r,client);
                break;
            case(HELLO_KO):
                do{
                    if( (self_id = rand() % MAX_NODES) == dest_id){
                        self_id = rand() % MAX_NODES;
                    }
                    address_id_update(client, self_id);
                    send_hello(client, server);
                }while(cmd == HELLO_KO);
                address_clean(server);
                break;
            case(GET):
                get(t,r,client);
                break;
            case(ANSWER):
                //TODO
                break;
            case(PUT):
                put(t,r,client);
                break;
            case(ACK):
                ack(t,r,client);
                break;
            case(GET_RESP):
                //TODO
                break;
            case(ANSWER_RESP):
                //TODO
                break;
            case(UPDATE_TABLE):
                update_table(t, r, client);
                break;
            default:
                printf("well... code %d\n", cmd);
                exit( cmd);
                break;
        }
        free(msg);
    }
    routing_clean(r);
    token_clean(t);
    address_clean(client);
    return 0;
}


       /*char *msg;
        do{
            send_hello(client, server);
            printf("Node awaiting for an insertion answer ...\n");

            msg = comm_in(client);
            printf("received %s\n", msg);
            t = token_generate(msg); 
            cmd = cmd_sort(t);
            if(cmd==HELLO_KO){
                if( (self_id = rand() % MAX_NODES) == dest_id){
                    self_id = rand() % MAX_NODES;
                }
                address_id_update(client, self_id);
            }
        }while(cmd == HELLO_KO);
        address_clean(server);
        free(msg);
        hello_ok(t,r,client);*/
    //}