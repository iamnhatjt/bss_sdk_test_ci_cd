//
// Created by vnbk on 31/03/2023.
//

#ifndef STRING_T_H_
#define STRING_T_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct string_t string_t;

typedef struct string_p {
    void (*dispose)(string_t*);
    void (*clear)(string_t*);
    bool (*equals)(string_t*, string_t*);
    bool (*equalsz)(string_t*, const char*);
    string_t* (*append)(string_t*, const char*, int);
    string_t* (*appendz)(string_t*, const char*);
}string_p;

struct string_t  {
    const string_p* proc;
    char* buffer;
    int length;
};

string_t*   string_create   (const char* buffer, int length);
string_t*   string_createz  (const char* buffer);
void        string_dispose  (string_t* string);
void        string_clear    (string_t* string);
bool        string_equals   (string_t* a, string_t* b);
bool        string_equalsz  (string_t* a, const char* sz);
string_t* 	string_append	(string_t* src, const char* append, int length);
string_t* 	string_appendz	(string_t* src, const char* append);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif //STRING_T_H
