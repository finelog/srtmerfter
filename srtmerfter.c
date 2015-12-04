#include <stdio.h>
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

int   errno = 0;
char *errmsg[] = {
/* 0 */ "OK",
/* 1 */ "Open File Error, Please Check If File Exist.",
/* 2 */ "File Format Error, Can't Process Mac File Format.",
/* 3 */ "Write Srt File Error, Can't Write File To Disk.",
};

//function in main
int readargs(int argc, char **argv);
void perrror(char *msg);
int shiftsrt(char *time, char *file);
int mergesrt(char *firstsrt, char *secondsrt);

//function used commonly
char *detecteol(FILE *fp);
srtnode *loadsrt(FILE *fp, srtnode *head, char *EOL);
srtnode *genode(FILE *fp, char *line, srtnode *p, char *EOL);
srtnode *insertnode(srtnode *head, srtnode *p);
void freesrt(srtnode *head);
void node2srt(srtnode *head, FILE *fp, char *EOL);

//funtion used by shift srt
void tplus(char *srctime, char *shiftime);

//function used by merge srt
srtnode *mergelist(srtnode *head, srtnode *head2);

//void printsrt(srtnode *p);
//void printnode(srtnode *p);

int main(int argc, char **argv)
{
    int i = readargs(argc, argv);
    switch(i)
    {
        case 0:
            printf("%s", "Usage: srtmerfter [options] file...\nOptions:\n"
                         "\tfile1 file2 ... \n"
                         "\t-s +/-01,500 file1 file2 ...\n"
                         "\t-m file1 file2\n"
                         "\t-h display this message.\n");
            break;
        case 1:
            for(i; i<argc; i++)
                shiftsrt("0", argv[i]);
            break;
        case 3:
            for(i; i<argc; i++)
                shiftsrt(argv[2], argv[i]);
            break;
        case 2:
            mergesrt(argv[3], argv[4]);
            break;
        defalt: break;
    }

    return 0;
}

int readargs(int argc, char **argv)
{
    switch(argc)
    {
        case 1:
            return 0;
        case 2:
            if(strcmp("-h", argv[1]) == 0)
                return 0;
            else if('-' == argv[1][0])
                return 0;
            else
                return 1;
        default:
            if(strcmp("-s", argv[1]) != 0)
                return 3;
            else if(strcmp("-m", argv[1]) != 0)
                return 2;
            else if('-' == argv[1][0])
                return 0;
            else
                return 1;
    }
}

void perrror(char *msg)
{
    printf("%s:%s\n", msg, errmsg[errno]);
}

int shiftsrt(char *time, char *file)
{
    FILE    *fp;
    srtnode *head, *p;
    char    *EOL = "\r\n";
    errno = 0;

    if(NULL == (fp = fopen(file, "r")))
    {
        errno = 1;
        perror(file);
        return 1;
    }
    EOL = detecteol(fp);
    if(NULL == EOL)
    {
        errno = 2;
        perror(file);
        return 2;
    }
    head = loadsrt(fp, head, EOL);
    fclose(fp);
    if(NULL == (fp = fopen(file, "w")))
    {
        errno = 3;
        perror(file);
        return 3;
    }
    p = head;
    while(p)
    {
        tplus(p->stime, time);
        tplus(p->etime, time);
        p = p->next;
    }
    node2srt(head, fp, EOL);
    freesrt(head);
    return errno;
}

int mergesrt(char *firstsrt, char *secondsrt)
{
    FILE    *fp, *fp2;
    srtnode *head, *head2;
    int  len;
    char *EOL  = "\r\n";
    char *EOL2 = "\r\n";
    errno = 0;

    if(NULL == (fp = fopen(firstsrt, "r")))
    {
        errno = 1;
        perror(firstsrt);
        return 1;
    }
    else if(NULL == (fp2 = fopen(secondsrt, "r")))
    {
        errno = 1;
        perror(secondsrt);
        return 1;
    }
    EOL  = detecteol(fp);
    EOL2 = detecteol(fp2);
    if(NULL == EOL)
    {
        errno = 2;
        perror(firstsrt);
        return 2;
    }
    else if(NULL == EOL2)
    {
        errno = 2;
        perror(secondsrt);
        return 2;
    }

    head  = loadsrt(fp,  head,  EOL);
    head2 = loadsrt(fp2, head2, EOL2);
    fclose(fp);
    fclose(fp2);

    len = strlen(firstsrt);
    firstsrt[len-3] = 'm';
    firstsrt[len-2] = 'e';
    firstsrt[len-1] = 'r';
    strcat(firstsrt, "ged.srt");
    if(NULL == (fp = fopen(firstsrt, "w")))
    {
        errno = 3;
        perror(firstsrt);
        return 3;
    }
    head = mergelist(head, head2);
    node2srt(head, fp, EOL);
    freesrt(head);
    return errno;
}

char *detecteol(FILE *fp)
{
    int c;
    do{
        c = fgetc(fp);
    }while(c != '\r' && c != '\n');
    if(c == '\r')
    {
        c = fgetc(fp);
        if(c == '\n')
            c = 'd';
        else
            c = 'm';
    }
    else if(c == '\n')
        c = 'u';
    else
        c = 'm';
    rewind(fp);
    switch(c)
    {
        case 'd': return "\r\n";
        case 'u': return "\n";
        default : return NULL;
    }
}

srtnode *loadsrt(FILE *fp, srtnode *head, char *EOL)
{
    char line[103];
    fgets(line, 100, fp);
    head = genode(fp, line, head, EOL);
    head = insertnode(head, head);
    while(NULL != fgets(line, 100, fp))
    {
        if(0 == strcmp(line, EOL))
        {
            continue;
        }
        srtnode *p;
        p    = genode(fp, line, p, EOL);
        head = insertnode(head, p);
    }
    return head;
}

srtnode *genode(FILE *fp, char *line, srtnode *p, char *EOL)
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
    while(0 != strcmp(line, EOL))
    {
        length += strlen(line);
        if(length < sizeof(p->content))
        {
            strcat(p->content, line);
            if(NULL == fgets(line, 100, fp))
                break;
        }
        else
        {
            strcat(p->content, EOL);
            do{
                if(NULL == fgets(line, 100, fp))
                {
                   return p;
                }
            }while(0 != strcmp(line, EOL));
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
                        srctime[i]   -= 10;
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
                        srctime[i]   -= 10;
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
                        int j=0;
                        for(j=0; j<12; j++)
                        {
                            if(srctime[j]==':' || srctime[j]==',')
                                continue;
                            srctime[j] = '0';
                        }
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

srtnode *mergelist(srtnode *head, srtnode *head2)
{
    srtnode *temp, *stay, *move, *head0;
    move  = strcmp(head->stime, head2->stime) < 0 ? head : head2 ;
    stay  = move != head ? head : head2 ;
    head0 = move;
    while(move != NULL)
    {
        if(strcmp(move->stime, stay->stime) <= 0)
        {
            if(move->next == NULL)
            {
                break;
            }
            move = move->next;
            continue;
        }
        temp             = stay;
        stay             = stay->next;
        temp->prev       = move->prev;
        move->prev->next = temp;
        if(stay == NULL)
        {
            move->prev = temp;
            temp->next = move;
            break;
        }
        else if(strcmp(move->stime, stay->stime) > 0 )
        {
            temp = move;
            move = stay;
            stay = temp;
        }
        else
        {
            move->prev = temp;
            temp->next = move;
            stay->prev = NULL;
        }
    }
    if(stay != NULL)
    {
        stay->prev = move;
        move->next = stay;
    }
    return head0;
}

//another way to merge srt files
//this is the more quick way,probably
/*
srtnode *srtmerge(srtnode *head, srtnode *head2)
{
    srtnode *temp, *line2, *line1, *head0;
    line1  = strcmp(head->stime, head2->stime) < 0 ? head : head2 ;
    line2  = line1 != head ? head : head2 ;
    head0 = line1;
    while(line1 != NULL)
    {
        if(strcmp(line1->stime, line2->stime) <= 0)
        {
            if(line1->next == NULL)
            {
                break;
            }
            line1 = line1->next;
            continue;
        }
        temp = line2;
        while(line2->next != NULL)
        {
            line2 = line2->next;
            if(strcmp(line2->stime, line1->stime) > 0)
                break;
        }
        temp->prev        = line1->prev;
        line1->prev->next = temp;
        if(temp == line2)
        {
            line1->prev = temp;
            temp->next  = line1;
            line2       = NULL;
            break;
        }
        line1->prev       = line2->prev;
        line2->prev->next = line1;
    }
    if(line2 != NULL)
    {
        line2->prev = line1;
        line1->next = line2;
    }
    return head0;
}
*/

void node2srt(srtnode *head, FILE *fp, char *EOL)
{
    int i = 1;
    while(head != NULL)
    {
        fprintf(fp, "%d%s%s --> %s%s%s%s", i++, EOL, head->stime, head->etime, EOL, head->content, EOL);
        head = head->next;
    }
    fclose(fp);
}

void freesrt(srtnode *head)
{
    srtnode *p;
    p = head->next;
    while(p)
    {
        free(head);
        head = NULL;
        head = p;
        p    = p->next;
    }
    free(head);
    head = NULL;
}
