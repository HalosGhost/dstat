/*********************************\
* Status Output Program for DWM   *
* License: GPLv2 Sam Stuewe, 2014 *
\*********************************/

#include <stdint.h>   // int32_t, uint32_t
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

    int32_t n = 0, l;
    char stat, line [81];
    time_t current;

    for ( FILE * in; ; sleep(6) ) {
        l = 80;

        if ( (in = fopen("/sys/class/net/" EFACE "/operstate", "r")) ) {
            fscanf(in, "%c", &stat); fclose(in);
            l -= snprintf(line + (80 - l), (uint32_t )l, "E: %c | ",
                          stat == 'u' ? 'U' : 'D');
        }

        if ( (in = fopen("/proc/net/wireless", "r")) ) {
            const char wifi_levels [4][13] = { "⡀   ", "⡀⡄  ", "⡀⡄⡆ ", "⡀⡄⡆⡇" };
            fscanf(in, "%*[^\n]\n%*[^\n]\n" WFACE ": %*d %d.", &n); fclose(in);
            l -= snprintf(line + (80 - l), (uint32_t )l, "%s | ",
                          wifi_levels[n / 17]);
        }

        if ( (in = popen("ponymix get-volume", "r")) ) {
            fscanf(in, "%d", &n); pclose(in);
            l -= snprintf(line + (80 - l), (uint32_t )l, "A: %d%c | ", n,
                          system("ponymix is-muted") ? '%' : 'M');
        }

        if ( (in = fopen("/sys/class/power_supply/" BAT "/capacity", "r")) ) {
            fscanf(in, "%d", &n); fclose(in);
            l -= snprintf(line + (80 - l), (uint32_t )l, "B: %d", n);

            if ( (in = fopen("/sys/class/power_supply/" BAT "/status", "r")) ) {
                fscanf(in, "%c", &stat); fclose(in);
                l -= snprintf(line + (80 - l), (uint32_t )l, "%s | ",
                              stat == 'C' ? "ϟ" : "D");
            }
        }

        // Clock //
        time(&current);
        l -= strftime(line + (80 - l), (uint32_t )l, "%H.%M | %A, %d %B %Y",
                      localtime(&current));

        XStoreName(display, DefaultRootWindow(display), line);
        XSync(display, False);
    } XCloseDisplay(display); return 0;
}

// vim:set ts=4 sw=4 et:
