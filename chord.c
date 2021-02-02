#include "chord.h"

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