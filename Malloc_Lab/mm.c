#include<stdlib.h>
#include <stdint.h>
#include<stdio.h>
#define BLOCK_SIZE 16
#define FREE 0
#define USED 1
#define MAX_LEVEL 26

typedef struct blockE{
    uint16_t length;
    uint16_t flag;
} __attribute__((packed)) blockE_t;
typedef struct blockH{
    blockE_t lastEnd;
    uint16_t length;
    uint16_t flag;
    struct blockH *next;
} __attribute__((packed)) blockH_t;

void * kmalloc(size_t size);
void addFreeChunk(uint16_t len,blockH_t * ptr);
int get_index(uint16_t a);
void removeFromList(blockH_t * ptr);
void kfree(void* p);


blockH_t * block_heads[MAX_LEVEL];
void * heap_start;
blockH_t * heap_end;


int main(){
    heap_start=malloc(4096*BLOCK_SIZE);
    //heap_end=heap_start+(4095*32);
    ((blockH_t *)heap_start)->length=4094;
    ((blockH_t *)heap_start)->flag=FREE;
    ((blockH_t *)heap_start)->next=NULL;
    ((blockH_t *)heap_start)->lastEnd.length=0;
    ((blockH_t *)heap_start)->lastEnd.flag=USED;
    heap_end=(blockH_t*)((uint64_t)heap_start+4095*BLOCK_SIZE);
    //printf
    
    heap_end->flag=USED;

    addFreeChunk(4095,heap_start);
    //(blockE_t *)((uint64_t)(heap_start)+4096*BLOCK_SIZE-4)->length=4095;
    /* for(int i=0;i<MAX_LEVEL;i++)
            {
                printf("\n%x",block_heads[i]);
                if(block_heads[i])
                    {printf("    %d",block_heads[i]->length);
                    for(blockH_t *p=block_heads[i];p;p=p->next)
                        printf("%d",p->length);
            }}  */

    //printf("\n STOP \n");

    //(blockE_t *)((uint64_t)(heap_start)+4096*BLOCK_SIZE-4)->flag=FREE;
    //free(kmalloc(22));

    printf("%x\n",kmalloc(16));
    //printf("%x\n",kmalloc(53245+4200+4200+1000+260+150*16));
    printf("%x\n",kmalloc(64));
    void * p4=kmalloc(16*2);
    void * p1=kmalloc(16*16);
            kmalloc(16*16);
    void * p5=kmalloc(16*2);
    void * p2=kmalloc(16*16);
    
    void * p3=kmalloc(16*2);
    kmalloc(16*32);
    void * p6=kmalloc(16*2);
    printf("%x %x %x %x",p1,p3,p5,p6);
    

    kfree(p4);
    kfree(p1);
    kfree(p5);
    kfree(p3);
    //kmalloc(16*1);
    kmalloc(16*16);
    //kfree(p3);
    //kfree(p6);
    //kfree(p5);

for(int i=0;i<MAX_LEVEL;i++)
            {
                printf("\n%x",block_heads[i]);
                if(block_heads[i])
                    {
                    for(blockH_t *p=block_heads[i];p;p=p->next)
                        printf("%  d  ",p->length);
            }}


    /*for(int i=0;i<MAX_LEVEL;i++)
            {
                printf("\n%x",block_heads[i]);
                if(block_heads[i])
                    printf("    %d",block_heads[i]->length);
            }  */
    
    //printf("%x\n",kmalloc(2243));


}

void * kmalloc(size_t size){
    uint16_t block_needed=(size/BLOCK_SIZE);
    if(size%BLOCK_SIZE)
        block_needed++;
    blockH_t * block_head=NULL;

    int i;
    if((i=get_index(block_needed))!=-1)
        {
            for(;i<MAX_LEVEL;i++)
            {
                if(!block_heads[i])
                    continue;
                if(block_heads[i]->length>=block_needed)
                    {    
                        
                        block_head=block_heads[i];
                        block_heads[i]=block_head->next;
                        break;
                    }

                for(blockH_t *ptr=block_heads[i];ptr->next;ptr=ptr->next)
                {
                    if(ptr->next->length>=block_needed)
                    {
                        block_head=ptr->next;
                        ptr->next=block_head->next;
                        break;
                    }
                }
            }

            if(!block_head)
                return NULL;

            blockH_t *freeHead=block_head+1+block_needed;




            block_heads[i]=block_head->next;
            uint16_t newlen=block_head->length-block_needed;
            freeHead->lastEnd.length=block_head->length=block_needed;
            freeHead->lastEnd.flag=block_head->flag=USED;


            addFreeChunk(newlen,freeHead);

        

            return (blockH_t *) ((uint64_t) block_head+BLOCK_SIZE);
        }






    
    return NULL;

}


void addFreeChunk(uint16_t len,blockH_t * ptr){
    
    if(len==0)
        return;
    if(ptr->lastEnd.flag==FREE)
        {
            ptr=(blockH_t * )((uint64_t)ptr-(ptr->lastEnd.length+1)*BLOCK_SIZE);
            removeFromList(ptr);
            len=len+ptr->length+1;
        }

    blockH_t * nextHead=((blockH_t *)((uint64_t)ptr+len*BLOCK_SIZE));

    if(nextHead->flag==FREE)
        {
 
            removeFromList(nextHead);

            len+=nextHead->length+1;
            nextHead=((blockH_t *)((uint64_t)ptr+len*BLOCK_SIZE));
        }

    ptr->flag=FREE;
    ptr->length=len-1;
    nextHead->lastEnd.length=len-1;
    nextHead->lastEnd.flag=FREE;

    int i=0;
    for(int x=(ptr->length)>>1;x;x=x>>1)
        i++;

    ptr->next=block_heads[i];
    block_heads[i]=ptr;

}

int get_index(uint16_t a){
    int i=0;
    for(int x=a>>1;x;x=x>>1)
        i++;
    for(;i<MAX_LEVEL;i++)
    {
        if(block_heads[i]!=NULL)
            return i;
    }
    return -1;
}

void removeFromList(blockH_t * ptr){
    int i=0;
    for(int x=(ptr->length)>>1;x;x=x>>1)
        i++;

    blockH_t *p=block_heads[i];
    if(p==ptr)
    {
        block_heads[i]=p->next;
        return;
    }

    for(;;p=p->next)
        if(p->next==ptr)
        {
            p->next==ptr->next;
            return;
        }
}

void kfree(void* p){

    blockH_t * ptr=(blockH_t *)((uint64_t)p-BLOCK_SIZE);
    
    addFreeChunk(ptr->length+1,ptr);
}

