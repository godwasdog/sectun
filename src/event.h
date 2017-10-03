//
// Created by Secrect on 2017-09-17.
//

#ifndef SECTUN_EVENT_H
#define SECTUN_EVENT_H

#include "3rd/libuev/uev.h"

/**
 * 用面向对象来封装调用
 *
 * 这里我们实现了 singleton 模式，整个系统只有一个 event 的 instance，这样使用起来比较方便和安全
 *
 * 具体的使用方法见 uev.h 的说明
 *
 * */


typedef struct {

    int (*run)(int flags);

    int (*exit)();

    int (*io_init)(uev_t *w, uev_cb_t *cb, void *arg, int fd, int events);

    int (*io_set)(uev_t *w, int fd, int events);

    int (*io_start)(uev_t *w);

    int (*io_stop)(uev_t *w);

    int (*timer_init)(uev_t *w, uev_cb_t *cb, void *arg, int timeout, int period);

    int (*timer_set)(uev_t *w, int timeout, int period);

    int (*timer_start)(uev_t *w);

    int (*timer_stop)(uev_t *w);

    int (*cron_init)(uev_t *w, uev_cb_t *cb, void *arg, time_t when, time_t interval);

    int (*cron_set)(uev_t *w, time_t when, time_t interval);

    int (*cron_start)(uev_t *w);

    int (*cron_stop)(uev_t *w);

    int (*signal_init)(uev_t *w, uev_cb_t *cb, void *arg, int signo);

    int (*signal_set)(uev_t *w, int signo);

    int (*signal_start)(uev_t *w);

    int (*signal_stop)(uev_t *w);

} sectun_event_t;


/**
 *
 * 返回 singleton 的实例，常量不可更改
 *
 * */
const sectun_event_t *const sectunGetEventInstance();


#endif //SECTUN_EVENT_H
