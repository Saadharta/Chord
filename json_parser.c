#include "json_parser.h"
#include <stdio.h>

char *deep_cpy(const char *src) {
    size_t len = strlen(src) + 1;
    char *s = malloc(len);
    if (s == NULL)
        return NULL;
    return (char *)memcpy(s, src, len);
}

// memory allocation
json_object *j_obj_alloc(void){
    printf("creating new object\n");
    json_object *jobj = malloc( sizeof(json_object));
    jobj->jcpl = NULL;
    jobj->i_cpl = 0;
    printf("new object created at %p\n", (void*)jobj);
    return jobj; 
}

json_couple *j_cpl_alloc(void){
    printf("creating new couple\n");
    json_couple *jcpl = malloc(sizeof(json_couple));
    jcpl->prev = NULL;
    jcpl->next = NULL;
    printf("new couple created at %p\n", (void*)jcpl);
    return jcpl;
}

void j_print(json_object *jobj){
    json_couple *tmp_cpl;
    printf("{");
    if(jobj->jcpl != NULL){
        tmp_cpl=jobj->jcpl;
        do{
            printf("(%s:",tmp_cpl->jkey);
            if(tmp_cpl->is_obj){
                j_print(tmp_cpl->jobj);
                printf(")");
            }else{
                if(tmp_cpl->is_flt){
                    printf("%f)",tmp_cpl->fval);
                }else{
                    printf("%s)",tmp_cpl->cval);
                }
            }
            tmp_cpl = tmp_cpl->next;
            if(tmp_cpl != jobj->jcpl)
                printf(",");
        }while(tmp_cpl!=jobj->jcpl);
        
    }
    printf("}");
}

int j_free(json_object *jobj){
    printf("cleaning address of jobj : %p having %d couples \n", jobj, jobj->i_cpl);
    struct json_couple_t *tmp_cpl;
    if(jobj->i_cpl){
        if(jobj->jcpl == NULL){
            free(jobj);
            printf("jobj has no couples\n");
        }else{
            tmp_cpl = jobj->jcpl->prev;

            while(tmp_cpl!=jobj->jcpl){
                
                if(tmp_cpl->is_obj){
                    printf("data is jobject : %p\n", tmp_cpl->jobj);
                    //printf("couple is obj\n");
                    j_free(tmp_cpl->jobj);
                }else{
                    if(tmp_cpl->is_flt){
                        //printf("Can't free a float?\n");//free(tmp_cpl->fval);
                    }else{
                        //printf("Can't free char *?\n");
                        printf("data is cval : %p\n", tmp_cpl->cval);
                        free(tmp_cpl->cval);
                    }
                }
                free(tmp_cpl->jkey);
                tmp_cpl = tmp_cpl->prev;
                tmp_cpl->next->prev=NULL;
                printf("cleaning cpl : %p\n", tmp_cpl->next);
                free(tmp_cpl->next);
            }
            printf("cleaning cpl : %p\n", tmp_cpl);
            free(tmp_cpl);
        }
    }
    printf("cleaning object : %p\n", jobj);
    free(jobj);
    return 0;
}


void j_add(json_object *jobj, struct json_couple_t *jcpl){
    if(jobj->jcpl == NULL){
        jobj->jcpl = jcpl;
        jcpl->prev = jobj->jcpl;
        jcpl->next = jobj->jcpl;
    }else{
        jcpl->prev = jobj->jcpl->prev;
        jcpl->prev->next = jcpl;
        jcpl->next = jobj->jcpl;
        jobj->jcpl->prev = jcpl;
    //printf("address of cpl->prev->next : %p\n", jcpl->prev->next);
    }    
    //printf("address of cpl->prev : %p\n", jcpl->prev);
    //printf("address of cpl->next : %p\n", jcpl->next);
    jobj->i_cpl+=1;
    printf("Object has %d couples after add\n", jobj->i_cpl);
}


void j_cpl_kadd(struct json_couple_t *jcpl, char *k){
    printf("adding key %s to cpl\n",k);
    jcpl->jkey = deep_cpy(k);
}

void j_cpl_vadd(struct json_couple_t *jcpl, bool is_f, bool is_o, void *val){
    jcpl->is_flt = is_f;
    jcpl->is_obj = is_o;
    if(jcpl->is_obj){
        jcpl->jobj = val;
        printf("adding object to %s\n", jcpl->jkey);
    }else{
        if(jcpl->is_flt){
            float *v = (float *)val;
            //memcpy(&jcpl->fval,val, sizeof(float));
            jcpl->fval = *v;
            printf("adding float %f to %s\n",jcpl->fval, jcpl->jkey);
        }else{
            jcpl->cval = deep_cpy(val);
            printf("adding string %s to %s\n",jcpl->cval,jcpl->jkey);
        }
    }
}


void conv_to_json(char *str, json_object *jobj){
    char c[strlen(str)];
    memcpy (c,str,strlen(str));
    int i_c, len = 0;
    bool is_k, is_v, is_o;
    int obj_cpt = 0;
    int o_beg, o_end, o_len;
    int k_beg, k_end, k_len;
    bool is_f;
    int v_beg, v_end, v_len;

    printf("input:\t%s\n",c);
    json_couple *jcpl; 
    len = strlen(str);
    is_k = true;
    is_o = false;
    k_beg = 2;
    i_c=0;
    while(++i_c < len){
        switch(c[i_c]){
        case '{':
            if(!is_o){
                is_o = true;
                o_beg = i_c;
                }
            ++obj_cpt;
            break;
        case ':' :
            if(!is_o){
                jcpl= j_cpl_alloc();
                char *key;
                is_v = true;
                is_k = false;
                k_end = i_c-2;
                if(k_beg && k_end){
                    k_len = k_end-k_beg+2;
                    //printf("k_beg : %d; k_end : %d\n", k_beg, k_end);
                    key = calloc(k_len, sizeof(char));
                    snprintf(key, k_len, "%.*s", k_len, c+k_beg);
                    //printf("key:\t%s\n", key);
                    j_cpl_kadd(jcpl, key);
                    free(key);
                }
                v_beg = i_c+1;
            }
            //printf(" | ");
            break;
        case ',':
            if(!is_o){
                char *val;
                is_k = true;
                is_v = false;
                v_end = i_c-1;
                if(v_beg && v_end){
                    if(c[v_beg]=='"' && c[v_end]=='"'){
                        v_end--; 
                        v_beg++;
                        is_f = false;
                    }else{
                        is_f = true;
                    }
                    v_len = v_end-v_beg+2;
                    //printf("v_beg : %d; v_end : %d\n", v_beg, v_end);
                    val = calloc(v_len, sizeof(char));
                    snprintf(val, v_len, "%.*s", v_len, c+v_beg);
                    printf("val:\t%s\n", val);
                    if(is_f){
                        float flt = strtof(val, NULL);
                        j_cpl_vadd(jcpl, true, false, &flt);
                    }else{
                        j_cpl_vadd(jcpl,false, false, (void *)val);
                    }
                    j_add(jobj, jcpl);
                    free(val);
                }
                k_beg = i_c+2;
            }
            //printf(" ],[ ");
            break;
        case '}':
            if(!is_o){
                char *val;
                is_v = false;
                v_end = i_c-1;
                if(v_beg && v_end){
                    if(c[v_beg]=='"' && c[v_end]=='"'){
                        v_end--; 
                        v_beg++;
                        is_f = false;
                    }else{
                        is_f = true;
                    }
                    v_len = v_end-v_beg+2;
                    //printf("v_beg : %d; v_end : %d\n", v_beg, v_end);
                    val = calloc(v_len, sizeof(char));
                    snprintf(val, v_len, "%.*s", v_len, c+v_beg);
                    printf("val:\t%s\n", val);
                    if(is_f){
                        float flt = strtof(val, NULL);
                        j_cpl_vadd(jcpl, true, false, &flt);
                    }else{
                        j_cpl_vadd(jcpl,false, false, (void *)val);
                    }
                    j_add(jobj, jcpl);
                    free(val);
                }
            }else{
                if(! (--obj_cpt)){
                    o_end = i_c;
                    if(o_beg && o_end){
                        printf("about to alloc to json object done\n");
                        char *obj;
                        o_len = o_end-o_beg+2;
                        printf("o_beg : %d; o_end : %d\n", o_beg, o_end);
                        obj = calloc(o_len, sizeof(char));
                        snprintf(obj, o_len, "%.*s ", o_len, c+o_beg);
                        printf("obj:\t%s\n", obj);
                        // alloc
                        json_object *cpl_jobj = j_obj_alloc();
                        conv_to_json(obj, cpl_jobj);
                        printf("alloc to json object done\n");
                        j_cpl_vadd(jcpl,false, true, (void *)cpl_jobj);
                        j_add(jobj, jcpl);
                        free(obj);
                    }
                }
            }
            break;
        default:
            break;
        }
    }    
}

char *conv_to_string(json_object json);

int main(int argc, char **argv){   
    json_object *j_obj = j_obj_alloc();
    conv_to_json("{\"cmd\":\"get\",\"args\":{\"host\":{\"IP\":\"127.0.0.1\",\"port\":75375,\"idNode\":15000},\"key\":50}}", j_obj);
   
    /*//test float
    json_couple *flt_tst = j_cpl_alloc();
    char *key = "float test";
    j_cpl_kadd(flt_tst,key);
    float f = 10.5;
    j_cpl_vadd(flt_tst, true, false, &f);
    j_add(j_obj, flt_tst);

    //test char
    json_couple  *jcpl = j_cpl_alloc();
    char *key2 = "char test";
    j_cpl_kadd(jcpl, key2);
    char *test_cval = "toto";
    j_cpl_vadd(jcpl, false, false, (void *)test_cval);
    j_add(j_obj,jcpl);

    // test obj
    json_couple *cplobj = j_cpl_alloc();
    json_object *j_cplobj = j_obj_alloc();
    j_add(j_obj, cplobj);
    j_cpl_kadd(cplobj, "cplobj");
    j_cpl_vadd(cplobj, false, true, (void *)j_cplobj);
    
    json_couple *j_cplobj1 = j_cpl_alloc();
    j_add(j_cplobj, j_cplobj1);
    j_cpl_kadd(j_cplobj1, "jcplobj_key1");
    char *j_cplobj_cval = "jcplobj_val1";
    j_cpl_vadd(j_cplobj1, false, false, (void *)j_cplobj_cval);

    json_couple *j_cplobj2 = j_cpl_alloc();
    j_add(j_cplobj, j_cplobj2);
    j_cpl_kadd(j_cplobj2, "jcplobj_key2");
    float j_cplobj_fval = 6942.0;
    j_cpl_vadd(j_cplobj2, true, false, &j_cplobj_fval);

    
    //j_cpl = j_obj
    //printf("Jobj_cpl : %d\n", j_obj->i_cpl);*/
    j_print(j_obj);
    printf("\n-----------\nTime to clean!\n-----------\n");
    //

    j_free(j_obj);
    return 0;
}

