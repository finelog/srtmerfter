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
srtnode *srtmerge(srtnode *head, srtnode *head2);

//void printsrt(srtnode *p);
//void printnode(srtnode *p);

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
        return 1;
    }
    else if(2 == argc && 0 == strcmp(argv[1], "-h"))
    {
        printf("%s", help);
        return 1;
    }
    else if(0 == strcmp(argv[1], "-s"))
    {
        srtnode *p;
        char *EOL  = "\r\n";
        if(NULL == (fp = fopen(argv[3], "r")))
        {
            printf("Can't open file:%s\n", argv[3]);
            return 2;
        }
        EOL = detecteol(fp);
        if(NULL == EOL)
        {
            printf("Cannot process Mac file format,"
                   "please convert it to Unix or Dos format.\n");
            return 4;
        }
        head = loadsrt(fp, head, EOL);
        fclose(fp);
        if(NULL == (fp = fopen(argv[3], "w")))
        {
            printf("Can't open file to write:%s\n", argv[3]);
            return 3;
        }
        p = head;
        while(p != NULL)
        {
            tplus(p->stime, argv[2]);
            tplus(p->etime, argv[2]);
            p = p->next;
        }
        node2srt(head, fp, EOL);
        fclose(fp);
        freesrt(head);
        return 0;
    }
    else if(0 == strcmp(argv[1], "-m"))
    {
        FILE *fp2;
        int  len;
        srtnode *head2;
        char *EOL  = "\r\n";
        char *EOL2 = "\r\n";
        if(NULL == (fp = fopen(argv[2], "r")))
        {
            printf("Can't open file:%s\n", argv[2]);
            return 2;
        }
        else if(NULL == (fp2 = fopen(argv[3], "r")))
        {
            printf("Can't open file:%s\n", argv[3]);
            return 2;
        }

        EOL  = detecteol(fp);
        EOL2 = detecteol(fp2);
        if(NULL == EOL)
        {
            printf("Cannot process file1 Mac file format,"
                   "please convert it to Unix or Dos format.\n");
            return 4;
        }
        else if(NULL == EOL2)
        {
            printf("Cannot process file2 Mac file format,"
                   "please convert it to Unix or Dos format.\n");
            return 4;
        }
        else if(0 != strcmp(EOL, EOL2))
        {
            printf("Cannot process different file formats at one time,"
                   "please convert it to the same Unix or Dos format.\n");
            return 4;
        }

        head  = loadsrt(fp,  head, EOL);
        head2 = loadsrt(fp2, head2, EOL);

        fclose(fp);
        fclose(fp2);

        len = strlen(argv[2]);
        argv[2][len-3] = 'm';
        argv[2][len-2] = 'e';
        argv[2][len-1] = 'r';
        strcat(argv[2], "ged.srt");
        if(NULL == (fp = fopen(argv[2], "w")))
        {
            printf("Can't open file to write:%s\n", argv[2]);
            return 3;
        }
        head = srtmerge(head, head2);
        node2srt(head, fp, EOL);
        fclose(fp);
        freesrt(head);
    }
    else
    {
        printf("%s", help);
    }
    return 0;
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
                        for(j=0; i<12; j++)
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

//This is not a have-to, another way is fprintf the smaller one
srtnode *srtmerge(srtnode *head, srtnode *head2)
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
//this is the more quick way,maybe
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
