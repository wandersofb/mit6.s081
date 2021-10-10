#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define stdini 0
#define stdouti 1

int
main(int argc,char *argv[])
{
    int status = -1;
    char buf[MAXARG-argc+1];
    for (int i =0;read(stdini,buf+i,1) == 1;i++)
    {
        if (*(buf+i) != '\n')
            continue;
        else{
            *(buf+i) = '\0';
            i=0;
            if (fork() == 0)
            {
                char *arg[MAXARG];
                arg[0] = argv[1];
                int argcount;
                for (argcount = 1;argcount+1<argc;argcount++)
                    arg[argcount] = argv[argcount+1];
                arg[argcount] = buf;
                argcount++;
                exec(arg[0],arg);
                exit(0);
            }
            else
                wait(&status);
        }
    }
    exit(0);
}