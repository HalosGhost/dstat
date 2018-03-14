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
#define FAIL_IOCTL(x) "Failed to execute " x " ioctl"

/* Personal Configuration */
#define EFACE "en0"
#define EPATH "/sys/class/net/" EFACE "/operstate"
#define WFACE "wl0"
#define WPATH "/proc/net/wireless"
#define SNDDV "default"
#define BATDV "BAT0"
#define BPATH "/sys/class/power_supply/" BATDV
#define BTFMT " %.2lu:%.2lu till %s"
#define MODSEP " | "
#define TMFMT "%H.%M (%Z)" MODSEP "%A, %d %B %Y"
#define STCHR "\n"

#define EN_INTERVAL 60
#define WL_INTERVAL 30
#define VL_INTERVAL 30
#define BT_INTERVAL 30
#define TM_INTERVAL 15
#define PT_INTERVAL 15

#define ENABLE_MOD_EN 1
#define ENABLE_MOD_WL 1
#define ENABLE_MOD_AU 1
#define ENABLE_MOD_BT 1
#define ENABLE_MOD_CK 1

#define MODS_ENABLED ( \
    ENABLE_MOD_EN    + \
    ENABLE_MOD_WL    + \
    ENABLE_MOD_AU    + \
    ENABLE_MOD_BT    + \
    ENABLE_MOD_CK    )

#if !(MODS_ENABLED)
    #error "You must enable at least one module"
#endif

/* Module-Dependent Configuration */
#define SEPLN (sizeof MODSEP)

#if ENABLE_MOD_EN == 1
    #define MOD_EN_SIZE 5
#else
    #define MOD_EN_SIZE 0
#endif

#if ENABLE_MOD_WL == 1
    #define MOD_WL_SIZE (IW_ESSID_MAX_SIZE + 24)
#else
    #define MOD_WL_SIZE 0
#endif

#if ENABLE_MOD_AU == 1
    #define MOD_AU_SIZE 8
#else
    #define MOD_AU_SIZE 0
#endif

#if ENABLE_MOD_BT == 1
    #define MOD_BT_SIZE 47
#else
    #define MOD_BT_SIZE 0
#endif

#if ENABLE_MOD_CK == 1
    #define MOD_CK_SIZE (40 + SEPLN)
#else
    #define MOD_CK_SIZE 0
#endif

#define LNSZE ( MOD_EN_SIZE \
              + MOD_WL_SIZE \
              + MOD_AU_SIZE \
              + MOD_BT_SIZE \
              + MOD_CK_SIZE \
              + 5 * SEPLN + 1)

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
