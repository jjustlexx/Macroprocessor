#ifndef MACRO_H
#define MACRO_H
#define size 1024

#include <stdio.h>

typedef struct Deftab {
    char* str;
    struct Deftab* next;
}Deftab;

typedef struct Argtab {
    char* key;
    char* val;
    int num;
    struct Argtab* next;
}Argtab;

typedef struct Nametab {
    char* key;
    Deftab* begin;
    Deftab* end;
    struct Nametab* next;
}Nametab;

void macro_start(const char* input, const char* output);
static void define_argtab(char* _name, char* ptr);
static Nametab* get_nametab(char* key);
static void define_name_nametab(char* _name);
static void define_key(char* ptr);
void macro_init(const char* _input, const char* _output);
static Deftab* get_last_deftab();
static Deftab* get_last_deftab();
static Nametab* has_macro(char* _str);
static Argtab* get_args(char* key);
static void macro_call(const Nametab* ptr, FILE* out, char* str);
#endif
