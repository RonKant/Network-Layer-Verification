//
// Created by Ido Yam on 27/09/2020.
//
#include "Functions.h"
// #include<seahorn/seahorn.h>
#include "network.h"
void* xmalloc(size_t sz){
    void *p;
    p=malloc(sz);
    // assume(p>0);
    return p;
}

#define IP_ADDR_SIZE 17
char* strcpy_t(char* dest, const char* source){
    //assume(dest > 0 && source > 0);
    if(dest == NULL || source == NULL)
        return NULL;
    unsigned int i;
    for(i=0; source[i]!='\0' && i<IP_ADDR_SIZE-1; i = i+1){
        dest[i] = source[i];
    }
    dest[i]='\0';
    i++;
    for(;i<IP_ADDR_SIZE;i++){
        dest[i]=1;
    }
    return dest;
}
int strcmp_t(const char string1[], const char string2[] )
{
    int i = 0;
    int flag = 0;
    while (flag == 0)
    {
        if (string1[i] > string2[i])
        {
            flag = 1;
        }
        else if (string1[i] < string2[i])
        {
            flag = -1;
        }

        if (string1[i] == '\0')
        {
            break;
        }

        i++;
    }
    return flag;
}
int strlen_t(const char *s) {
    int i;
    for (i = 0; s[i] != '\0'; i++) ;
    return i;
}
// Function to implement strcat() function in C
char* strcat_t(char* destination, const char* source)
{
    // make ptr point to the end of destination string
    char* ptr = destination + strlen_t(destination);

    // Appends characters of source to the destination string
    while (*source != '\0')
        *ptr++ = *source++;

    // null terminate destination string
    *ptr = '\0';

    // destination is returned by standard strcat()
    return destination;
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
