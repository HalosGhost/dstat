#include "dstat.h"

#define UPDATE_MODULE_AT(mod,ival) \
    do { \
        if ( !(c_time % (ival)) ) { \
            if ( (status = (mod)) != EXIT_SUCCESS ) { \
                goto cleanup; \
            } \
        } \
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
    uint16_t audio_vol [1]   = { 0 };
    char audio_mut [2]       = "%";
    uint8_t bat_capacity [1] = { 0 };
    char bat_state [3]       = "D";
    char time_state [44]     = "00.00 (GMT) | Thursday, 01 January";
    bool stdout_flag         = false;
    char status_line [104]   = "";

    mainloop = pa_mainloop_new();
    ctx = pa_context_new(pa_mainloop_get_api(mainloop), NULL);
    is_muted = 0;

    const char * vos = "hs";
    for ( signed oi = 0, c = getopt_long(argc, argv, vos, os, &oi);
          c != -1; c = getopt_long(argc, argv, vos, os, &oi) ) {
        switch ( c ) {
            case 'h': puts(usage_str); goto cleanup;
            case 's': stdout_flag = true; break;
        }
    }

    if ( !ctx ) {
        syslog(LOG_ERR, "Failed to initialize PulseAudio Context\n");
        status = EXIT_FAILURE;
        goto cleanup;
    }

    pa_stat = pa_context_connect(ctx, NULL, 0, NULL);
    if ( pa_stat ) {
        syslog(LOG_ERR, "Failed to connect to PulseAudio Context\n");
        status = EXIT_FAILURE;
        goto cleanup;
    }

    pa_context_set_state_callback(ctx, ctx_state_cb, NULL);

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
        pa_mainloop_iterate(mainloop, 0, &pa_stat);
        UPDATE_MODULE_AT(get_en_state(en_state), EN_INTERVAL);
        UPDATE_MODULE_AT(get_wl_strength(wl_strength), WL_INTERVAL);
        UPDATE_MODULE_AT(get_aud_volume(audio_vol), VL_INTERVAL);
        UPDATE_MODULE_AT(get_aud_mute(audio_mut), VL_INTERVAL);
        UPDATE_MODULE_AT(get_bat_cap(bat_capacity), BT_INTERVAL);
        UPDATE_MODULE_AT(get_bat_state(bat_state), BT_INTERVAL);
        UPDATE_MODULE_AT(get_time_state(time_state), TM_INTERVAL);

        if ( !(c_time % PT_INTERVAL) ) {
            snprintf(status_line, 104, LNFMT,
                    en_state,
                    wl_bars[*wl_strength],
                    *audio_vol, audio_mut,
                    *bat_capacity, bat_state,
                    time_state, STCHR);

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

        pa_context_unref(ctx);
        pa_mainloop_free(mainloop);

        if ( dpy ) {
            XCloseDisplay(dpy);
        } else {
            printf("\x1b[?25h");
        }

        closelog();
        return status;
}

void
signal_handler (signed signum) {

    syslog(LOG_INFO, "Caught %s; Terminating\n", sys_siglist[signum]);

    pa_context_unref(ctx);
    pa_mainloop_free(mainloop);

    if ( dpy ) {
        XCloseDisplay(dpy);
    } else {
        fputs("\x1b[?25h", stdout);
    }

    closelog();
    exit(EXIT_SUCCESS);
}

#define check_null_arg(ptr) \
    do { \
        if ( !(ptr) ) { \
            syslog(LOG_ERR, "%s() was passed a NULL pointer\n", __func__); \
            exit(EXIT_FAILURE); \
        } \
    } while ( false )

void
dump_sink_info (pa_context * c, const pa_sink_info * i, signed e, void * u) {

    (void )c; (void )e; (void )u;
    syslog(LOG_INFO, "Updating audio values\n");

    if ( i ) {
        cvol = i->volume.values[0];
        is_muted = i->mute;
    }
}

void
ctx_state_cb (pa_context * c, void * udata) {

    (void )udata;

    if ( !c ) {
        syslog(LOG_ERR, "Tried to check state of a NULL context\n");
        exit(EXIT_FAILURE);
    }

    pa_operation * op = NULL;

    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME: break;

        case PA_CONTEXT_READY:
            op = pa_context_get_sink_info_by_index(c, PASNK, dump_sink_info, NULL);
            if ( !op ) { syslog(LOG_ERR, "Failed to query sink info\n"); }
            break;

        case PA_CONTEXT_TERMINATED:
            syslog(LOG_INFO, "PulseAudio Context was terminated\n");
            break;

        case PA_CONTEXT_FAILED:
            syslog(LOG_ERR, "Failed to initialize PulseAudio Context: %s\n", pa_strerror(pa_context_errno(c)));
            exit(EXIT_FAILURE);
    }
}


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

    *strength = !n ? n : (n / 10) + (n < 7);
    return EXIT_SUCCESS;
}

signed
get_aud_volume (uint16_t * volume) {

    check_null_arg(volume);
    *volume = (uint16_t )(cvol * 100 / 65535);
    return EXIT_SUCCESS;
}

signed
get_aud_mute (char * mute) {

    check_null_arg(mute);
    *mute = is_muted ? 'M' : '%';
    return EXIT_SUCCESS;
}

signed
get_bat_cap (uint8_t * capacity) {

    check_null_arg(capacity);

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
        fclose(in);
        return EXIT_FAILURE;
    } fclose(in); return EXIT_SUCCESS;
}

signed
get_bat_state (char * state) {

    check_null_arg(state);

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
        fclose(in);
        return EXIT_FAILURE;
    } fclose(in);

    snprintf(state, sizeof "ϟ", "%s", st == 'C' ? "ϟ" : "D");
    return EXIT_SUCCESS;
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
