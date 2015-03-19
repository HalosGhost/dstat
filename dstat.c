/*********************************\
* Status Output Program for DWM   *
* License: GPLv2 Sam Stuewe, 2014 *
\*********************************/

#include <stdint.h>   // int32_t
#include <stdio.h>    // fputs(), fopen(), fscanf(), fclose(), popen(), pclose()
#include <unistd.h>   // sleep()
#include <stdbool.h>  // bool
#include <stdlib.h>   // system()
#include <time.h>     // time_t, time(), strftime(), localtime()
#include <X11/Xlib.h> // Display, XOpenDisplay(), XStoreName(),
                      // DefaultRootWindow(), XSync(), XCloseDisplay()

// Input Files //
#define EFACE "enp0s25"
#define WFACE "wlp3s0"
#define BAT   "BAT0"

// Main Function //
int32_t
main (void) {

    Display * display;
    if ( !(display = XOpenDisplay(NULL)) ) {
        fputs("Could not open display\n", stderr);
        return 1;
    }

    int32_t wr, n = 0;
    uint32_t len = 75;
    char stat = 0, line [len + 1];
    time_t current;

    for ( FILE * in; ; sleep(6) ) {
        wr = 0; len = 75;

        if ( (in = fopen("/sys/class/net/" EFACE "/operstate", "r")) ) {
            fscanf(in, "%c", &stat); fclose(in);
            wr += snprintf(line + wr, len, "E: %c | ", stat == 'u' ? 'U' : 'D');
        } len -= (uint32_t )wr;

        if ( (in = fopen("/proc/net/wireless", "r")) ) {
            fscanf(in, "%*[^\n]\n%*[^\n]\n" WFACE ": %*d %d.", &n); fclose(in);
            wr += snprintf(line + wr, len, "W: %.3g | ", (double )n / 70 * 100);
        } len -= (uint32_t )wr;

        if ( (in = popen("ponymix get-volume", "r")) ) {
            fscanf(in, "%d", &n); pclose(in);
            bool mute_status = system("ponymix is-muted");
            wr += snprintf(line + wr, len, "A: %d%c | ", n, mute_status ? '%' : 'M');
        } len -= (uint32_t )wr;

        if ( (in = fopen("/sys/class/power_supply/" BAT "/capacity", "r")) ) {
            fscanf(in, "%d", &n); fclose(in);
            wr += snprintf(line + wr, len, "B: %d", n);
            len -= (uint32_t )wr;


            if ( (in = fopen("/sys/class/power_supply/" BAT "/status", "r")) ) {
                fscanf(in, "%c", &stat); fclose(in);
            }

            wr += snprintf(line + wr, len, "%c | ", stat == 'C' ? 'C' : 'D');
        } len -= (uint32_t )wr;

        // Clock //
        time(&current);
        wr += strftime(line + wr, len, "%H.%M | %A, %d %B %Y", localtime(&current));
        XStoreName(display, DefaultRootWindow(display), line);
        XSync(display, False);
    } XCloseDisplay(display); return 0;
}

// vim:set ts=4 sw=4 et:
