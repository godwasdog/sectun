//
// Created by Secrect on 2017-09-17.
//

#include <string.h>

#include "inc.h"
#include "event.h"

/**
 * single event context
 * */
static uev_ctx_t _eventCtx;

static int sectun_event_run(int flags) {
    return uev_run(&_eventCtx, flags);
}

static int sectun_event_exit() {
    return uev_exit(&_eventCtx);
}

static int sectun_event_io_init(uev_t *w, uev_cb_t *cb, void *arg, int fd, int events) {
    return uev_io_init(&_eventCtx, w, cb, arg, fd, events);
}

static int sectun_event_timer_init(uev_t *w, uev_cb_t *cb, void *arg, int timeout, int period) {
    return uev_timer_init(&_eventCtx, w, cb, arg, timeout, period);
}

static int sectun_event_cron_init(uev_t *w, uev_cb_t *cb, void *arg, time_t when, time_t interval) {
    return uev_cron_init(&_eventCtx, w, cb, arg, when, interval);
}

static int sectun_event_signal_init(uev_t *w, uev_cb_t *cb, void *arg, int signo) {
    return uev_signal_init(&_eventCtx, w, cb, arg, signo);
}


/**
 * static object used as a singleton object
 * */
static sectun_event_t _eventObj;
static sectun_event_t *pEventObject = NULL;

/**
 * return singleton object
 * */
const sectun_event_t *const sectunGetEventInstance() {

    if (NULL != pEventObject) {
        goto out;
    }

    // init event context
    uev_init(&_eventCtx);

    memset(&_eventObj, 0, sizeof(sectun_event_t));
    pEventObject = &_eventObj;

    pEventObject->run = sectun_event_run;
    pEventObject->exit = sectun_event_exit;

    pEventObject->io_init = sectun_event_io_init;
    pEventObject->io_set = uev_io_set;
    pEventObject->io_start = uev_io_start;
    pEventObject->io_stop = uev_io_stop;

    pEventObject->timer_init = sectun_event_timer_init;
    pEventObject->timer_set = uev_timer_set;
    pEventObject->timer_start = uev_timer_start;
    pEventObject->timer_stop = uev_timer_stop;

    pEventObject->cron_init = sectun_event_cron_init;
    pEventObject->cron_set = uev_cron_set;
    pEventObject->cron_start = uev_cron_start;
    pEventObject->cron_stop = uev_cron_stop;

    pEventObject->signal_init = sectun_event_signal_init;
    pEventObject->signal_set = uev_signal_set;
    pEventObject->signal_start = uev_signal_start;
    pEventObject->signal_stop = uev_signal_stop;

    out:
    return pEventObject;
}

