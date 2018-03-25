#ifndef __HIREDIS_LIBMORDOR_H__
#define __HIREDIS_LIBMORDOR_H__
#include <mordor/iomanager.h>
#include <boost/bind.hpp>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>

using namespace Mordor;

namespace Mordor {
namespace Redis {
    struct Connection;
}}

typedef struct redisMordorEvents {
    redisAsyncContext *context;
    IOManager *ioManager;
    int fd;
    Mordor::Redis::Connection* conn;
} redisMordorEvents;

static void redisMordorReadEvent(redisMordorEvents *e) {
    redisAsyncHandleRead(e->context);
}

static void redisMordorWriteEvent(redisMordorEvents *e) {
    redisAsyncHandleWrite(e->context);
}

static void redisMordorAddRead(void *privdata) {
    redisMordorEvents *e = (redisMordorEvents*)privdata;
    e->ioManager->registerEvent(e->fd, IOManager::READ, boost::bind(&redisMordorReadEvent, e));
}

static void redisMordorDelRead(void *privdata) {
    redisMordorEvents *e = (redisMordorEvents*)privdata;
    e->ioManager->unregisterEvent(e->fd, IOManager::READ);
}

static void redisMordorAddWrite(void *privdata) {
    redisMordorEvents *e = (redisMordorEvents*)privdata;
    e->ioManager->registerEvent(e->fd, IOManager::WRITE, boost::bind(&redisMordorWriteEvent, e));
}

static void redisMordorDelWrite(void *privdata) {
    redisMordorEvents *e = (redisMordorEvents*)privdata;
    e->ioManager->unregisterEvent(e->fd, IOManager::WRITE);
}

static void redisMordorCleanup(void *privdata) {
    redisMordorEvents *e = (redisMordorEvents*)privdata;
    free(e);
}

static int redisMordorAttach(redisAsyncContext *ac, IOManager* ioManager, Mordor::Redis::Connection* conn) {
    redisContext *c = &(ac->c);
    redisMordorEvents *e;

    /* Nothing should be attached when something is already attached */
    if (ac->ev.data != NULL)
        return REDIS_ERR;

    /* Create container for context and r/w events */
    e = (redisMordorEvents*)malloc(sizeof(*e));
    e->context = ac;
    e->ioManager = ioManager;
    e->fd = c->fd;
    e->conn = conn;

    /* Register functions to start/stop listening for events */
    ac->ev.addRead = redisMordorAddRead;
    ac->ev.delRead = redisMordorDelRead;
    ac->ev.addWrite = redisMordorAddWrite;
    ac->ev.delWrite = redisMordorDelWrite;
    ac->ev.cleanup = redisMordorCleanup;
    ac->ev.data = e;

    return REDIS_OK;
}
#endif
