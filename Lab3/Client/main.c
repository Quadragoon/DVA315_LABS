#include "../wrapper.h"
#include <unistd.h>
#include <signal.h>

mqd_t mailbox;
mqd_t serverMailbox;
planet_type* planet;
struct sigevent* signalEvent;

void MessageReceived();

int sendManyPlanets = 0;

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
    planet->next = NULL;
}

void SendManyPlanets()
{
    while (sendManyPlanets) {
        usleep(100000);
        strcpy(planet->name, "Random");
        AssignPlanetVariables(planet->name, (long)1e5%random(), random() % 800, random() % 600, 0, 0, 200000);
        if (MQwrite(&serverMailbox, planet))
            printf("Write success!\n");
        else {
            printf("Write failed!\n");
        }
    }
}

int main()
{
    srandom(time(NULL));

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

    char* planetSign;
    planetSign = malloc(sizeof(char)*10);

    do {
        strcpy(planet->name, "BLANK");

        //get input then create planet based on input
        if (fgets(planetSign, 10, stdin) == NULL) {
            printf("ERROR: input error");
        }

        if (strcmp(planetSign, "s\n") == 0) {
            strcpy(planet->name, "Sun");
            AssignPlanetVariables(planet->name, 1e8, 300, 300, 0, 0, 200000);
        } else if (strcmp(planetSign, "p\n") == 0) {
            strcpy(planet->name, "Planet");
            AssignPlanetVariables(planet->name, 1000, 200, 300, 0, 0.008, 200000);
        } else if (strcmp(planetSign, "r\n") == 0) {
            strcpy(planet->name, "BLANK");
            sendManyPlanets = !sendManyPlanets;
            if (sendManyPlanets) {
                pthread_t manyPlanetsThread;
                pthread_create(&manyPlanetsThread, NULL, (void*)SendManyPlanets, NULL);
            }
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
    mq_close(mailbox);

    return 0;
}

void MessageReceived() {
    struct mq_attr mqAttr;
    do {
        printf("Signal success!\n");
        MQread(&mailbox, (void**) &planet);
        printf("Planet %s has tragically passed away...\n", planet->name);
        mq_getattr(mailbox, &mqAttr);
    } while (mqAttr.mq_curmsgs > 0);

    mq_notify(mailbox, signalEvent);
    pthread_exit(NULL);
}