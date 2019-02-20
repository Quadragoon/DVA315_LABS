#include "../wrapper.h"
#include <unistd.h>
#include <signal.h>

mqd_t mailbox;
planet_type* planet;
struct sigevent* signalEvent;

void MessageReceived();

void AssignPlanetVariables(const char* name, double mass, double sx, double sy, double vx, double vy, int life)
{
    int pid = getpid();
    strcpy(planet->name, name);
    planet->pid = pid;
    planet->mass = mass;
    planet->sx = sx;
    planet->sy = sy;
    planet->vx = vx;
    planet->vy = vy;
    planet->life = life;
}

int main()
{
    srandom(time(NULL));

    mqd_t serverMailbox;
    while (!MQconnect(&serverMailbox, "/MQ_Planets_MAIN")) {
        printf("Connect failed. Retrying...\n");
        sleep(1);
    }
    printf("Connect success!\n");

    pid_t pid = getpid();
    char* mailboxName = malloc(sizeof(char) * 64);
    snprintf(mailboxName, 64, "/MQ_Planets_%d", pid);
    while (!MQcreate(&mailbox, mailboxName)) {
        sleep(1);
    }
    printf("New mailbox created successfully!\n");
    free(mailboxName);

    signalEvent = malloc(sizeof(struct sigevent));
    signalEvent->sigev_notify = SIGEV_THREAD;
    signalEvent->sigev_notify_function = &MessageReceived;
    mq_notify(mailbox, signalEvent);

    planet = malloc(sizeof(planet_type));
    if (planet == NULL) {
        printf("ERROR: Planet malloc failed!\n");
        return -1;
    }
/*
    AssignPlanetVariables("CONNECT", 0, 0, 0, 0, 0, 0);

    if (MQwrite(&serverMailbox, planet))
        printf("Connect success!\n");
    else {
        printf("Connect failed!\n");
        return 0;
    }

    sleep(1);
*/
    char* planetSign;
    planetSign = malloc(sizeof(char)*10);

    do {
        strcpy(planet->name, "BLANK");

        //get input then create planet based on input
        if (fgets(planetSign, 10, stdin) == NULL) {
            printf("ERROR: input error");
        }

        /*
        int i;
        for (i = 0; i<5; i++) {
            planet->name[i] = 'a' + random() % 25;
        }
        */
        if (strcmp(planetSign, "s\n") == 0) {
            strcpy(planet->name, "Sun");
            AssignPlanetVariables(planet->name, 100000000, 300, 300, 0, 0, 20);
        } else if (strcmp(planetSign, "p\n") == 0) {
            strcpy(planet->name, "Planet");
            AssignPlanetVariables(planet->name, 1000, 200, 300, 0, 0.008, 20);
        }
        if (strcmp(planet->name, "BLANK") != 0) {
            if (MQwrite(&serverMailbox, planet))
                printf("Write success!\n");
            else {
                printf("Write failed!\n");
                return 0;
            }
        }
        //sleep(1);
    }while(strcmp(planetSign, "x\n") != 0);

    /*
    for (int n = 0; n < 4; n++) {
        for (i = 0; i < 5; i++) {
            planet->name[i] = 'a' + random() % 25;
        }
        planet->name[i] = '\0';
        AssignPlanetVariables(planet->name, 1000, 200, 300, 0, 0.008, 20);

        if (MQwrite(&serverMailbox, planet))
            printf("Write success!\n");
        else {
            printf("Write failed!\n");
            return 0;
        }

        sleep(1);
    }
*/
    mq_close(mailbox);

    return 0;
}

void MessageReceived() {
    mq_notify(mailbox, signalEvent);

    printf("Signal success!\n");
    MQread(&mailbox, (void **) &planet);
    printf("Planet %s has tragically passed away...\n", planet->name);
    pthread_exit(NULL);
}