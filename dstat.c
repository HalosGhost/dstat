/*********************************\
* Status Output Program for DWM   *
* License: GPLv2 Sam Stuewe, 2014 *
\*********************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include <X11/Xlib.h>

// Input Files //
static const char * wireStat = "/sys/class/net/eth0/operstate";
static const char * wifiStat = "/proc/net/wireless";
static const char * battNow  = "/sys/class/power_supply/BAT0/capacity";
static const char * battStat = "/sys/class/power_supply/BAT0/status";

// Variables //
static FILE * in;
static char stat, clck[38] = "", back[72] = "", forth[72] = "";
static int n;
static time_t current;
static Display * display;

// Main Function //
int main (int argc, char ** argv)
{   if ( argc > 1 ) _usage();

    if ( !(display = XOpenDisplay(NULL)) )
    {   fprintf(stderr, "could not open display\n");
        return 1;
    }

    for ( ; ; sleep(6) )
    {   // Wired Interface //
        if ( (in = fopen(wireStat, "r")) )
        {   fscanf(in, "%c", &stat); fclose(in);
            snprintf(back, sizeof(back), "E: %s", ( stat == 'u' ? "U" : "D" ));
            snprintf(forth, sizeof(forth), back);
        }

        // Wi-Fi Interface //
        if ( (in = fopen(wifiStat, "r")) )
        {   n = 0;
            fscanf(in, "%*[^\n]\n%*[^\n]\nwlan0: %*d %d.", &n); fclose(in);
            snprintf(back, sizeof(back), "%s | W: %.3g", forth, (double )n/70 * 100);
            snprintf(forth, sizeof(forth), back);
        }

        // Volume Monitor //
        if ( (in = popen("ponymix get-volume", "r")) )
        {   n = 0;
			fscanf(in, "%d", &n); pclose(in);
            int isMuted = system("ponymix is-muted");
            snprintf(back, sizeof(back), "%s | A: %d%s", forth, n, ( isMuted ? "" : "M"));
            snprintf(forth, sizeof(forth), back);
        }

        // Battery Monitor //
        if ( (in = fopen(battNow, "r")) )
        {   fscanf(in, "%d", &n); fclose(in);
            snprintf(forth, sizeof(forth), "%s | B: %d", back, n);
            if ( (in = fopen(battStat, "r")) )
            {   fscanf(in, "%c", &stat);
                fclose(in);
            }
            snprintf(back, sizeof(back), "%s%s", forth, ( stat == 'C' ? "C" : "" ));
            snprintf(forth, sizeof(forth), back);
        }

        // Clock //
        time(&current);
        strftime(clck, 37, "%H.%M | %A, %d %B %Y", localtime(&current));
        snprintf(back, sizeof(back), "%s | %s", forth, clck);
		XStoreName(display, DefaultRootWindow(display), statusOut);
		XSync(display, False);
    }
    XCloseDisplay(display);
    return 0;
}

// vim:set ts=4 sw=4 et:
