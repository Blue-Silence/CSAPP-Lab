#include "cachelab.h"
#include <stdlib.h>
#include <stdio.h>

#define END_BREAK {if(fv) printf("\n"); return 0;}
#define V_OUTPUT(X) {if(fv) printf(X);}
                

int fv,miss,hit,evict;
int fs,fe,fb;

typedef struct line
{
    unsigned tag,vaild,life;
} line;

void parseFile(FILE * input,struct line * p);
int opf(char op,unsigned long long addr,struct line * p);

int main(int argc, char *argv[])
{

    printf("abcabc");

    FILE * input;


    for(int i=1;i<argc;i++)
    {
        if (argv[i][1]=='s')
        {
            i++;
            fs=atoi(argv[i]);
            continue;
        }

        if (argv[i][1]=='E')
        {
            i++;
            fe=atoi(argv[i]);
            continue;
        }

        if (argv[i][1]=='b')
        {
            i++;
            fb=atoi(argv[i]);
            continue;
        }

        if (argv[i][1]=='t')
        {
            i++;
            input=fopen(argv[i],"r");
            continue;
        }

        if (argv[i][1]=='v')
        {
            fv=1;
            continue;
        }

        if (argv[i][1]=='h')
        {
            printf("Sorry~Help yourself!\n");
            return 0;
        }

    }

    struct line * p=malloc(fs*fe*sizeof(line));
    for(int i=0;i<fs*fe;i++)
        {
            p[i].vaild=0;
            p[i].life=0;
        }

    parseFile(input,p);

    printSummary(hit, miss, evict);
    return 0;
}

void parseFile(FILE * input,struct line * p){
    int size;
    char op;
    unsigned long long addr;
    while(1)
    {
        if(fscanf(input," %c %llx,%d",&op,&addr,&size)==EOF)
            break;

        if (op!='I')
        {
            if (fv)
                printf("%c %llx,%d",op,addr,size);

            opf(op,addr,p);
        }
    }
}

int opf(char op,unsigned long long addr,struct line * p){
    unsigned long long s,tag;
    s=addr<<(64-fs-fb)>>(64-fs);
    tag=addr>>(fs+fb);
    p=p+fe*s;

    for(int i=0;i<fe;i++)
    {
        if(p[i].tag==tag&&p[i].vaild==1)
        {

            V_OUTPUT(" hit")
            hit++;

            for(int i=0;i<fe;i++)
                p[i].life++;
            
            p[i].life=0;
            END_BREAK
        }
    }

    V_OUTPUT(" miss")
    miss++;

    for(int i=0;i<fe;i++)
    {
        
        if(p[i].vaild==0)
        {
            
            p[i].vaild=1;
            p[i].tag=tag;

            for(int i=0;i<fe;i++)
                p[i].life++;
            
            p[i].life=0;

            if(op=='M')
            {
                hit++;
                V_OUTPUT(" hit")
            }
            END_BREAK
        }
    }

    int maxl,maxi;
    maxl=maxi=0;
    for(int i=0;i<fe;i++)
    {
        if (p[i].life>maxl)
        {
            maxl=p[i].life;
            maxi=i;
        }
    }

    for(int i=0;i<fe;i++)
        p[i].life++;
    p[maxi].tag=tag;
    p[maxi].life=0;

    evict++;
    V_OUTPUT(" eviction")
    if(op=='M')
    {
        hit++;
        V_OUTPUT(" hit")
    }
    END_BREAK
}