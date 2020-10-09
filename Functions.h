//
// Created by Ido Yam on 27/09/2020.
//

#ifndef PROJECT_FUNCTIONS_H
#define PROJECT_FUNCTIONS_H

#include <stdlib.h>


void* xmalloc(size_t sz);
char* strcpy_t(char* dest, const char* source);
int strcmp_t(const char string1[], const char string2[]);
void myMemCpy(void *dest, void *src, size_t n);
int strlen_t(const char *s);
char* strcat_t(char* destination, const char* source);



#endif //PROJECT_FUNCTIONS_H
