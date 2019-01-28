#include "wrapper.h"
#include <fcntl.h>
#include <sys/stat.h>

#define MAX_SIZE 1024

int MQcreate (mqd_t * mq, char * name)
{
    /* Creates a mailslot with the specified name. */
    /* Uses mq as reference pointer, so that you can reach the handle from anywhere */
    /* Should be able to handle a messages of any size */
    /* Should return 1 on success and 0 on fail*/
    mqd_t mailslot;
    mailslot = mq_open(name, O_RDWR|O_CREAT|O_EXCL, 644, NULL);
    if (mailslot == -1)
        return 0;
    else {
        mq = &mailslot;
        return 1;
    }
}

int MQconnect (mqd_t * mq, char * name)
{
    /*  Uses mq as reference pointer, so that you can 	reach the handle from anywhere */
    /* Connects to an existing mailslot for writing */
    /* Should return 1 on success and 0 on fail*/

    mqd_t mailslot;
    mailslot = mq_open(name, O_RDWR);
    if (mailslot == -1)
        return 0;
    else {
        mq = &mailslot;
        return 1;
    }
}

int MQread (mqd_t * mq, void ** refBuffer)
{
    /*  Uses mq as reference pointer, so that you can 		reach the handle from anywhere */
    /* Read a msg from a mailslot, return nr */
    /* of successful bytes read              */

    int retval = mq_receive(*mq, (char*)*refBuffer, 8192, NULL);
    if (retval > 0)
        return retval;
    else
        return 0;
}

int MQwrite (mqd_t * mq, void * sendBuffer)
{
    /*  Uses mq as reference pointer, so that you can 	     reach the handle from anywhere */
    /* Write a msg to a mailslot, return nr */
    /* of successful bytes written         */
    int length = 0;
    char* message = (char*)sendBuffer;
    while(message[length] != '\0') {
        length++;
    }

    int retval;
    //mq_send(mq, sendBuffer, sizeof(char) * length, )
    retval = mq_send(*mq, sendBuffer, sizeof((char*)sendBuffer), 1);
    if (retval == 0)
        return sizeof((char*)sendBuffer);
    else
        return 0;
}

int MQclose(mqd_t * mq, char * name)
{
    /*  Uses mq as reference pointer, so that you can reach the handle from anywhere */
    /* close a mailslot, returning whatever the service call returns */
    /* Should return 1 on success and 0 on fail*/
    return 0;
}
int threadCreate (void * functionCall, int threadParam)
{
	/* Creates a thread running threadFunc */
	/* Should return 1 on success and 0 on fail*/
	return 0;
}


