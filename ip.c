#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ip.h"


const char* ip_str_repr = "%01X%01X%02X%04X%04X%04X%02X%02X%04X%015s%015s%";


void *xxmalloc(size_t sz){
    void *p;
    p=malloc(sz);
 //   assume(p>0);
    return p;
}

void strcpy1(char dest[], const char source[],int size)
{
    int i = 0;
    while (1)
    {
        dest[i] = source[i];

        if (dest[i] == '\0' || i==size-1)
        {
            break;
        }

        i++;
    }
}

int str_to_int(char* str, int size_of_int){
    // Initialize result
    int res = 0;

    // Iterate through all characters
    // of input string and update result
    for (int i = 0; i<size_of_int; ++i)
        res = res * 10 + str[i] - '0';
    // return result.
    return res;
}

long long power(int base, int exponent) {
    long long result=1;
    for (; exponent>0; exponent--)
    {
        result = result * base;
    }
    return result;
}

void int_to_str(char* str, int size_of_str,long n){


    long long help = power(10,size_of_str);

    for (int i = 0; i<size_of_str; ++i) {
        long long tmp = n % help;
        help=help/10;
        str[i] = (tmp /help)+'0';
    }
}

IPPacket str_to_ip(char* s) {
    IPPacket result = (IPPacket)xxmalloc(sizeof(*result));
    if (strlen(s)<50){
        return NULL;
    }
    result->total_length = str_to_int(s,10);
    strcpy1(result->src_ip,s+10*sizeof(char),16);
    strcpy1(result->dst_ip,s+26*sizeof(char),16);
    result->header_checksum = str_to_int(s+42* sizeof(char),8);
    int data_size = result->total_length-50;
    result->data= xxmalloc(data_size* sizeof(char));
    if(strlen(s)>50) {
        strcpy1(result->data, s + 50 * sizeof(char), 0);
     }
    return result;
}

char* ip_to_str(IPPacket packet) {

    int length_of_msg = packet->total_length*4;
    char* result = (char*) xxmalloc(length_of_msg*sizeof(char));

    if (result == NULL) {
        return NULL;
    }
    int_to_str(result,10,packet->total_length);
    strcpy1(result+10*sizeof(char),packet->src_ip,16);
    strcpy1(result+26*sizeof(char),packet->dst_ip,16);
    int_to_str(result+42* sizeof(char), 8, packet->header_checksum);
    strcpy1(result+ 50*sizeof(char), packet->data,0);

    return result;
}

int ip_calc_checksum(char* src, char* dst){
    return 5;
}

IPPacket create_ip_packet(char* src, char* dst, char* data){
    if (strlen(src)!=16 || strlen(dst)!=16){ return NULL;}
    IPPacket result = (IPPacket)xxmalloc(sizeof(*result));

    result->header_checksum = ip_calc_checksum(src,dst);
    strcpy1(result->src_ip,src,16);
    strcpy1(result->dst_ip,dst,16);
    result->data = data;
    result->total_length = 50 + strlen(data);
    return result;
}

void destroy_ip_packet(IPPacket ipPacket){
    if (ipPacket == NULL){ return;}
    free(ipPacket);
}