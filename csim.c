#include "cachelab.h"
#include <stdlib.h>
#include <stdio.h>


int fv,miss,hit,evict;
int fs,fe,fb;

typedef struct line
{
    unsigned tag,vaild,used;
} line;

void parseFile(FILE * input,struct line * p);
int opf(char op,unsigned long long addr,struct line * p);

int main(int argc, char *argv[])
{

    FILE * input;


    for(int i=1;i<=argc;i++)
    {
        if (argv[i][1]='s')
        {
            i++;
            fs=atoi(argv[i]);
            continue;
        }

        if (argv[i][1]='e')
        {
            i++;
            fe=atoi(argv[i]);
            continue;
        }

        if (argv[i][1]='b')
        {
            i++;
            fb=atoi(argv[i]);
            continue;
        }

        if (argv[i][1]='t')
        {
            i++;
            input=fopen(argv[i],"r");
            continue;
        }

        if (argv[i][1]='h')
        {
            printf("Sorry~Help yourself!");
            if (argv[i][1]='v')
                fv=1;
            continue;
        }

    }


    struct line * p=malloc(fs*fe*sizeof(line));
    for(int i=0;i<fs*fe;i++)
        p[i].vaild=0;

    parseFile(input,p);

    printSummary(0, 0, 0);
    return 0;
}

void parseFile(FILE * input,struct line * p){
    int re,size;
    char op;
    unsigned long long addr;
    while(1)
    {
        if(fscanf(input," %c %llx,%d",&op,&addr,&size)==EOF)
            break;

        if (op!='I')
        {
            if(fv)
                printf("%c %llx,%d",op,addr,size);

            opf(op,addr,p);
        }
    }
}

int op(char op,unsigned long long addr,struct line * p){
    unsigned long long s,tag;
    s=addr<<(64-fs-fb)>>(64-fs);
    tag=addr>>(fs+fb);
    p=p+fe*s;
    int is_hitted=0;

    for(int i=0;i<fe;i++)
    {
        if(p[i].tag==tag&&p[i].vaild==1)
        {
            if(fv)
             printf(" hit");
            hit++;

            for(int i=0;i<fe;i++)
                p[i].used=0;
            
            p[i].used=1;
            return 0;
        }
    }

    if(fv)
        printf(" miss");
    miss++;

    for(int i=0;i<fe;i++)
    {
        if(p[i].vaild==0)
        {
            p[i].vaild==1;
            p[i].tag=tag;

            for(int i=0;i<fe;i++)
                p[i].used=0;
            
            p[i].used=1;

            if(op=='M')
            {
                hit++;
                if(fv)
                    printf(" hit");
            }

            return 0;
        }
    }

    for(int i=0;i<fe;i++)
    {
        if(p[i].used==0)
            continue;
        
        p[i].tag=tag;
        evict++;

        if(fv)
            printf(" eviction");
        
        if(op=='M')
        {
            hit++;

            if(fv)
                printf(" hit");
        }

        break;
        
    }

    return 0;

}
