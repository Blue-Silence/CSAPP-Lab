#include<stdlib.h>
#include <stdint.h>
#include<stdio.h>
#include <string.h>

#define BLOCK_SIZE 16
#define FREE 0
#define USED 1
#define MAX_LEVEL 26



#include<unistd.h>
#define mem_sbrk sbrk
#define mem_pagesize getpagesize





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

void * mm_malloc(size_t size);
void mm_free(void* p);
void *mm_realloc(void *ptr, size_t size);
int mm_init();

void addFreeChunk(uint16_t len,blockH_t * ptr);
int get_index(uint16_t a);
void removeFromList(blockH_t * ptr);



blockH_t * block_heads[MAX_LEVEL];
blockH_t * heap_start;
blockH_t * heap_end;
int page_size;


void * malloc_h(size_t size){
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
            p->next=ptr->next;
            return;
        }
}

void mm_free(void* p){

    blockH_t * ptr=(blockH_t *)((uint64_t)p-BLOCK_SIZE);
    
    addFreeChunk(ptr->length+1,ptr);
}



int mm_init(){
    page_size=mem_pagesize();
    uint16_t block_n=page_size/BLOCK_SIZE;
    void *p=mem_sbrk(page_size);
    if(p==(void*)-1)
        return -1;
    heap_start=p;
    
    heap_start->lastEnd.length=0;
    heap_start->lastEnd.flag=USED;
    
    heap_end=heap_start+(block_n-1);
    
    heap_end->flag=USED;

    addFreeChunk(block_n-1,heap_start);
    return 0;
}



int incr_heap(size_t size){
    int i=(size*2)/page_size;
    if((size*2)%page_size)
        i++;
    if(mem_sbrk(i*page_size)==(void *)-1)
        return -1;

    uint16_t block_n=i*(page_size/BLOCK_SIZE);
    heap_end[block_n].flag=USED;
    heap_end[block_n].length=0;
    addFreeChunk(block_n,heap_end);
    heap_end=heap_end+block_n;
    return 0;
}

void *mm_malloc(size_t size){
    void *p=malloc_h(size);
    if(p==NULL)
    {
        incr_heap(size);
        return malloc_h(size);
    }
    return p;
}


void *mm_realloc(void *ptr, size_t size){
    blockH_t *p=(blockH_t *)((uint64_t)ptr-BLOCK_SIZE);
    if(!size)
        mm_free(ptr);
    if(!ptr)
        return malloc(size);
    
    void *t=malloc(size);
    memmove(t,p+1,p->length*BLOCK_SIZE);
    mm_free(p+1);
    return t;
}