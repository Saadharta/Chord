#include "cmd_splitter.h"

int cmd_sort(list l){
    char *str = list_value(l);
    printf("identifier : %s\n",str);
    printf("%d\n",strcmp(str,"\"hello\""));
    int ret;
    if (!strcmp(str,"\"hello\"")) {
        ret = HELLO;
    }else if(!strcmp(str,"\"hello_ok\"")){
        ret =  HELLO_OK;
    }else if(!strcmp(str,"\"hello_ko\"")){
        ret =  HELLO_KO;
    }else if(!strcmp(str,"\"get\"")){
        ret =  GET;
    }else if(!strcmp(str,"\"answer\"")){
        ret =  ANSWER;
    }else if(!strcmp(str,"\"put\"")){
        ret =  PUT;
    }else if(!strcmp(str,"\"ack\"")){
        ret =  ACK;
    }else if(!strcmp(str,"\"get_resp\"")){
        ret =  GET_RESP;
    }else if(!strcmp(str,"\"answer_resp\"")){
        ret =  ANSWER_RESP;
    }else if(!strcmp(str,"\"update_table\"")){
        ret =  UPDATE_TABLE;
    }else{
        ret =  -1;
    }
    printf("cmd analysis, returning %d\n",ret);
        return ret;
}

/*int main(int argc, char **argv){  
    //char *chain = "{\"cmd\":\"get\",\"args\":{\"host\":{\"IP\":\"127.0.0.1\",\"port\":75375,\"idNode\":15000},\"key\":50}}"; 
    char *chain = "{\"cmd\":\"hello_ok\", \"args\":{\"id_requested\":xxxx, \"resp\":{\"idNode\":yyyy,\"ip\":YYYY,\"port\":yYyY}, \"data\":{20:50,25:80},\"ip_port_prec\":{\"idNode\":zzzz, \"IP\":ZZZZ, \"port\":zZzZ}, \"id_request\":aAaAa}";
    list l= list_generate( chain);
    printf("\n----------\ngeneration done\n----------\n");
    list_print(l);
    printf("\n----------\nprinting done\n----------\n");
    list_clean(l);
    printf("\n----------\ncleaning done\n----------\n");
    printf("\n");
  

    return 0;
}
*/