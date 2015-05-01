#include "decode.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dmedia/dmedia.h>


  char jpegData[262144];
  char outData[262144];
void main() {

  unsigned long long before , after;  
  jDecoder* dec;
  int fd;
  int size,i;
  int status;
  

  fd = open("jout",O_RDONLY);
  if(fd <0) {
    perror("opening file");
    exit(1);
  }
  size = read(fd,jpegData,262144);
  dec = initDecoder();
  dmGetUST(&before);
  for(i = 0; i < 100; i++) {
    status = decodeFrame(dec,jpegData,size,outData);
  }
  dmGetUST(&after);  
  printf("%lld nanoseconds\n",(after - before)/i);
  close(fd);
  
  fd = open("out.pm",O_WRONLY | O_CREAT,0666);
  write(fd,outData,262144);
  close(fd);
  

}
