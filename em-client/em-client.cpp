/* For "devctl()" */
#include <devctl.h>
#include <sys/dcmd_chr.h>
#include <sys/can_dcmd.h>

/* For "open()" */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* For Errors */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "..\dev.h"

struct can_devctl_error derror;

int main(void)
{
    int fd, error;
    ESDataExchangeStruct data;
    data.inWord = 15;

    if((fd = open ("/dev/em-device", O_RDWR)) == -1)
    {
        printf("Error with open() on /dev/em-device.  Make sure it exists.\n");
        exit(EXIT_FAILURE);
    }
    
    error = devctl(fd, RM_EM_CTL_CODE_ADD_FAX, &data, sizeof(data), NULL);
    if (error != EOK)
    {
    	printf("Error: %s\n", strerror(error));
        exit(EXIT_FAILURE);
    }

    printf("out %d\n", data.outData);

    return (0);
}
