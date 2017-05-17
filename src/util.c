#include <stdlib.h>
#include "util.h"
uint64_t bkdr_hash(const char* str){
    if(NULL == str){
        return 0;
    }
    uint64_t seed = 1313131313;
    uint64_t hash = 0;
    while(*str){
        hash = hash * seed + (*str++);
    }
    return hash;
}

void pack_string(conn *c, const char *str) {
    int len;

    len = strlen(str);
    if (len + 2 > c->wsize) {
        /* ought to be always enough. just fail for simplicity */
        str = "SERVER_ERROR output line too long";
        len = strlen(str);
    }

    strcpy(c->wbuf, str);
    strcat(c->wbuf, "\r\n");
    c->wbytes = len + 2;

    return;
}

void out_string(conn *c, const char *str) {
    pack_string(c, str);
    c->wcurr = c->wbuf;
    c->state = conn_write;
    c->write_and_go = conn_read;
    return;
}

