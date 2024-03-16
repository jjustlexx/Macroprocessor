#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "macro.h"
#define size 1024

Deftab* deftab = NULL;
Argtab* argtab = NULL;
Nametab* nametab = NULL;

char* separators = " :.,\t\n";
char* substr_start = "#macro";
char* substr_end = "#mend";
int in_macro = 0;

//Создание объекта Argtab
static void define_argtab(char* _name, char* ptr) {
    //Если объект с нужным именем существует - создаем таблицу
    if(get_nametab(_name) == NULL)
        return;

    Argtab* tmp;

    if(argtab == NULL) {
        argtab = (Argtab*)malloc(sizeof(Argtab));
        argtab->next = NULL;
        tmp = argtab;
    }else {
        tmp = (Argtab*)malloc(sizeof(Argtab));
        tmp->next = argtab->next;
        argtab->next = tmp;
    }

    char* cpy = strdup(ptr);
    int num_of_param = 0;
    const char* token = strtok(cpy, separators);
    //Убираем первый токен, так как это имя
    token = strtok(NULL, separators);
    ptr += strlen(_name) + 1;
    //Считаем кол-во параметов
    while(token != NULL) {
        num_of_param++;
        token = strtok(NULL, separators);
    }

    tmp->key = (char*)malloc(strlen(_name)+1);
    tmp->val = (char*)malloc(strlen(ptr) + 1);
    tmp->num = num_of_param;

    strcpy(tmp->key, _name);
    strcpy(tmp->val, ptr);

}

//Получение Nametab по имени макроса
static Nametab* get_nametab(char* key) {
    Nametab* tmp = nametab;

    while(tmp!=NULL) {
        if (strcmp(tmp->key, key) == 0)
            return tmp;
        tmp = tmp->next;
    }

    return NULL;
}

//Создание Nametab и добавление в него имени
static void define_name_nametab(char* _name) {
    Nametab* tmp = (Nametab*)malloc(sizeof(Nametab));
    tmp->key = (char*)malloc(strlen(_name) + 1);

    strcpy(tmp->key, _name);

    tmp->next = nametab;
    nametab = tmp;
}

static void define_key(char* ptr) {
    ptr += strlen(substr_start) + 1;
    char* cpy = strdup(ptr);
    char* tmp = strtok(ptr, separators);
    //Находим имя макроса
    char* name[size];
    strcpy(name, tmp);

    if(get_nametab(name) == NULL) {
        in_macro = 1;
        //Передаем его в функцию, которая создает Nametab с именем этого макроса
        define_name_nametab(name);

        tmp = strtok(NULL, separators);
        //Проверяем, есть ли параметры после имени
        //Если есть передаем их в функцию, которая создает Agrtab с текущим именем макроса
        if(tmp != NULL ) {
            define_argtab(name, cpy);
        }
    }
}

static Deftab* get_last_deftab() {
    if (deftab == NULL) {
        deftab = (Deftab*)malloc(sizeof(Deftab));
        deftab->next = NULL;
        return deftab;
    }

    Deftab* tmp = deftab;

    while (tmp->next!=NULL)
        tmp = tmp->next;

    tmp->next = (Deftab*)malloc(sizeof(Deftab));
    tmp = tmp->next;
    tmp->next = NULL;

    return tmp;
}

static void define_deftab(FILE* _file) {
    char* str[size];
    // const char* cpy;
    Deftab* begin = get_last_deftab();
    fgets(str, sizeof(str), _file);
    begin->str = (char*)malloc(strlen(str) + 1);
    strcpy(begin->str, str);
    nametab->begin = begin;

    while(fgets(str, sizeof(str), _file) != NULL) {
        const char* substr = strstr(str, substr_end);
        if(substr != NULL) {
            Deftab* end = deftab;
            while(end->next != NULL)
                end = end->next;
            // end->str = (char*)malloc(strlen(cpy) + 1);
            // strcpy(end->str, cpy);
            nametab->end = end;
            in_macro = 0;
            break;
        }
        Deftab* tmp = get_last_deftab();
        tmp->str = (char*)malloc(strlen(str) + 1);
        strcpy(tmp->str, str);
        // cpy = strdup(str);
    }
}

static Nametab* has_macro(char* _str) {
    Nametab* tmp = nametab;
    while (tmp != NULL) {
        if(strstr(_str, tmp->key) != NULL)
            return tmp;
        tmp = tmp->next;
    }

    return NULL;
}

static Argtab* get_args(char* key) {
    Argtab* tmp = argtab;

    while(tmp != NULL) {
        if(strcmp(tmp->key, key) == 0)
            return tmp;
        tmp = tmp->next;
    }

    return NULL;
}

static void remove_comments(const Deftab* tmp) {
    const char* sign = "//";
    const char* comment = strstr(tmp->str, sign);
    // printf(tmp->str);
    if(comment != NULL) {
        char* cpy = (char*)malloc(strlen(tmp->str));
        memset(cpy, '\0', strlen(tmp->str) + 1);
        strncpy(cpy, tmp->str, strlen(tmp->str) - strlen(comment));
        // printf(cpy);
        // // if(cpy[strlen(cpy)] != '\n')
        // //    strcat(cpy, "\n");
        // printf("%s\n", cpy);
        // char* c = strtok(cpy, separators);
        // if(c == NULL)
        //     memset(cpy, '\0', strlen(cpy));
        //
        //
        if(cpy[strlen(cpy) - 1] != '\n')
            strcat(cpy, "\n");
        strcpy(tmp->str, cpy);
        // // printf(cpy);
    }
}

static void macro_call(const Nametab* ptr, FILE* out, char* str) {
    const Deftab* dtmp = ptr->begin;
    const Argtab* atmp = get_args(ptr->key);
    
    if(atmp == NULL) {
        while(dtmp != ptr->end) {
            remove_comments(dtmp);
            fprintf(out, "%s", dtmp->str);
            dtmp = dtmp->next;
        }
        fprintf(out, "%s", dtmp->str);
    }else {
        char* formal_index_arr[atmp->num];
        char* real_index_arr[atmp->num];
        int formal_index = 0;
        int real_index = 0;
        const char* token = strtok(atmp->val, separators);

        while(token != NULL) {
            formal_index_arr[formal_index] = (char*)malloc(strlen(token));
            strcpy(formal_index_arr[formal_index], token);
            formal_index++;
            token = strtok(NULL, separators);
        }

        const char* strtmp = strtok(str, separators);
        strtmp = strtok(NULL, separators);

        while(strtmp != NULL) {
            real_index_arr[real_index] = (char*)malloc(strlen(strtmp));
            strcpy(real_index_arr[real_index], strtmp);
            real_index++;
            strtmp = strtok(NULL, separators);
        }

        while(dtmp != NULL) {
            for(int i = 0; i < atmp->num; ++i) {
                const char* par = strstr(dtmp->str, formal_index_arr[i]);
                if(par != NULL) {
                    char* copy = (char*)malloc(strlen(dtmp->str));
                    memset(copy, '\0', strlen(dtmp->str));
                    strncpy(copy, dtmp->str,strlen(dtmp->str) - strlen(par));
                    par += strlen(formal_index_arr[i]);
                    strcat(copy, real_index_arr[i]);
                    strcat(copy, par);
                    strcpy(dtmp->str, copy);
                }
            }

            fprintf(out, "%s", dtmp->str);
            if(dtmp == ptr->end)
                break;
            dtmp = dtmp->next;
        }
    }
}

static void clear_argtab() {
    Argtab* tmp = argtab;

    while(tmp != NULL){
        argtab = tmp;
        tmp = tmp->next;
        free(argtab->key);
        free(argtab->val);
        free(argtab);
    }
}
static void clear_deftab() {
    Deftab* tmp = deftab;

    while(tmp != NULL){
        deftab = tmp;
        tmp = tmp->next;
        free(deftab->str);
        free(deftab);

    }
}
static void clear_nametab() {
    Nametab* tmp = nametab;
    while(tmp != NULL){
        nametab = tmp;
        tmp = tmp->next;
        free(nametab->key);
        free(nametab);
    }
}

void macro_init(const char* _input, const char* _output) {
    FILE* input = fopen(_input, "r");
    FILE* output = fopen(_output, "w");

    char buff[size];

    while (fgets(buff, sizeof(buff), input) != NULL) {
        char* str = strstr(buff, substr_start);
        if(str != NULL) {
            define_key(str);
            // in_macro = 1;
        }
        if(in_macro == 1) {
            define_deftab(input);
            continue;
        }

        const Nametab* tmp = has_macro(buff);
        if(tmp != NULL) {
            macro_call(tmp, output, buff);
            continue;
        }

        fprintf(output, buff, strlen(buff));
    }
    fclose(input);
    clear_deftab();
    clear_nametab();
    clear_argtab();
}
