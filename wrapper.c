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

    struct mq_attr attributes;

    attributes.mq_flags = 0;
    attributes.mq_maxmsg = 10;
    attributes.mq_msgsize = MAX_SIZE;
    attributes.mq_curmsgs = 0;

    mailslot = mq_open(name, O_CREAT | O_RDWR | O_NONBLOCK, 0777, &attributes);
    if (mailslot == -1) {
        printf("ERROR in MQcreate: ERROR CODE %d: %s\n", errno, strerror(errno));
        return 0;
    }
    else {
        *mq = mailslot;
        return 1;
    }
}

int MQconnect (mqd_t * mq, char * name)
{
    /* Uses mq as reference pointer, so that you can reach the handle from anywhere */
    /* Connects to an existing mailslot for writing */
    /* Should return 1 on success and 0 on fail*/

    mqd_t mailslot;
    mailslot = mq_open(name, O_RDWR | O_NONBLOCK);
    if (mailslot == -1)
        return 0;
    else {
        *mq = mailslot;
        return 1;
    }
}

int MQread (mqd_t * mq, void ** refBuffer)
{
    /*  Uses mq as reference pointer, so that you can 		reach the handle from anywhere */
    /* Read a msg from a mailslot, return nr */
    /* of successful bytes read            */

    ssize_t retval = mq_receive(*mq, *refBuffer, MAX_SIZE, NULL);
    if (retval >= 0) {
        //printf("MQread success, %d bytes read\n", retval);
        return (int)retval;
    }
    else
        return 0;
}

int MQwrite (mqd_t * mq, void * sendBuffer)
{
    /* Uses mq as reference pointer, so that you can reach the handle from anywhere */
    /* Write a msg to a mailslot, return nr */
    /* of successful bytes written */

#ifndef ASSIGNMENT_B
    int length = 0;
    char* message = (char*)sendBuffer;
    while(message[length] != '\0') {
        length++;
    }
    length++;

    int retval;
    retval = mq_send(*mq, sendBuffer, (size_t)sizeof(char) * length, 0);
    if (retval == 0) {
        //printf("MQwrite success, %d bytes written\n", (int)sizeof(char) * length);
        return (int)sizeof(char) * length;
    }
    else
        return 0;
#endif

#ifdef ASSIGNMENT_B
    size_t msgSize = sizeof(planet_type);

    int retval;
    retval = mq_send(*mq, sendBuffer, msgSize, 0);
    if (retval == 0) {
        return (int)sizeof(planet_type);
    } else
        return 0;
#endif
}

int MQclose(mqd_t * mq, char * name)
{
    /*  Uses mq as reference pointer, so that you can reach the handle from anywhere */
    /* close a mailslot, returning whatever the service call returns */
    /* Should return 1 on success and 0 on fail*/
    int retval;
    retval = mq_close(*mq);
    if (retval == 0)
        return 1;
    else
        return 0;
}