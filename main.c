#include "wrapper.h"
#include <zconf.h>
#include <semaphore.h>

#define MAX_SIZE 1024
#define QUEUE_NAME "/Mailbox"

void* helloWorld(int* count);
void* helloMoon(int* count);
void* readText();

sem_t mutex;

int main(int ac, char ** argv) {
    pthread_t readThread;

    pthread_create(&readThread, NULL, readText, NULL);

    mqd_t mailbox;
    int connected = 0;
    while (!connected) {
        connected = MQconnect(&mailbox, QUEUE_NAME);
        usleep(50);
    }

    sem_init(&mutex, 0, 1);

    char* textString = malloc(sizeof(char)*MAX_SIZE);
    while(1) {
        printf("Enter string: ");
        sem_wait(&mutex);
        fgets(textString, MAX_SIZE, stdin);
        textString[strlen(textString) - 1] = '\0';
        MQwrite(&mailbox, textString);
        sem_post(&mutex);
        if (strcmp(textString, "END") == 0)
            pthread_exit(NULL);
        sleep(1);
    }
}

void* readText() {
    mqd_t mailbox;

    int success = 0;
    while (!success) {
        success = MQcreate(&mailbox, QUEUE_NAME);
    }

    void* textString = malloc(MAX_SIZE);

    while (1) {
        sleep(1);
        sem_wait(&mutex);
        if (!MQread(&mailbox, &textString))
            printf("Read failed: error %d, %s\n", errno, strerror(errno));
        else {
            printf("%s\n", (char *) textString);
        }
        sem_post(&mutex);
        if (strcmp(textString, "END") == 0) {
            mq_unlink(QUEUE_NAME);
            pthread_exit(NULL);
        }
    }
}