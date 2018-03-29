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

    if((fd = open ("/dev/em-device", O_RDWR)) == -1)
    {
        printf("Error with open() on /dev/em-device.  Make sure it exists.\n");
        exit(EXIT_FAILURE);
    }
    
    data.inWord = 2;
    error = devctl(fd, RM_EM_CTL_CODE_ADD_FAX, &data, sizeof(data), NULL);
    if (error != EOK)
    {
    	printf("Error: %s\n", strerror(error));
        exit(EXIT_FAILURE);
    }

    data.inWord = 3;
    error = devctl(fd, RM_EM_CTL_CODE_ADD_VOICE1, &data, sizeof(data), NULL);
    if (error != EOK)
    {
      	printf("Error: %s\n", strerror(error));
        exit(EXIT_FAILURE);
    }

    data.inWord = 4;
    error = devctl(fd, RM_EM_CTL_CODE_ADD_VOICE2, &data, sizeof(data), NULL);
    if (error != EOK)
    {
       	printf("Error: %s\n", strerror(error));
        exit(EXIT_FAILURE);
    }

    data.inWord = 5;
    error = devctl(fd, RM_EM_CTL_CODE_ADD_DIALUP, &data, sizeof(data), NULL);
    if (error != EOK)
    {
        printf("Error: %s\n", strerror(error));
        exit(EXIT_FAILURE);
    }

    data.inWord = 6;
    error = devctl(fd, RM_EM_CTL_CODE_CLEAR, &data, sizeof(data), NULL);
    if (error != EOK)
    {
        printf("Error: %s\n", strerror(error));
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < OUT_WORDS_COUNT; ++i)
    {
    	printf(">> [%d] : %x\n", i + 1, data.outData[i]);
    }

    return (0);
}
