#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFERSIZE 200

int main( void )
{
int fp = 0 ;
char str[BUFFERSIZE];
//char str1[] = "hello,mydevice";  /*test*/

fp = open( "/dev/mydev", O_RDWR,S_IRUSR|S_IWUSR );
if (fp<0)
{
	printf("Open device failed\n");
	return -1;
}

//write(fp,str1,sizeof(str1));			 /*test*/
write( fp, "Hello, my devices", strlen("Hello, my devices") );
lseek( fp, 0, 0 );
read( fp, str, BUFFERSIZE );
printf("Read content: %s\n", str );
close(fp);
return 0;
}
