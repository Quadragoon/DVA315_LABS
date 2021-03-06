#include "wrapper.h"
#include <zconf.h>
#include <semaphore.h>

#define MAX_SIZE 1024
#define QUEUE_NAME "/Mailbox2"

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

#ifndef ASSIGNMENT_B
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
#endif

#ifdef ASSIGNMENT_B
    char* textString = malloc(sizeof(char)*MAX_SIZE);
    planet_type planetType;
    int c;
    while(1) {
        printf("Enter name: ");
        sem_wait(&mutex);
        fgets(textString, 21, stdin);
        if (textString[strlen(textString) - 1] != '\n')
            while ((c = getchar()) != '\n' && c != EOF) { }
        textString[strlen(textString) - 1] = '\0';
        strcpy(planetType.name, textString);
        MQwrite(&mailbox, &planetType);
        sem_post(&mutex);
        if (strcmp(textString, "END") == 0)
            pthread_exit(NULL);
        sleep(1);
    }
#endif
}

void* readText() {
    mqd_t mailbox;

    int success = 0;
    while (!success) {
        success = MQcreate(&mailbox, QUEUE_NAME);
    }

#ifndef ASSIGNMENT_B
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
#endif

#ifdef ASSIGNMENT_B
    planet_type receivedPlanet;
    planet_type* planetPointer = &receivedPlanet;

    while (1) {
        sleep(1);
        sem_wait(&mutex);
        if (!MQread(&mailbox, (void**)&planetPointer)) {
            if (errno != 11)
                printf("Read failed: error %d, %s\n", errno, strerror(errno));
        }
        else {
            printf("Received data, name: %s\n", (char *) receivedPlanet.name);
        }
        sem_post(&mutex);
        if (strcmp(receivedPlanet.name, "END") == 0) {
            mq_unlink(QUEUE_NAME);
            pthread_exit(NULL);
        }
    }
#endif
}