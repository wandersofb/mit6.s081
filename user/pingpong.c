#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define READ 0
#define WRITE 1
int
main()
{
    int p1[2];
    int p2[2];
    char buf[1];
    pipe(p1);
    pipe(p2);

    if (fork() == 0) {
        close(p1[WRITE]);
        close(p2[READ]);
        read(p1[READ],buf,1);
        close(p1[READ]);
        printf("%d: received ping\n",getpid());
        write(p2[1],buf,1);
        close(p2[1]); 

    }
    else{
        close(p1[0]);
        close(p2[1]);        
        write(p1[1],buf,1);
        close(p1[1]);
        read(p2[0],buf,1);
        close(p2[0]); 
        printf("%d: received pong\n",getpid());
    }
    exit(0);
}