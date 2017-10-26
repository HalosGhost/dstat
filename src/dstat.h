#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#define _POSIX_C_SOURCE 1
#pragma clang diagnostic pop

#include <stdint.h>
#include <inttypes.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <X11/Xlib.h>
#include <syslog.h>
#include <signal.h>
#include <getopt.h>
#include <alsa/asoundlib.h>
#include <alsa/control.h>
#include <linux/if.h>
#include <linux/wireless.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stropts.h>

#define FAIL_OPEN(x) "Failed to open " x
#define FAIL_READ(x) "Failed to read from " x

/* Personal Configuration */
#define EFACE "en0"
#define EPATH "/sys/class/net/" EFACE "/operstate"
#define WFACE "wl0"
#define WPATH "/proc/net/wireless"
#define SNDDV "default"
#define BATDV "BAT0"
#define BPATH "/sys/class/power_supply/" BATDV
#define BTFMT " %.2lu:%.2lu till %s"
#define TMFMT "%H.%M (%Z) | %A, %d %B %Y"
#define LNFMT " E: %s | %s: %s | A: %ld%s | B: %" PRIu8 "%% (%+.2lgW)%s | %s%s"
#define STCHR "\n"
#define LNSZE 155

#define EN_INTERVAL 60
#define WL_INTERVAL 30
#define VL_INTERVAL 30
#define BT_INTERVAL 30
#define TM_INTERVAL 15
#define PT_INTERVAL 15

static const char wl_bars [][22] = {
    "No Signal", "▂", "▂▃", "▂▃▄", "▂▃▄▅", "▂▃▄▅▆", "▂▃▄▅▆▇", "▂▃▄▅▆▇█"
};

static struct option os [] = {
    { "help",   0, 0, 'h' },
    { "stdout", 0, 0, 's' },
    { 0,        0, 0, 0   }
};

static const char usage_str [] =
    "Usage: dstat [option ...]\n"
    "dstat -- a simple dwm/stdout status program\n\n"
    "Options:\n"
    "  -h, --help      Print this help and exit\n"
    "  -s, --stdout    Output to stdout";

static Display * dpy;
extern const char * const sys_siglist [];
static snd_hctl_t * alsa_handle;
static snd_ctl_elem_id_t * alsa_sid;
static snd_ctl_elem_value_t * alsa_control;

_Noreturn void
signal_handler (signed);

signed
get_en_state (char *);

signed
get_wl_strength (uint8_t *);

signed
get_wl_essid (char *);

signed
get_bat_state (uint8_t *, double *, char *);

signed
get_time_state (char *);

signed
get_aud_volume (long *);

signed
get_aud_mute (char *);
