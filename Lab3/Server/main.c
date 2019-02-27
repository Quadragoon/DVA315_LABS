#include <cairo.h>
#include <gtk/gtk.h>
#include "../wrapper.h"
#include <signal.h>
#include <math.h>

#define TWO_PI 6.283
#define MSLEEP(time) usleep(time*1000)
#define CAPITAL_G 6.67259e-11
#define DELTA_T 100
#define RADTODEG(degrees) ((degrees) * M_PI / 180)

static void do_drawing(cairo_t*, GtkWidget* widget);

GtkWidget* window;
GtkWidget* darea;

planet_type* planetList = NULL; // first node of a single-linked list of planets
int planetCount = 0; // number of planets currently alive and simulating

mqd_t mainBox;
pthread_t planetThreads[1];

struct sigevent* signalEvent;

pthread_mutex_t mutex;


// Appends the supplied planet to the end of the linked planet list.
// Returns a pointer to the new planet on success and NULL on failure.
void* AddExistingPlanet(planet_type* planetToAdd)
{
    if (planetList == NULL) {
        planetList = planetToAdd;
        planetCount++;
        return planetToAdd;
    } else {
        planet_type* lastPlanet = planetList;
        while (lastPlanet->next != NULL)
            lastPlanet = lastPlanet->next;

        lastPlanet->next = planetToAdd;
        planetCount++;
        return planetToAdd;
    }
}

void InformClient(planet_type* planetBeingRemoved)
{
    if (strcmp(planetBeingRemoved->name, "TEST") != 0) {
        mqd_t clientBox;
        char* boxName;
        boxName = malloc(sizeof(char) * 64);
        snprintf(boxName, 64, "/MQ_Planets_%d", planetBeingRemoved->pid);
        if (!MQconnect(&clientBox, boxName)) {
        }
        if (!MQwrite((&clientBox), planetBeingRemoved)) {
        }
    }
}

// Removes the planet with the supplied address from the linked planet list.
// Returns a pointer containing the removed planet's ->next value
planet_type* RemovePlanet(planet_type* planetToRemove)
{
    planet_type* currentPlanet = planetList;

    if (planetToRemove == planetList)
    {
        InformClient(planetToRemove);
        planetList = planetToRemove->next;
    }
    while (currentPlanet != NULL) {
        if (currentPlanet->next == planetToRemove) {
            InformClient(planetToRemove);
            planet_type* nextNextPlanet = currentPlanet->next->next;
            free(currentPlanet->next);
            currentPlanet->next = nextNextPlanet;
            planetCount--;
            return nextNextPlanet;
        }
        currentPlanet = currentPlanet->next;
    }
    return FALSE;
}

// Merges two planets, combining their mass and momentum into firstPlanet.
// This function will set secondPlanet->life to 0!
// Returns secondPlanet->next
void MergePlanets(planet_type* firstPlanet, planet_type* secondPlanet)
{
    double totalMass = firstPlanet->mass + secondPlanet->mass;
    double firstPlanetFractionOfMass = firstPlanet->mass / totalMass;
    double secondPlanetFractionOfMass = secondPlanet->mass / totalMass;
    double newXVelocity =
            (firstPlanet->vx * firstPlanetFractionOfMass) + (secondPlanet->vx * secondPlanetFractionOfMass);
    double newYVelocity =
            (firstPlanet->vy * firstPlanetFractionOfMass) + (secondPlanet->vy * secondPlanetFractionOfMass);
    double newXPosition =
            (firstPlanet->sx * firstPlanetFractionOfMass) + (secondPlanet->sx * secondPlanetFractionOfMass);
    double newYPosition =
            (firstPlanet->sy * firstPlanetFractionOfMass) + (secondPlanet->sy * secondPlanetFractionOfMass);
    int newLife = (firstPlanet->life > secondPlanet->life ? firstPlanet->life : secondPlanet->life);

    strcpy(firstPlanet->name, (firstPlanet->mass > secondPlanet->mass ? firstPlanet->name : secondPlanet->name));
    firstPlanet->sx = newXPosition;
    firstPlanet->sy = newYPosition;
    firstPlanet->vx = newXVelocity;
    firstPlanet->vy = newYVelocity;
    firstPlanet->mass = totalMass;
    firstPlanet->life = newLife;

    secondPlanet->life = 0;
}

// Run one of these for each planet that needs to simulate gravity (all of 'em)
// Returns nothing. It's a void function.
void PlanetUpdateThreadFunc(planet_type* planet)
{
    while (1) {
        double xaccel = 0;
        double yaccel = 0;
        pthread_mutex_lock(&mutex);
        planet_type* currentPlanet = planetList;
        while (currentPlanet != NULL) {
            if (currentPlanet != planet) {
                double xdiff = currentPlanet->sx - planet->sx;
                double ydiff = currentPlanet->sy - planet->sy;
                double r = sqrt(pow(xdiff, 2) + pow(ydiff, 2));

                double planetRadius = pow(((planet->mass * 3.0) / TWO_PI * 2.0), 1.0 / 3.0) * 0.1;
                double currentPlanetRadius = pow(((currentPlanet->mass * 3.0) / TWO_PI * 2.0), 1.0 / 3.0) * 0.1;
                double largestRadius = (planetRadius > currentPlanetRadius ? planetRadius : currentPlanetRadius);

                if (r < largestRadius) {
                    MergePlanets(currentPlanet, planet);
                    break;
                } else {
                    double accel = CAPITAL_G * (currentPlanet->mass / pow(r, 2));
                    xaccel += accel * (xdiff / r);
                    yaccel += accel * (ydiff / r);
                }
            }
            currentPlanet = currentPlanet->next;
        }

        if (planet->life <= 0) {
            RemovePlanet(planet);
            pthread_mutex_unlock(&mutex);
            break;
        }
        planet->vx += xaccel * DELTA_T;
        planet->vy += yaccel * DELTA_T;
        planet->sx += planet->vx * DELTA_T;
        planet->sy += planet->vy * DELTA_T;
        pthread_mutex_unlock(&mutex);
        MSLEEP(10);
    }
}

// Asynchronous function being called when a message arrives on an empty mailbox.
// Can't really return something by its nature as an asynchronous function call.
void MessageReceived()
{
    struct mq_attr attributes;
    do {
        planet_type* newPlanet;
        newPlanet = malloc(sizeof(planet_type));
        MQread(&mainBox, (void**) &newPlanet);

        if (strcmp(newPlanet->name, "CONNECT") == 0) {

        } else {
            pthread_mutex_lock(&mutex);
            AddExistingPlanet(newPlanet);
            pthread_create(planetThreads, NULL, (void*) PlanetUpdateThreadFunc, newPlanet);
            pthread_mutex_unlock(&mutex);
        }

        mq_getattr(mainBox, &attributes);
    } while (attributes.mq_curmsgs > 0);

    mq_notify(mainBox, signalEvent);
}

// Draw event for cairo, will be triggered each time a draw event is executed
static gboolean on_draw_event(GtkWidget* widget, cairo_t* cr, gpointer user_data)
{
    do_drawing(cr, widget); //Launch the actual draw method
    return FALSE; //Return something
}

// Keypress event for gdk, triggered every time a key is pressed in the server window.
gboolean on_key_press(GtkWidget* widget, GdkEventKey* eventKey, gpointer userData)
{
    if (eventKey->keyval == GDK_KEY_Escape) {
        mq_unlink("/MQ_Planets_MAIN");
        gtk_main_quit();
        return TRUE;
    }

    return FALSE;
}

//Do the drawing against the cairo surface area cr
static void do_drawing(cairo_t* cr, GtkWidget* widget)
{
    //Printing planets should reasonably be done something like this:
    // --------- for all planets in list:
    // --------- cairo_arc(cr, planet.xpos, planet.ypos, 10, 0, 2*3.1415)
    // --------- cairo_fill(cr)
    //------------------------------------------Insert planet drawings below-------------------------------------------
    GdkRGBA colour;

    double i = 0;

    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_paint_with_alpha(cr, 0.3); //background for the galaxy second input is value between 0 (light) and 1 dark

    pthread_mutex_lock(&mutex);
    planet_type* nextPlanet;
    nextPlanet = planetList;
    while (nextPlanet != NULL) {
        double planetRadius = pow(((nextPlanet->mass * 3.0) / TWO_PI * 2.0), 1.0 / 3.0) * 0.1;
        cairo_arc(cr, nextPlanet->sx, nextPlanet->sy, planetRadius, 0, TWO_PI);

        //gtk_style_context_get_color (context, gtk_style_context_get_state (context), &colour);
        if (strcmp(nextPlanet->name, "Sun") == 0) {
            colour.alpha = 1;
            colour.green = 1;
            colour.red = 1;
            colour.blue = 0;
        } else {
            i = RADTODEG(nextPlanet->pid % 360);
            colour.alpha = 1;
            colour.green = fabs(cos(i));
            colour.red = fabs(tan(i));
            colour.blue = fabs(sin(i));
        }

        gdk_cairo_set_source_rgba(cr, &colour);

        cairo_fill(cr);
        nextPlanet = nextPlanet->next;
    }
    pthread_mutex_unlock(&mutex);
    //------------------------------------------Insert planet drawings Above-------------------------------------------

    /*cairo_set_source_rgb(cr, 255, 255, 255); //Set RGB source of cairo, 0,0,0 = blac
    cairo_select_font_face(cr, "Comic Sans MS",
                           CAIRO_FONT_SLANT_ITALIC,
                           CAIRO_FONT_WEIGHT_NORMAL);
    cairo_move_to(cr, 20, 30);
    //cairo_show_text(cr, "You probably do not want to debug using text output, but you can");
    cairo_show_text(cr, globalText);*/
}

GtkTickCallback
on_frame_tick(GtkWidget* widget, GdkFrameClock* frame_clock, gpointer user_data) //Tick handler to update the frame
{
    gdk_frame_clock_begin_updating(frame_clock); //Update the frame clock
    gtk_widget_queue_draw(darea); //Queue a draw event
    gdk_frame_clock_end_updating(frame_clock); //Stop updating frame clock
}

int main(int argc, char* argv[]) //Main function
{
    //----------------------------------------Variable declarations should be placed below---------------------------------



    //----------------------------------------Variable declarations should be placed Above---------------------------------

    //GUI stuff, don't touch unless you know what you are doing, or if you talked to me
    gtk_init(&argc, &argv); //Initialize GTK environment
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL); //Create a new window which will serve as your top layer
    darea = gtk_drawing_area_new(); //Create draw area, which will be used under top layer window
    gtk_container_add(GTK_CONTAINER(window), darea); //add draw area to top layer window
    g_signal_connect(G_OBJECT(darea), "draw",
                     G_CALLBACK(on_draw_event), NULL); //Connect callback function for the draw event of darea
    g_signal_connect(window, "destroy", //Destroy event, not implemented yet, although not needed
                     G_CALLBACK(gtk_main_quit), NULL);

    //Hey I touched the GUI stuff (the code between this and DELIMITER_COMMENT below)
    g_signal_connect(G_OBJECT(window), "key_press_event", G_CALLBACK(on_key_press), NULL);
    //DELIMITER_COMMENT

    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER); //Set position of window
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600); //Set size of window
    gtk_window_set_title(GTK_WINDOW(window), "GTK window"); //Title
    gtk_widget_show_all(window); //Show window
    gtk_widget_add_tick_callback(darea, (GtkTickCallback) on_frame_tick, NULL,
                                 NULL); //Add timer callback functionality for darea
    //GUI stuff, don't touch unless you know what you are doing, or if you talked to me

    //---------Insert code for pthreads below------------------------------------------------
    srandom(time(NULL));

    mq_unlink("/MQ_Planets_MAIN");
    if (!MQcreate(&mainBox, "/MQ_Planets_MAIN")) {
        return 0;
    }

    signalEvent = malloc(sizeof(struct sigevent));
    signalEvent->sigev_notify = SIGEV_THREAD;
    signalEvent->sigev_notify_function = MessageReceived;

    pthread_mutex_init(&mutex, NULL);
    mq_notify(mainBox, signalEvent);
    //-------------------------------Insert code for pthreads above------------------------------------------------

    gtk_main();//Call gtk_main which handles basic GUI functionality
    return 0;
}