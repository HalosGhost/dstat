#include "dstat.h"

#define UPDATE_MODULE_AT(mod,ival) \
    do { \
        if ( !(c_time % (ival)) ) { \
            if ( (status = (mod)) != EXIT_SUCCESS ) { \
                goto cleanup; \
            } \
        } \
    } while ( false )

#define general_termination() \
    do { \
        snd_hctl_close(alsa_handle); \
        if ( alsa_control ) { snd_ctl_elem_value_free(alsa_control); } \
        if ( alsa_sid ) { snd_ctl_elem_id_free(alsa_sid); } \
        if ( dpy ) { \
            XCloseDisplay(dpy); \
        } else { \
            printf("\x1b[?25h"); \
        } \
        closelog(); \
    } while ( false )

signed
main (signed argc, char * argv []) {

    signed status = EXIT_SUCCESS;
    openlog(NULL, LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "Starting\n");

    signal(2, signal_handler);
    signal(3, signal_handler);
    signal(15, signal_handler);

    char en_state [2]        = "D";
    uint8_t wl_strength [1]  = { 0 };
    char wl_essid [33]       = "W";
    long audio_vol [1]       = { 0 };
    char audio_mut [2]       = "%";
    uint8_t bat_cap [1]      = { 0 };
    double bat_pow [1]       = { -100 };
    char bat_time [29]       = " | 00:00:00 till replenished";
    char time_state [44]     = "00.00 (UTC) | Monday, 01 January 0001";
    bool stdout_flag         = false;
    char status_line [159]   = "";

    snd_ctl_elem_id_malloc(&alsa_sid);
    if ( !alsa_sid ) {
        syslog(LOG_ERR, "Could not allocate ALSA element\n");
        goto cleanup;
    }

    snd_ctl_elem_value_malloc(&alsa_control);
    if ( !alsa_control ) {
        syslog(LOG_ERR, "Could not allocate ALSA control\n");
        goto cleanup;
    }

    snd_hctl_open(&alsa_handle, SNDDV, 0);
    snd_hctl_load(alsa_handle);

    snd_ctl_elem_id_set_interface(alsa_sid, SND_CTL_ELEM_IFACE_MIXER);

    const char * vos = "hs";
    for ( signed oi = 0, c = getopt_long(argc, argv, vos, os, &oi);
          c != -1; c = getopt_long(argc, argv, vos, os, &oi) ) {
        switch ( c ) {
            case 'h': puts(usage_str); goto cleanup;
            case 's': stdout_flag = true; break;
        }
    }

    if ( !stdout_flag ) {
        dpy = XOpenDisplay(NULL);
        if ( !dpy ) {
            syslog(LOG_ERR, "Could not open display\n");
            return EXIT_FAILURE;
        }
    } else {
        fputs("\x1b[?25l", stdout);
    }

    for ( time_t c_time = 0; ; c_time = time(NULL) ) {
        UPDATE_MODULE_AT(get_en_state(en_state), EN_INTERVAL);
        UPDATE_MODULE_AT(get_wl_strength(wl_strength), WL_INTERVAL);
        UPDATE_MODULE_AT(get_wl_essid(wl_essid), WL_INTERVAL);
        UPDATE_MODULE_AT(get_aud_volume(audio_vol), VL_INTERVAL);
        UPDATE_MODULE_AT(get_aud_mute(audio_mut), VL_INTERVAL);
        UPDATE_MODULE_AT(get_bat_state(bat_cap, bat_pow, bat_time), BT_INTERVAL);
        UPDATE_MODULE_AT(get_time_state(time_state), TM_INTERVAL);

        if ( !(c_time % PT_INTERVAL) ) {
            snprintf(status_line, 135 - !stdout_flag, LNFMT,
                    en_state,
                    wl_essid, wl_bars[*wl_strength],
                    *audio_vol, audio_mut,
                    *bat_cap, *bat_pow, bat_time,
                    time_state, stdout_flag ? STCHR : " ");

            if ( dpy ) {
                XStoreName(dpy, DefaultRootWindow(dpy), status_line);
                XSync(dpy, False);
            } else {
                fputs(status_line, stdout);
                fflush(stdout);
            }
        } sleep(1);
    }

    cleanup:
        syslog(LOG_INFO, "Terminating\n");
        general_termination();
        return status;
}

void
signal_handler (signed signum) {

    syslog(LOG_INFO, "Caught %s; Terminating\n", sys_siglist[signum]);
    general_termination();
    exit(EXIT_SUCCESS);
}

#define check_null_arg(ptr) \
    do { \
        if ( !(ptr) ) { \
            syslog(LOG_ERR, "%s() was passed a NULL pointer\n", __func__); \
            exit(EXIT_FAILURE); \
        } \
    } while ( false )

signed
get_en_state (char * state) {

    check_null_arg(state);

    signed errsv = 0;
    errno = 0;
    FILE * in = fopen(EPATH, "r");
    if ( !in ) {
        errsv = errno;
        syslog(LOG_ERR, FAIL_OPEN(EPATH) ": %s\n", strerror(errsv));
        return EXIT_FAILURE;
    }

    errno = 0;
    char st = 0;
    if ( fscanf(in, "%c", &st) != 1 ) {
        errsv = errno;
        syslog(LOG_ERR, FAIL_READ(EPATH) ": %s\n", strerror(errsv));
        fclose(in);
        return EXIT_FAILURE;
    } fclose(in);

    *state = st - 32;
    return EXIT_SUCCESS;
}

signed
get_wl_strength (uint8_t * strength) {

    check_null_arg(strength);

    signed errsv = 0;
    errno = 0;
    FILE * in = fopen(WPATH, "r");
    if ( !in ) {
        errsv = errno;
        syslog(LOG_ERR, FAIL_OPEN(WPATH) ": %s\n", strerror(errsv));
        return EXIT_FAILURE;
    }

    errno = 0;
    uint8_t n = 0;
    if ( fscanf(in, "%*[^\n]\n%*[^\n]\n" WFACE ": %*d %" SCNu8 ".", &n) != 1 ) {
        errsv = errno;
        syslog(LOG_ERR, FAIL_READ(WPATH) ": %s\n", strerror(errsv));
        n = 0;
    } fclose(in);

    *strength = !n ? n : n * 7 / 100;
    return EXIT_SUCCESS;
}

signed
get_wl_essid (char * ssid) {

    check_null_arg(ssid);

    strncpy(ssid, 2, "W");

    signed sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct iwreq w;

    strncpy(w.ifr_ifrn.ifrn_name, WFACE, IFNAMSIZ);

    w.u.essid.pointer = ssid;
    w.u.data.length = IW_ESSID_MAX_SIZE;
    w.u.data.flags = 0;

    ioctl(sock, SIOCGIWESSID, &w);
    close(sock);

    return EXIT_SUCCESS;
}

signed
get_aud_volume (long * volume) {

    check_null_arg(volume);

    snd_ctl_elem_id_set_name(alsa_sid, "Master Playback Volume");
    snd_hctl_elem_t * alsa_element = snd_hctl_find_elem(alsa_handle, alsa_sid);

    snd_ctl_elem_value_set_id(alsa_control, alsa_sid);

    snd_hctl_elem_read(alsa_element, alsa_control);
    *volume = snd_ctl_elem_value_get_integer(alsa_control, 0) * 100 / ((1 << 16) - 1);

    return EXIT_SUCCESS;
}

signed
get_aud_mute (char * mute) {

    check_null_arg(mute);

    snd_ctl_elem_id_set_name(alsa_sid, "Master Playback Switch");
    snd_hctl_elem_t * alsa_element = snd_hctl_find_elem(alsa_handle, alsa_sid);

    snd_ctl_elem_value_set_id(alsa_control, alsa_sid);

    snd_hctl_elem_read(alsa_element, alsa_control);
    *mute = snd_ctl_elem_value_get_boolean(alsa_control, 0) ? '%' : 'M';

    return EXIT_SUCCESS;
}

signed
get_bat_state (uint8_t * cap, double * pow, char * time) {

    check_null_arg(pow);
    check_null_arg(time);

    signed errsv = 0;
    errno = 0;
    FILE * in = fopen(BPATH "/uevent", "r");
    if ( !in ) {
        errsv = errno;
        syslog(LOG_ERR, FAIL_OPEN(BPATH) "/uevent: %s\n", strerror(errsv));
        return EXIT_FAILURE;
    }

    enum { DISCHARGING, EMPTY, CHARGING, FULL } status = EMPTY;
    long power_now = 0,
         current_now = 0;
    unsigned long voltage_now = 0,
                  energy_full_design = 0,
                  energy_full = 0,
                  charge_full = 0,
                  charge_full_design = 0,
                  energy_now = 0,
                  charge_now = 0;
    uint8_t capacity = 0;

    char keybuf [64] = "", // pretty sure it should never be larger than 32
         val    [64] = "";

    while ( fscanf(in, "%[^=]=%s\n", keybuf, val) != EOF ) {
        char * key = keybuf + sizeof("POWER_SUPPLY_") - 1; // ignore nul byte

        if ( !strncmp(key, "STATUS", 6) ) {
            switch ( val[0] ) {
                case 'D': status = DISCHARGING; break;
                case 'C': status = CHARGING; break;
                case 'F': status = FULL; break;
            }
        } else if ( !strncmp(key, "POWER_NOW", 9) ) {
            sscanf(val, "%ld", &power_now);
        } else if ( !strncmp(key, "CURRENT_NOW", 11) ) {
            sscanf(val, "%ld", &current_now);
        } else if ( !strncmp(key, "VOLTAGE_NOW", 11) ) {
            sscanf(val, "%lu", &voltage_now);
        } else if ( !strncmp(key, "ENERGY_FULL_DESIGN", 18) ) {
            sscanf(val, "%lu", &energy_full_design);
        } else if ( !strncmp(key, "ENERGY_FULL", 11) ) {
            sscanf(val, "%lu", &energy_full);
        } else if ( !strncmp(key, "CHARGE_FULL_DESIGN", 18) ) {
            sscanf(val, "%lu", &charge_full_design);
        } else if ( !strncmp(key, "CHARGE_FULL", 11) ) {
            sscanf(val, "%lu", &charge_full);
        } else if ( !strncmp(key, "ENERGY_NOW", 10) ) {
            sscanf(val, "%lu", &energy_now);
        } else if ( !strncmp(key, "CHARGE_NOW", 10) ) {
            sscanf(val, "%lu", &charge_now);
        } else if ( !strncmp(key, "CAPACITY", 8) ) {
            sscanf(val, "%" SCNu8, &capacity);
        }
    }

    fclose(in);

    *cap = capacity;

    voltage_now /= 1000;

    power_now   /= 1000;
    current_now /= 1000;

    energy_now         /= voltage_now;
    energy_full        /= voltage_now;
    energy_full_design /= voltage_now;
    charge_now         /= 1000;
    charge_full        /= 1000;
    charge_full_design /= 1000;

    long rate = power_now ? power_now : current_now;
    rate = rate >= 0 ? rate : -rate;
    while ( rate >= 10000 ) {
        rate /= 10;
    }

    unsigned long max_capacity = charge_full        ? charge_full        :
                                 energy_full        ? energy_full        :
                                 charge_full_design ? charge_full_design :
                                 energy_full_design ? energy_full_design : 0;
    unsigned long cur_capacity = charge_now ? charge_now : energy_now;

    unsigned long target = cur_capacity;
    double power = (signed long )-voltage_now * rate / 1000000.0;
    if ( status == CHARGING ) {
        target = max_capacity - cur_capacity;
        power *= -1;
    }

    *pow = power;

    unsigned long seconds = 3600 * target / (unsigned long )rate;
    unsigned long hours = seconds / 3600;
    seconds -= hours * 3600;

    unsigned long minutes = seconds / 60;
    seconds -= minutes * 60;

    const char * when = status == CHARGING ? "replenished" : "depleted";

    if ( hours || minutes || seconds ) {
        snprintf(time, 29, BTFMT, hours, minutes, seconds, when);
    } else {
        snprintf(time, 29, "");
    } return EXIT_SUCCESS;
}

signed
get_time_state (char * state) {

    check_null_arg(state);

    time_t current;
    time(&current);
    if ( !strftime(state, 43, TMFMT, localtime(&current)) ) {
        syslog(LOG_ERR, "strftime() returned 0\n");
        return EXIT_FAILURE;
    } return EXIT_SUCCESS;
}

// vim: set ts=4 sw=4 et:
