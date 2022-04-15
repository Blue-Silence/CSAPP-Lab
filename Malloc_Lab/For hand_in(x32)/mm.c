/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <stdint.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
/*void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}  */






#include<stdlib.h>
#include <stdint.h>
#include<stdio.h>
#include <string.h>

#define BLOCK_SIZE 16
#define FREE 0
#define USED 1
#define MAX_LEVEL 26



#include<unistd.h>

typedef struct blockE{
    uint16_t length;
    uint16_t flag;
    uint32_t nothing;
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
    blockH_t *p;
    uint16_t block_n=page_size/BLOCK_SIZE;
        p=mem_sbrk(page_size);
    if(p==(blockH_t *)-1)
        return -1;
    heap_start=p;
    
    heap_start->lastEnd.length=0;
    heap_start->lastEnd.flag=USED;
    
    heap_end=heap_start+(block_n-1);
    
    heap_end->flag=USED;

    addFreeChunk(block_n-1,heap_start);
}



int incr_heap(size_t size){
    int i=(size*1)/BLOCK_SIZE;
    if((size*1)%BLOCK_SIZE)
        i++;
    if(mem_sbrk(i*BLOCK_SIZE)==(void *)-1)
        return -1;

    uint16_t block_n=i;
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