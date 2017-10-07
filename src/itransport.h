/**
 * Created by YuQiang on 2017-09-22.
 *
 * define transport layer interface, we can implement transport interface by udp,icmp,...
 *
 *
 */


#ifndef ITRANSPORT_H
#define ITRANSPORT_H

#include <stdlib.h>

struct itransport {

    /**************************************************************************************
     *
     *                  这里的函数是由模块自己提供，提供给别的模块调用
     *
     * ************************************************************************************/

    /**
     * @return the real bytes that read
     *
     * @param buffer buffer to store data
     * @param len  data len
     */
    ssize_t (*readData)(char *buffer, size_t len, void *context);

    /**
     * @return the real bytes that write
     *
     * @param buffer
     * @param len
     */
    ssize_t (*writeData)(char *buffer, size_t len, void *context);

    /**
     *  start the transport
     */
    int (*start)();

    /**
     * stop the transport
     */
    int (*stop)();

    /**
     *  set Next Layer Transport
     * @param transport
     */
    void (*setNextLayer)(struct itransport *transport);

    /**************************************************************************************
     *
     *        这里的函数是由别的模块提供，由当前模版调用，用于模块之间相互通讯
     *
     * ************************************************************************************/

    /**
     *  当前模块读取到数据，用forwardRead 把数据传输给下一个模块
     *
     * @param buffer
     * @param len
     */
    ssize_t (*forwardRead)(char *buffer, size_t len, void *context);

    /**
     *  当前模版把数据 fowardRead 给下一个模块，数据传完之后调用这个方法，
     *  告诉下一个模块数据结束
     *
     * @param totalLen
     */
    ssize_t (*forwardReadFinish)(size_t totalLen, void *context);


    /**
     *  当前模块写数据，用forwardWrite 把数据传输给下一个模块
     *
     * @param buffer
     * @param len
     */
    ssize_t (*forwardWrite)(char *buffer, size_t len, void *context);

    /**
     *  当前模版把数据 fowardWrite 给下一个模块，数据传完之后调用这个方法，
     *  告诉下一个模块数据结束
     *
     * @param totalLen
     */
    ssize_t (*forwardWriteFinish)(size_t totalLen, void *context);

};


extern struct itransport __dummyTransport;


#endif //ITRANSPORT_H
