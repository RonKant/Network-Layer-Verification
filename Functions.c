//
// Created by Ido Yam on 27/09/2020.
//
#include "Functions.h"

void* xmalloc(size_t sz){
    void *p;
    p=malloc(sz);
    //assume(p>0);
    return p;
}
char* strcpy_t(char* dest, char* source){
    //assume(dest > 0 && source > 0);
    if(dest == NULL || source == NULL)
        return NULL;
    char* ptr = dest;
    while(*source != '\0'){
        *dest = *source;
        dest++;
        source++;
    }
    *dest = '\0';
    return ptr;
}
int strcmp_t(char* str1,char* str2){
    //sassert(str1 != NULL && str2 !=NULL);
    if (NULL == str1) {
        return NULL != str2;
    }
    while(*str1 && (*str1==*str2))
        str1++,str2++;
    return *(const unsigned char*)str1-*(const unsigned char*)str2;
}

void myMemCpy(void *dest, void *src, size_t n)
{
    // Typecast src and dest addresses to (char *)
    char *csrc = (char *)src;
    char *cdest = (char *)dest;

    // Copy contents of src[] to dest[]
    for (int i=0; i<n; i++)
        cdest[i] = csrc[i];
}