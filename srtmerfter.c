#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

//define srt srtnode type
typedef struct srtnode srtnode;
struct srtnode {
    srtnode *next;
    srtnode *prev;
    char content[256];
    char stime[13];
    char etime[13];
};

srtnode *loadsrt(FILE *fp, srtnode *head, srtnode *tail);
srtnode *genode(FILE *fp, char *line, srtnode *p);
void insertnode(srtnode *head, srtnode *tail, srtnode *p);
void tplus();
void printnode(srtnode *p);

void srtshift(char *t, srtnode *head, srtnode *tail);
void srtmerge(srtnode *head, srtnode *head1);
void node2srt(srtnode *head, srtnode *tail, FILE *fp);

srtnode *head, *tail;

int main(int argc, char **argv)
{
    FILE *fp;
    char *help = "Usage: srtmerfter [options] file...\nOptions:\n\t\t-s\t01,500 file\n\t\t-m\tfile1 file2\n\t\t-h\tdisplay this message.\n";
    if(1 == argc)
    {
        printf("%s", help);
        return -1;
    }
    else if(2 == argc && 0 == strcmp(argv[1], "-h"))
    {
        printf("%s", help);
        return -1;
    }
    else if(0 == strcmp(argv[1], "-s"))
    {
        if(NULL == (fp = fopen(argv[3], "r")))
        {
            printf("Can't open file:%s\n", argv[3]);
            return -2;
        }
        head = loadsrt(fp, head, tail);
        fclose(fp);
        /*
        int len = strlen(argv[3]);
        argv[3][len-3] = 'n';
        argv[3][len-2] = 'e';
        argv[3][len-1] = 'w';
        strcat(argv[3], ".srt");
        printf("%s\n", argv[3]);
        if(NULL == (fp = fopen(argv[3], "w")))
        {
            printf("Can't open file to write:%s\n", argv[3]);
            return -3;
        }
        node2srt(head, tail, fp);
        fclose(fp);
        */
        return 0;
    }
    else if(0 == strcmp(argv[1], "-m"))
    {
    }
    else
    {
        printf("%s", help);
    }
    return 0;
}

srtnode *loadsrt(FILE *fp, srtnode *head, srtnode *tail)
{
    char line[100];
    fgets(line, 100, fp);
    head = genode(fp, line, head);
    tail = head;
    while(NULL != fgets(line, 100, fp))
    {
        if(0 == strcmp(line, "\r\n"))
        {
            continue;
        }
        srtnode *p;
        p = genode(fp, line, p);
        insertnode(head, tail, p);
        printnode(p);
    }
    head->prev = NULL;
    tail->next = NULL;
    return head;
}


srtnode *genode(FILE *fp, char *line, srtnode *p)
{
    p = malloc(sizeof(srtnode));
    fgets(p->stime, 13, fp);
    fgets(line, 6, fp);
    fgets(p->etime, 13, fp);
    fgets(line, 100, fp);
    fgets(p->content, 100, fp);
    fgets(line, 100, fp);
    while(0 != strcmp(line, "\r\n"))
    {
        strcat(p->content, line);
        if(NULL == fgets(line, 100, fp))
        {
            break;
        }
    }
    return p;
}

void insertnode(srtnode *head, srtnode *tail, srtnode *p)
{
    srtnode *q, *t;
    if(strcmp(p->stime, tail->stime) >= 0)
    {
        tail->next = p;
        p->prev    = tail;
        tail       = p;
        return;
    }
    else if(strcmp(p->stime, head->stime) <= 0)
    {
        p->next    = head;
        head->prev = p;
        head       = p;
        return;
    }
    if(strcmp(p->stime, head->stime) <= -strcmp(p->stime, tail->stime))
    {
        q = head->next;
        while(strcmp(p->stime, q->stime) > 0)
        {
            q = q->next;
        }
        t = q->prev;
    }
    else
    {
        q = tail->prev;
        while(strcmp(p->stime, q->stime) < 0)
        {
            q = q->prev;
        }
        t = q;
        q = t->next;
    }
    t->next = p;
    p->prev = t;
    p->next = q;
    q->prev = p;
}

void printnode(srtnode *p)
{
    printf("%s --> %s", p->stime, p->etime);
    printf("%s", p->content);
}
