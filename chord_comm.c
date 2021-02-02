#include "chord_comm.h"

char *comm_in(const address self, int sock_id){

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
    //{\"cmd\":\"stats\",\"args\":{\"src\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"nb_get\":%d,\"nb_put\":%d,\"nb_other\":%d}
    char msg[250];
    char *msg_src = address_print(src);
    char *msg_dest = address_print(dest);
    char *msg_self = address_print(self);
    snprintf(msg,150, "{\"cmd\":\"stats\",\"args\":{\"src\":{\"idNode\":%d,\"ip\":\"%s\",\"port\":%d},\"nb_get\":%d,\"nb_put\":%d,\"nb_other\":%d}", address_id(src),address_ip(src),address_port(src), msg_get, msg_put, msg_gst);
    printf("%s : Sending [GET_STATS] from %s to %s\n", msg_self, msg_src, msg_dest);
    comm_out(dest, msg);
    free(msg_src);
    free(msg_dest);
    free(msg_self);
    return;
}