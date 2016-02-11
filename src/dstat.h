#pragma once

#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <X11/Xlib.h>
#include <syslog.h>

#define FAIL_OPEN(x) "Failed to open " x
#define FAIL_READ(x) "Failed to read from " x

/* Personal Configuration */
#define EFACE "en0"
#define EPATH "/sys/class/net/" EFACE "/operstate"
#define WFACE "wl0"
#define WPATH "/proc/net/wireless"
#define PASNK 0
#define BATDV "BAT0"
#define BPATH "/sys/class/power_supply/" BATDV
#define TMFMT "%H.%M (%Z) | %A, %d %B %Y"
#define LNFMT "E: %s | W: %s | A: %" PRIu16 "%s | B: %" PRIu8 "%s | %s"

#define EN_INTERVAL 60
#define WL_INTERVAL 30
#define VL_INTERVAL 30
#define BT_INTERVAL 30
#define TM_INTERVAL 15
#define PT_INTERVAL 15

static const char wl_bars [][22] = {
	"No Signal", "▂", "▂▃", "▂▃▄", "▂▃▄▅", "▂▃▄▅▆", "▂▃▄▅▆▇", "▂▃▄▅▆▇█"
};

signed
get_en_state (char *);

signed
get_wl_strength (uint8_t *);

signed
get_bat_cap (uint8_t *);

signed
get_bat_state (char *);

signed
get_time_state (char *);

signed
get_aud_volume (uint16_t *);

signed
get_aud_mute (char *);
