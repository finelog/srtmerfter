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

srtnode *loadsrt(FILE *fp, srtnode *head);
srtnode *genode(FILE *fp, char *line, srtnode *p);
srtnode *insertnode(srtnode *head, srtnode *p);

void tplus(char *srctime, char *shiftime);

srtnode *srtmerge(srtnode *head, srtnode *head2);
void node2srt(srtnode *head, FILE *fp);
void printnode(srtnode *p);

void freesrt(srtnode *head);

int main(int argc, char **argv)
{
    FILE *fp;
    srtnode *head;
    char *help = "Usage: srtmerfter [options] file...\nOptions:\n"
                 "\t-s +/-01,500 file\n"
                 "\t-m file1 file2\n"
                 "\t-h display this message.\n";
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
        srtnode *p;
        if(NULL == (fp = fopen(argv[3], "r")))
        {
            printf("Can't open file:%s\n", argv[3]);
            return -2;
        }
        head = loadsrt(fp, head);
        fclose(fp);
        int len = strlen(argv[3]);
        argv[3][len-3] = 'n';
        argv[3][len-2] = 'e';
        argv[3][len-1] = 'w';
        strcat(argv[3], ".srt");
        if(NULL == (fp = fopen(argv[3], "w")))
        {
            printf("Can't open file to write:%s\n", argv[3]);
            return -3;
        }
        p = head;
        while(p != NULL)
        {
            tplus(p->stime, argv[2]);
            tplus(p->etime, argv[2]);
            p = p->next;
        }
        node2srt(head, fp);
        fclose(fp);
        freesrt(head);
        return 0;
    }
    else if(0 == strcmp(argv[1], "-m"))
    {
        FILE *fp2;
        srtnode *head2;
        if(NULL == (fp = fopen(argv[3], "r")))
        {
            printf("Can't open file:%s\n", argv[3]);
            return -2;
        }
        else if(NULL == (fp2 = fopen(argv[4], "r")))
        {
            printf("Can't open file:%s\n", argv[4]);
            return -2;
        }
        head  = loadsrt(fp,  head);
        head2 = loadsrt(fp2, head2);
        fclose(fp);
        fclose(fp2);
        strcat(argv[3], ".merged.srt");
        if(NULL == (fp = fopen(argv[3], "w")))
        {
            printf("Can't open file to write:%s\n", argv[3]);
            return -3;
        }
        head = srtmerge(head, head2);
        node2srt(head, fp);
        fclose(fp);
    }
    else
    {
        printf("%s", help);
    }
    return 0;
}

srtnode *loadsrt(FILE *fp, srtnode *head)
{
    char line[103];
    fgets(line, 100, fp);
    head = genode(fp, line, head);
    head = insertnode(head, head);
    while(NULL != fgets(line, 100, fp))
    {
        if(0 == strcmp(line, "\r\n"))
        {
            continue;
        }
        srtnode *p;
        p    = genode(fp, line, p);
        head = insertnode(head, p);
    }
    return head;
}

srtnode *genode(FILE *fp, char *line, srtnode *p)
{
    int length;
    p = malloc(sizeof(srtnode));
    p->prev = NULL;
    p->next = NULL;
    fgets(p->stime, 13, fp);
    fgets(line, 6, fp);
    fgets(p->etime, 13, fp);
    fgets(line, 100, fp);
    fgets(p->content, 250, fp);
    if(NULL == fgets(line, 100, fp))
        return p;
    length = strlen(p->content);
    while(0 != strcmp(line, "\r\n"))
    {
        length += strlen(line);
        if(length < sizeof(p->content))
        {
            strcat(p->content, line);
            fgets(line, 100, fp);
        }
        else
        {
            strcat(p->content, "\r\n");
            do{
                if(NULL == fgets(line, 100, fp))
                {
                    break;
                }
            }while(0 != strcmp(line, "\r\n"));
        }
    }
    return p;
}

srtnode *insertnode(srtnode *head, srtnode *p)
{
    static srtnode *tail = NULL;
    srtnode *q, *t;
    if(p == head)
    {
        tail = head;
        return head;
    }
    else if(strcmp(p->stime, tail->stime) >= 0)
    {
        tail->next = p;
        p->prev    = tail;
        tail       = p;
        return head;
    }
    else if(strcmp(p->stime, head->stime) <= 0)
    {
        p->next    = head;
        head->prev = p;
        head       = p;
        return head;
    }
    else if(strcmp(p->stime, head->stime) <= -strcmp(p->stime, tail->stime))
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
    return head;
}

void tplus(char *srctime, char *shiftime)
{
    unsigned int ctime, stime;
    int i, len;
    len = strlen(shiftime);
    if(shiftime[0] == '+')
    {
        for(i=1; i<len; i++)
        {
            if(shiftime[len-i] == ',' || shiftime[len-i] == ':')
                continue;
            srctime[12-i] += shiftime[len-i] - '0';
        }
        for(i=11; i>=0; i--)
        {
            switch(i)
            {
                case 0:
                    if(srctime[i] > '9')
                    {
                        srctime[i] = '9';
                    }
                    break;
                case 1:
                case 4:
                case 7:
                case 10:
                case 11:
                    if(srctime[i] > '9')
                    {
                        srctime[i-1] += 1;
                        srctime[i]   += '0' - '9';
                    }
                    break;
                case 3:
                case 6:
                    if(srctime[i] > '5')
                    {
                        srctime[i-2] += 1;
                        srctime[i]    = '0';
                    }
                    break;
                case 9:
                    if(srctime[i] > '9')
                    {
                        srctime[i-2] += 1;
                        srctime[i]   += '0' - '9';
                    }
                    break;
                default:break;
            }
        }
    }
    else if(shiftime[0] == '-')
    {

        for(i=1; i<len; i++)
        {
            if(shiftime[len-i] == ',' || shiftime[len-i] == ':')
                continue;
            srctime[12-i] -= shiftime[len-i] - '0';
        }
        for(i=11; i>=0; i--)
        {
            switch(i)
            {
                case 0:
                    if(srctime[i] < '0')
                    {
                        srctime[i] = '0';
                    }
                    break;
                case 1:
                case 4:
                case 7:
                case 10:
                case 11:
                    if(srctime[i] < '0')
                    {
                        srctime[i-1] -= 1;
                        srctime[i]   += 10;
                    }
                    break;
                case 3:
                case 6:
                    if(srctime[i] < '0')
                    {
                        srctime[i-2] -= 1;
                        srctime[i]   += 6;
                    }
                    break;
                case 9:
                    if(srctime[i] < '0')
                    {
                        srctime[i-2] -= 1;
                        srctime[i]   += 10;
                    }
                    break;
                default:break;
            }
        }
    }
}

//This is not a have-to, another way is fprintf the bigger one
srtnode *srtmerge(srtnode *head, srtnode *head2)
{
}

void node2srt(srtnode *head, FILE *fp)
{
    int i = 1;
    while(head != NULL)
    {
        fprintf(fp, "%d\r\n%s --> %s\r\n%s\r\n", i++, head->stime, head->etime, head->content);
        head = head->next;
    }
}

void printnode(srtnode *p)
{
    printf("%s --> %s", p->stime, p->etime);
    printf("%s", p->content);
}

void freesrt(srtnode *head)
{
    srtnode *p;
    p = head->next;
    while(p != NULL)
    {
        free(p->prev);
        p = p->next;
    }
}
