#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), '\0', DIRSIZ-strlen(p));
  return buf;
}

void 
find(char *dir,char *fil)
{
    char buf[512],*p;
    char *n = "\n";
    int fd;
    struct stat st;
    struct dirent de;
    char *dot = ".\0";
    char *dotd = "..\0";

    if ((fd = open(dir, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", dir);
    return;
    }

    if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", dir);
    close(fd);
    return;
    }

    switch (st.type){
    case T_FILE:
        fprintf(2, "find cannot find a file in file\n");
        close(fd);
        return;
    case T_DIR:
        if(strlen(dir) + 1 + DIRSIZ + 1 > sizeof buf){
        printf("ls: path too long\n");
        break;
        }
        strcpy(buf, dir);
        p = buf+strlen(buf);
        *p++ = '/';
        //current dir 
        while(read(fd, &de, sizeof(de)) == sizeof(de)){
            if(de.inum == 0)
                break;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if(stat(buf, &st) < 0){
                printf("ls: cannot stat %s\n", buf);
                continue;
            }

            if (st.type == T_FILE)
            {
                if (strcmp(fmtname(buf),fil) == 0)
                {
                    
                    write(1,buf,strlen(buf));
                    write(1,n,1);
                }
            }
            else if (st.type == T_DIR)
            {
                if (strcmp(fmtname(buf),dot) !=0 && strcmp(fmtname(buf),dotd) !=0)
                    find(buf,fil);
            }
        }
        break;
    }
    close(fd);
}

int
main(int argc, char *argv[])
{
    if (argc < 3 || argc > 3)
    {
        printf("argument false, please try again.");
        exit(0);
    }
    else
    {
        find(argv[1],argv[2]);
        exit(0);
    }
}