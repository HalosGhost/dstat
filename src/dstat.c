#include "dstat.h"

signed
get_en_state (char * state) {

    if ( !state ) {
        syslog(LOG_ERR, "get_en_state() was passed a NULL pointer\n");
        return EXIT_FAILURE;
    }

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
        return EXIT_FAILURE;
    } fclose(in);

    *state = st - 32;
    return EXIT_SUCCESS;
}

signed
get_wl_strength (uint8_t * strength) {

    if ( !strength ) {
        syslog(LOG_ERR, "get_wl_strength() was passed a NULL pointer\n");
        return EXIT_FAILURE;
    }

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
        return EXIT_FAILURE;
    } fclose(in);

    *strength = !n ? n : (n / 10) + 1;
    return EXIT_SUCCESS;
}

signed
get_aud_volume (uint16_t * volume) {

    if ( !volume ) {
        syslog(LOG_ERR, "get_aud_volume() was passed a NULL pointer\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

signed
get_aud_mute (char * mute) {

    if ( !mute ) {
        syslog(LOG_ERR, "get_aud_mute() was passed a NULL pointer\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

signed
get_bat_cap (uint8_t * capacity) {

    if ( !capacity ) {
        syslog(LOG_ERR, "get_bat_cap() was passed a NULL pointer\n");
        return EXIT_FAILURE;
    }

    signed errsv = 0;
    errno = 0;
    FILE * in = fopen(BPATH "/capacity", "r");
    if ( !in ) {
        errsv = errno;
        syslog(LOG_ERR, FAIL_OPEN(BPATH) "/capacity: %s\n", strerror(errsv));
        return EXIT_FAILURE;
    }

    errno = 0;
    if ( fscanf(in, "%" SCNu8, capacity) != 1 ) {
        errsv = errno;
        syslog(LOG_ERR, FAIL_READ(BPATH) "/capacity: %s\n", strerror(errsv));
        return EXIT_FAILURE;
    } fclose(in); return EXIT_SUCCESS;
}

signed
get_bat_state (char * state) {

    if ( !state ) {
        syslog(LOG_ERR, "get_bat_state() was passed a NULL pointer\n");
        return EXIT_FAILURE;
    }

    signed errsv = 0;
    errno = 0;
    FILE * in = fopen(BPATH "/status", "r");
    if ( !in ) {
        errsv = errno;
        syslog(LOG_ERR, FAIL_OPEN(BPATH) "/status: %s\n", strerror(errsv));
        return EXIT_FAILURE;
    }

    char st = 0;
    errno = 0;
    if ( fscanf(in, "%c", &st) != 1 ) {
        errsv = errno;
        syslog(LOG_ERR, FAIL_READ(BPATH) "/status: %s\n", strerror(errsv));
        return EXIT_FAILURE;
    } fclose(in);

    snprintf(state, sizeof "ϟ", "%s", st == 'C' ? "ϟ" : "D");
    return EXIT_SUCCESS;
}

signed
get_time_state (char * state) {

    if ( !state ) {
        syslog(LOG_ERR, "get_time_state() was passed a NULL pointer\n");
        return EXIT_FAILURE;
    }

    time_t current;
    time(&current);
    if ( !strftime(state, 43, TMFMT, localtime(&current)) ) {
        syslog(LOG_ERR, "strftime() returned 0\n");
        return EXIT_FAILURE;
    } return EXIT_SUCCESS;
}

signed
main (void) {

    signed status = EXIT_SUCCESS;
    openlog(NULL, LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "Starting\n");

    Display * dpy = XOpenDisplay(NULL);
    if ( !dpy ) {
        syslog(LOG_ERR, "Could not open display\n");
        return EXIT_FAILURE;
    }

    char en_state [2]        = "D";
    uint8_t wl_strength [1]  = { 0 };
    uint16_t audio_vol [1]   = { 0 };
    char audio_mut [2]       = "%";
    uint8_t bat_capacity [1] = { 0 };
    char bat_state [3]       = "D";
    char time_state [44]     = "00.00 (GMT) | Thursday, 01 January";
    char status_line [102]   = "";

    for ( time_t c_time = 0; ; c_time = time(NULL) ) {
        if ( !(c_time % EN_INTERVAL) ) {
            if ( (status = get_en_state(en_state)) != EXIT_SUCCESS ) {
                goto cleanup;
            }
        }

        if ( !(c_time % WL_INTERVAL) ) {
            if ( (status = get_wl_strength(wl_strength)) != EXIT_SUCCESS ) {
                goto cleanup;
            }
        }

        if ( !(c_time % VL_INTERVAL) ) {
            if ( (status = get_aud_volume(audio_vol)) != EXIT_SUCCESS ) {
                goto cleanup;
            }
        }

        if ( !(c_time % VL_INTERVAL) ) {
            if ( (status = get_aud_mute(audio_mut)) != EXIT_SUCCESS ) {
                goto cleanup;
            }
        }

        if ( !(c_time % BT_INTERVAL) ) {
            if ( (status = get_bat_cap(bat_capacity)) != EXIT_SUCCESS ) {
                goto cleanup;
            }
        }

        if ( !(c_time % BT_INTERVAL) ) {
            if ( (status = get_bat_state(bat_state)) != EXIT_SUCCESS ) {
                goto cleanup;
            }
        }

        if ( !(c_time % TM_INTERVAL) ) {
            if ( (status = get_time_state(time_state)) != EXIT_SUCCESS ) {
                goto cleanup;
            }
        }

        if ( !(c_time % PT_INTERVAL) ) {
            snprintf(status_line, 99, LNFMT,
                    en_state,
                    wl_bars[*wl_strength],
                    *audio_vol, audio_mut,
                    *bat_capacity, bat_state,
                    time_state);

            XStoreName(dpy, DefaultRootWindow(dpy), status_line);
            XSync(dpy, False);
        } sleep(1);
    }

    cleanup:
        syslog(LOG_INFO, "Terminating\n");
        XCloseDisplay(dpy);
        closelog();
        return status;
}

// vim: set ts=4 sw=4 et:
