#include <cairo.h>
#include <gtk/gtk.h>
#include <pthread.h>


static void do_drawing(cairo_t *);
int x = 0;
int y = 0;
int x2 = 0;
GtkWidget *window;
GtkWidget *darea;
void*more_thread(void*args)
{

}
static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, //Draw event for cairo, will be triggered each time a draw event is executed
                              gpointer user_data)
{
    do_drawing(cr); //Launch the actual draw method
    return FALSE; //Return something
}

static void do_drawing(cairo_t *cr) //Do the drawing against the cairo surface area cr
{

    cairo_set_source_rgb(cr, 0, 0, 0); //Set RGB source of cairo, 0,0,0 = black
    x++; //dummy calculation
    y++;
    x2++;
    cairo_select_font_face(cr, "Purisa",
                           CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_BOLD);
    cairo_move_to(cr, 20, 30);
    cairo_show_text(cr, "You probably do not want to debug using text output, but you can");
    cairo_arc(cr, x,y,50,0,2*3.1415); //Create cairo shape: Parameters: Surface area, x pos, y pos, radius, Angle 1, Angle 2
    cairo_fill(cr);
    cairo_arc(cr, x2+100,0,25,0,2*3.1415); //These drawings are just examples, remove them once you understood how to draw your planets
    cairo_fill(cr);
    //Printing planets should reasonably be done something like this:
    // --------- for all planets in list:
    // --------- cairo_arc(cr, planet.xpos, planet.ypos, 10, 0, 2*3.1415)
    // --------- cairo_fill(cr)
    //------------------------------------------Insert planet drawings below-------------------------------------------




    //------------------------------------------Insert planet drawings Above-------------------------------------------

}
GtkTickCallback on_frame_tick(GtkWidget * widget, GdkFrameClock * frame_clock, gpointer user_data) //Tick handler to update the frame
{
    gdk_frame_clock_begin_updating (frame_clock); //Update the frame clock
    gtk_widget_queue_draw(darea); //Queue a draw event
    gdk_frame_clock_end_updating (frame_clock); //Stop updating frame clock
}

int main(int argc, char *argv[]) //Main function
{
    //----------------------------------------Variable declarations should be placed below---------------------------------
    pthread_t i_am_thread;

    //----------------------------------------Variable declarations should be placed Above---------------------------------

    //GUI stuff, don't touch unless you know what you are doing, or if you talked to me
    gtk_init(&argc, &argv); //Initialize GTK environment
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL); //Create a new window which will serve as your top layer
    darea = gtk_drawing_area_new(); //Create draw area, which will be used under top layer window
    gtk_container_add(GTK_CONTAINER(window), darea); //add draw area to top layer window
    g_signal_connect(G_OBJECT(darea), "draw",
                     G_CALLBACK(on_draw_event), NULL); //Connect callback function for the draw event of darea
    g_signal_connect(window, "destroy", //Destroy event, not implemented yet, altough not needed
                     G_CALLBACK(gtk_main_quit), NULL);

    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER); //Set position of window
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600); //Set size of window
    gtk_window_set_title(GTK_WINDOW(window), "GTK window"); //Title
    gtk_widget_show_all(window); //Show window
    gtk_widget_add_tick_callback(darea, on_frame_tick, NULL, 1); //Add timer callback functionality for darea
    //GUI stuff, don't touch unless you know what you are doing, or if you talked to me


    //-------------------------------Insert code for pthreads below------------------------------------------------
    int ec = pthread_create(&i_am_thread, NULL, more_thread, (void*)0);

    //-------------------------------Insert code for pthreads above------------------------------------------------


    gtk_main();//Call gtk_main which handles basic GUI functionality
    return 0;
}