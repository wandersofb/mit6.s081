#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define READ 0
#define WRITE 1
#define false 0
#define true 1

int isprime(int a)
{
    int x=0;
    for (int i=2;i<=(a/2);i++)
    {
        if (a%i ==0)
            x++;
    }
    return (x > 0) ? false : true;
}
int
main()
{
    int status = -1;
    for (int i=2;i<35;i++)
    {
        int buf[1];
        int p[2];
        pipe(p);
        
        if (fork() ==0)
        {
            
            close(p[READ]);
            if (isprime(i) == 1)
            {
                buf[0] = i;
                write(p[WRITE],buf,4);
                close(p[WRITE]);
            }
            else
            {
                buf[0] = 0;
                write(p[WRITE],buf,4);
                close(p[WRITE]);
            }
                
            exit(0);
        }
        else 
        {
            close(p[WRITE]);
            wait(&status);
            read(p[READ],buf,4);
            int x = *buf;
            if (isprime(x) == 1 && x != 0)
            {
                printf("prime %d\n",x);
            }
            close(p[READ]);
        }
    }
    exit(0);
}