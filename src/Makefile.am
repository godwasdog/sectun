bin_PROGRAMS = sectun

lib_LTLIBRARIES = libsectun.la

libsectun_la_SOURCES = \
        3rd/aes/aes.c \
        3rd/chacha20/chacha20.c \
        3rd/ikcp/ikcp.c \
        3rd/jsmn/jsmn.c \
        3rd/libuev/uev.c 3rd/libuev/timer.c 3rd/libuev/signal.c 3rd/libuev/io.c 3rd/libuev/cron.c \
        3rd/md5/md5.c \
        args.c auth.c encrypt.c event.c heartbeat.c itransport.c kcp.c log.c shell.c tun.c udp.c util.c

sectun_SOURCES = main.c
sectun_LDADD = libsectun.la
