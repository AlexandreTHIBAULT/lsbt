#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h> 
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <regex.h>   
#include <sys/ioctl.h>
#include <math.h>
#include <wchar.h>
#include <locale.h>
#include <limits.h>
#include <stdio.h>

#define DEFCOLORS "def\n0\n\ndir\n34;1\n\ntw\n30;42\n\now\n34;42\n\nst\n37;44\n\nblk\n33;40\n\nchara\n33;40\n\nfifo\n33;40\n\nsocket\n35;1\n\nlnk\n36;1\n\nlnk_broken\n40;31;01\n\nexec\n32;1\n\nmedia\n35;1\n\ncompressed\n01;31\n\naudio\n00;36"

typedef struct lsbt_colors
{
    char def[10];
    char dir[10];
    char tw[10];
    char ow[10];
    char st[10];
    char blk[10];
    char chara[10];
    char fifo[10];
    char socket[10];
    char lnk[10];
    char lnk_broken[10];
    char exec[10];
    char media[10];
    char compressed[10];
    char audio[10];
} lsbt_colors;

ssize_t getlinenum(char ** lineptr, size_t * n, FILE * stream, int * linenb){
    ssize_t read = 0;
    int empty = 1;

    while(read<=1 && read!=-1){
        read = getline(lineptr, n, stream);
        (*linenb)++;
    }
    (*lineptr)[strcspn((*lineptr), "\n")] = 0;
    return read;
}

void create_default_config(){
    FILE * fp;
    char *file;
    char *fileName = "/.config/lsbt/lsbt_colors";
    file = malloc(strlen(getenv("HOME")) + strlen(fileName) + 2);
    strcpy(file, getenv("HOME"));
    strcat(file, fileName);
    fp = fopen(file, "w");
    if (fp == NULL){
        perror(file);
        exit(EXIT_FAILURE);
    }
    fprintf(fp, DEFCOLORS);
    fclose(fp);
}

lsbt_colors get_colors(){
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int l = 0;

    lsbt_colors colors;

    char *file;
    char *fileName = "/.config/lsbt/lsbt_colors";
    file = malloc(strlen(getenv("HOME")) + strlen(fileName) + 2);
    strcpy(file, getenv("HOME"));
    strcat(file, fileName);

    fp = fopen(file, "r");
    if (fp == NULL){
        create_default_config();
        fp = fopen(file, "r");
        if (fp == NULL){
            perror(file);
            exit(EXIT_FAILURE);
        }
    }
    
    while ((read = getlinenum(&line, &len, fp, &l)) != -1) {

        if(strcmp(line, "def")==0){
            read = getlinenum(&line, &len, fp, &l);
            sprintf(colors.def, "%s", line);

        }
        else if(strcmp(line, "dir")==0){
            read = getlinenum(&line, &len, fp, &l);
            sprintf(colors.dir, "%s", line);
        }
        else if(strcmp(line, "tw")==0){
            read = getlinenum(&line, &len, fp, &l);
            sprintf(colors.tw, "%s", line);
        }
        else if(strcmp(line, "ow")==0){
            read = getlinenum(&line, &len, fp, &l);
            sprintf(colors.ow, "%s", line);
        }
        else if(strcmp(line, "st")==0){
            read = getlinenum(&line, &len, fp, &l);
            sprintf(colors.st, "%s", line);
        }
        else if(strcmp(line, "blk")==0){
            read = getlinenum(&line, &len, fp, &l);
            sprintf(colors.blk, "%s", line);
        }
        else if(strcmp(line, "chara")==0){
            read = getlinenum(&line, &len, fp, &l);
            sprintf(colors.chara, "%s", line);
        }
        else if(strcmp(line, "fifo")==0){
            read = getlinenum(&line, &len, fp, &l);
            sprintf(colors.fifo, "%s", line);
        }
        else if(strcmp(line, "socket")==0){
            read = getlinenum(&line, &len, fp, &l);
            sprintf(colors.socket, "%s", line);
        }
        else if(strcmp(line, "lnk")==0){
            read = getlinenum(&line, &len, fp, &l);
            sprintf(colors.lnk, "%s", line);
        }
        else if(strcmp(line, "lnk_broken")==0){
            read = getlinenum(&line, &len, fp, &l);
            sprintf(colors.lnk_broken, "%s", line);
        }
        else if(strcmp(line, "exec")==0){
            read = getlinenum(&line, &len, fp, &l);
            sprintf(colors.exec, "%s", line);
        }
        else if(strcmp(line, "media")==0){
            read = getlinenum(&line, &len, fp, &l);
            sprintf(colors.media, "%s", line);
        }
        else if(strcmp(line, "compressed")==0){
            read = getlinenum(&line, &len, fp, &l);
            sprintf(colors.compressed, "%s", line);
        }
        else if(strcmp(line, "audio")==0){
            read = getlinenum(&line, &len, fp, &l);
            sprintf(colors.audio, "%s", line);
        }
        
    }

    return colors;
}
