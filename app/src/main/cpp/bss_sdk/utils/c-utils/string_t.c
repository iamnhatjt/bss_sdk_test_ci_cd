//
// Created by vnbk on 31/03/2023.
//
#include <stdlib.h>
#include "string_t.h"

static const string_p string_proc = {
        .dispose = string_dispose,
        .clear = string_clear,
        .equals = string_equals,
        .equalsz = string_equalsz,
        .append = string_append,
        .appendz = string_appendz
};

string_t* string_create(const char* buffer, int length) {
    if (buffer) {
        string_t* object = (string_t*) malloc(sizeof(string_t));
        object->proc = &string_proc;
        object->buffer = (char*) malloc(length + 1);
        object->length = length;
        memcpy(object->buffer, buffer, length);
        object->buffer[length] = 0;
        return object;
    }
    return NULL;
}

string_t* string_createz(const char* buffer) {
    return string_create(buffer, strlen(buffer));
}

void string_dispose(string_t* string) {
    if(string){
        if(string->buffer) free(string->buffer);
        free(string);
    }
}

void string_clear(string_t* a){
    if(a){
        free(a->buffer);
        a->buffer = NULL;
        a->length = 0;
    }
}

bool string_equals(string_t* a, string_t* b) {
    if (a == b)
        return true;
    else if (a == NULL || b == NULL)
        return false;
    else if (a->length == b->length)
        return memcmp(a->buffer, b->buffer, a->length) == 0;
    else
        return false;
}

bool string_equalsz(string_t* a, const char* sz) {
    return memcmp(a->buffer, sz, a->length) == 0;
}

string_t* string_append(string_t* src, const char* append, int length){
    if(append){
        string_t* object = (string_t*) malloc(sizeof(string_t));
        object->proc = &string_proc;
        object->length = src->length + length;
        object->buffer = (char*) malloc(object->length + 1);
        memcpy(object->buffer, src->buffer, src->length);
        memcpy(object->buffer + src->length, append, length);
        object->buffer[object->length] = 0;

        string_dispose(src);
        return object;
    }
    return src;
}

string_t* string_appendz(string_t* src, const char* append){
    if(append)
        return string_append(src, append, strlen(append));
    return src;
}

