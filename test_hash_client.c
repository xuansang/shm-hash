#include "ns_shm_hash.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>  
#include <sys/time.h>  
typedef struct IP_TUPLE
{
    unsigned char sip[4];
    unsigned char dip[4];
    unsigned char port[2];
}IP_TUPLE;

unsigned int clalc_key(void*element)
{
    IP_TUPLE* tuple = (IP_TUPLE*)element;
    unsigned long n = 0;
    char *p = (char*)&n;
    tuple->dip[0] = tuple->dip[0]&tuple->port[0];
    tuple->dip[1] = tuple->dip[1]&tuple->port[1];
    memcpy(p,tuple->sip,4);
    memcpy(p+4,tuple->dip,4);
    return n%65535;
}

int compare_ip(void*element,void*element2)
{
    return memcmp(element,element2,sizeof(IP_TUPLE));
}

void data(IP_TUPLE *tuple)
{
    int i = 0;
    for(;i<4;i++)
    {
        tuple->sip[i] =rand()%255;
        tuple->dip[i] = rand()%255;
    }
    tuple->port[0] = tuple->sip[i] =rand()%4;
    tuple->port[1] = tuple->sip[i] =rand()%4;
    
}


int main()
{
    key_t shm_key = 9999;
    unsigned int element_size = sizeof(IP_TUPLE);
    unsigned int max_element_cnt = 65535;
    ns_shm_hash shm_hash;
    int ret = init_SHM_hash(&shm_hash,element_size,max_element_cnt,shm_key);
    if(ret !=0)
    {
        printf("init_SHM_hash failed");
        return 0;
    }

    init_SHM_hash_func(&shm_hash,clalc_key,compare_ip);
    IP_TUPLE ip_tuple;
    struct timeval tv;    
    gettimeofday(&tv,NULL);    
    printf("%u\n",tv.tv_sec * 1000 + tv.tv_usec / 1000);
    while(1)
    {
        data(&ip_tuple);
        ret = find(&shm_hash,&ip_tuple);
        if(ret !=0)
        {
            continue;
        }
        sleep(0.1);
    }

    return 0;
}