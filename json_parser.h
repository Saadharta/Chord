#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
/* 
 * prev : previous value in list
 * next : next value in list
 * json value can either be int, char or a meta object
 */
struct json_couple_t{
        struct json_couple_t *prev;
        struct json_couple_t *next;
        char* jkey;
        bool is_flt;
        bool is_obj; 
        union{
            char *cval;
            float fval; 
            struct json_object_t *jobj;   
        };
};
typedef struct json_couple_t json_couple;
/*
 * meta object holding a list of couples 
 */
typedef struct json_object_t{
        struct json_couple_t *jcpl; 
        int i_cpl;
}json_object;

// memory allocation
struct json_object_t *j_obj_alloc();
struct json_couple_t *j_cpl_alloc();
// memory liberation
int j_free(json_object *jobj);

void j_add(json_object *jobj, struct json_couple_t *jcpl);
void j_cpl_kadd(struct json_couple_t *jcpl, char *k);
void j_cpl_vadd(struct json_couple_t *jcpl, bool is_f, bool is_o, void *val);
void conv_to_json(char *str, json_object *jobj);

char *conv_to_string(json_object json);

void j_print(json_object *jobj);

#endif
