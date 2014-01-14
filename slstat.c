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
static char stat, clck[38] = "", back[70] = "", forth[70] = "";
static int n;
static time_t current;
static Display *dpy;

// Prototypes //
void _usage (void);
void _setStatus (char * statusOut);

// Main Function //
int main (int argc, char ** argv)
{   if ( argc > 1 ) _usage();

    if ( !(dpy = XOpenDisplay(NULL)) )
    {   fprintf(stderr, "could not open display\n");
        return 1;
    }

    for ( ; ; sleep(15) )
    {   // Wired Interface //
        if ( (in = fopen(wireStat, "r")) )
        {   fscanf(in, "%c", &stat); fclose(in);
            snprintf(back, sizeof(back), "E: %s", ( stat == 'u' ? "U" : "D" ));
        }

        // Wi-Fi Interface //
        if ( (in = fopen(wifiStat, "r")) )
        {   n = 0;
            fscanf(in, "%*[^\n]\n%*[^\n]\nwlan0: %*d %d.", &n); fclose(in);
            if ( n > 0 ) snprintf(forth, sizeof(forth), "%s | W: %d", back, n);
            else snprintf(forth, sizeof(forth), "%s | W: D", back);
        }

        // Volume Monitor //
        if ( (in = popen("ponymix get-volume", "r")) )
        {   fscanf(in, "%d", &n); pclose(in);
            snprintf(back, sizeof(back), "%s | A: %d%s", forth, n, ( system("ponymix is-muted") ? "" : "M"));
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
        }

        // Clock //
        time(&current);
        strftime(clck, 37, "%H.%M | %A, %d %B %Y", localtime(&current));
        snprintf(forth, sizeof(forth), "%s | %s\n", back, clck);
        _setStatus(forth);
    }
    return 0;
}

// Usage //
void _usage (void)
{   fprintf(stderr, "Usage: slstat [-h|--help]\n");
    exit(0);
}

// Set Status //
void _setStatus (char * statusOut)
{   XStoreName(dpy, DefaultRootWindow(dpy), statusOut);
    XSync(dpy, False);
}
// vim:set ts=4 sw=4 et:
