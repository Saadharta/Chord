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
    printf("recvfrom socket : %d\n", sock_id);
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
    printf("Node en attente...\n");
    client_adr_len = sizeof(client_adr);
    if (recvfrom(sock_id, buffer, BUF_SIZE, SOCK_NONBLOCK, (struct sockaddr *)&client_adr, &client_adr_len) < 0) {
            perror("recvfrom error");
            exit(EXIT_FAILURE);
    }
    printf("Message reçu : %s\n", buffer);

    close(sock_id);
    return buffer;
}

int comm_out(address dest, char* buff){
    int sock_id, server_port;
    char server_ip[16]; /* address as a string */
    struct sockaddr_in server_adr;
    server_port = address_port(dest);
    strcpy(server_ip, address_ip(dest));

    printf("server_ip <%s>, server_port <%d>\n", server_ip, server_port);

    /* Datagram socket creation */
    /*-------------------------------------------*/
    sock_id = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sock_id < 0 ) {
            perror("socket error");
            exit(EXIT_FAILURE);
    }

    printf("sendto socket : %d\n", sock_id);
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
    printf("%s\n", buffer);

    /* send message to destination */
    /*-----------------------------*/
    if ( (sendto(sock_id, buffer, size, 0, (struct sockaddr*) &server_adr, sizeof(server_adr))) < 0) {
            perror("sendto error");
            exit(EXIT_FAILURE);
    }
    printf("sending done\n");
    close(sock_id);
    return 0;
}

/* Message types */
void send_get(address src, uint key){
    return;
}

void send_get_ans(address src, uint key, uint value, uint req_id){
    //{"cmd":"answer","args":{"key":key,"value":value,"id":req_id}}
    return;
}

void send_put(address src, uint key, float value, uint id, node_data data, routing r){
    if(routing_lid(r) <= key && key <= routing_hid(r)){
        node_data new_data = data_create(key, value);
        data_add(data,new_data);

        send_ack(src,id);
    }else{
        //send put to pred;
        //{"cmd":"put","args":{"host":{"IP":routing->previous->ip,"port":routing->previous->port,"idNode":routing->previous->idNode},"key":key,"value":value,"id":req_id}}
    }
    return;

}
void send_ack(address src, uint req_id){
    //send ack #req_id to src
    //{"cmd":"ack","args":{"id":req_id}}
    return ;
}
/* Insert */
void hello(list l, routing r, address self){
    list l_curr = l;
    uint id, port;
    char ip[16];
    address src;
    char *str;
    do{
        l_curr=list_next(l_curr);
        str = list_value(l_curr);
        if(!strcmp(str, "idNode")){
            id = (uint)atoi(list_value(list_next(l_curr)));
        }
        if(!strcmp(str, "ip")){
            strcpy(ip,list_value(list_next(l_curr)));
        }
        if(!strcmp(str, "port")){
            port = (uint)atoi(list_value(list_next(l_curr)));
        }
    }while(l_curr!=l);
    src = address_create(id,ip,port);
    if(is_between(routing_lid(r), routing_hid(r), id)){
        //node_data trans_data;
        //TODO : extract data if needed
        address prev = routing_prev(r);
        routing_update(r, src,NULL,address_id(src),-1,NULL);
        if(prev == NULL)
            send_hello_ok(address_id(src),self,NULL,self, ++id_req, src);
        else
            send_hello_ok(address_id(src),self,NULL,prev, ++id_req, src);
        
    }else{
        send_hello(src, routing_prev(r));
    }

}
void send_hello(address src, address dest){
    char msg[100];
    snprintf(msg,100, "{\"cmd\":\"hello\",\"args\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d}}", address_id(src),address_ip(src),address_port(src));

    printf("sending %s @%s:%d(%d)\n",msg,address_ip(dest),address_port(dest),address_id(dest));
    printf("%d\n",comm_out(dest, msg));
    return;
}

void send_hello_ok(uint id, address resp_addr, node_data d, address prev, uint req_id, address dest){
    char msg[BUF_SIZE];
    snprintf(msg,BUF_SIZE, "{\"cmd\":\"hello_ok\",\"args\":{\"id_requested\":%d,\"ip_port_resp\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"data\":%s,\"ip_port_prec\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"id_request\":%d}", id,address_id(resp_addr),address_ip(resp_addr),address_port(resp_addr),"data_print(d)",address_id(prev),address_ip(prev),address_port(prev),req_id);

    printf("sending:\n%s \n@%s:%d(%d)\n",msg,address_ip(dest),address_port(dest),address_id(dest));
    comm_out(dest, msg);
    return;
}

void send_hello_ko(uint id){
    return;
}
/* Update */
void send_get_resp(address src, uint key){
    //send cmd get with src and key
    //{"cmd":"ack","args":{"id":req_id}}
    return;
}
void ans_resp(uint key, address src){
    return;
}
void update(address src, uint lower_key, uint amount){
    return;
}
/* Stats */
void send_print(address src){
    return;
}
void send_get_stat(address src, int msg_get, int msg_put, int msg_gest){
    return;
}

int end(){
    return 0;
}

int main(int argc, char **argv){
    int i, cmd;
    uint self_id, dest_id;
    uint self_port, dest_port;
    char self_ip[16];
    char dest_ip[16];
    address client = NULL;
    address server = NULL;
    routing r;
    //statistics s;
    list l;
    for (i=0; i < argc; i++) {
        if (!strcmp(argv[i],"-a")) {
            dest_id = atoi(argv[i+1]);
            strcpy(dest_ip, argv[i+2]);
            dest_port= atoi(argv[i+3]);
            server=address_create(dest_id, dest_ip, dest_port);
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
        self_id = rand() % MAX_NODES;
    }
    client= address_create(self_id, self_ip, self_port);
    printf("self node : id=%d ip=%s port=%d\n",self_id,self_ip,self_port);
    if(server!=NULL){
        char *msg;
        do{
            send_hello(client, server);
            printf("Node awaiting for an insertion answer ...\n");

            msg=comm_in(client);
            printf("received %s\n", msg);
            l = list_generate(msg); 
            cmd = cmd_sort(l);
            if(cmd==HELLO_KO){
                if( (self_id = rand() % MAX_NODES) == dest_id){
                    self_id = rand() % MAX_NODES;
                }
                address_id_update(client, self_id);
            }
        }while(cmd == HELLO_KO);
        address_clean(server);
        free(msg);
    }else{
        r = routing_create(NULL,NULL, address_id(client)+1, address_id(client), NULL);
        char *msg = comm_in(client);
        printf("received %s\n", msg);
        l = list_generate(msg); 
        int cmd = cmd_sort(l);
        switch(cmd){
            case(HELLO):
                list_print(l);
                hello(l,r,client);
                break;
            case(HELLO_OK):
                //TODO
                break;
            case(HELLO_KO):
                //TODO
                break;
            case(GET):
                //TODO
                break;
            case(ANSWER):
                //TODO
                break;
            case(PUT):
                //TODO
                break;
            case(ACK):
                //TODO
                break;
            case(GET_RESP):
                //TODO
                break;
            case(ANSWER_RESP):
                //TODO
                break;
            case(UPDATE_TABLE):
                //TODO
                break;
            default:
                printf("well... code %d\n", cmd);
                exit( cmd);
                break;
        }
        free(msg);
        routing_clean(r);
    }
    list_clean(l);
    address_clean(client);
    return 0;
}