#ifndef __MORDOR_REDIS_CONNECTION_H__
#define __MORDOR_REDIS_CONNECTION_H__

#include <sstream>

#include <boost/noncopyable.hpp>
#include <boost/weak_ptr.hpp>

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include "reply.h"

namespace Mordor {

struct IOManager;
struct Fiber;

namespace Redis {

class Connection : boost::noncopyable {

public:

    Connection(const std::string& host, unsigned short port, Mordor::IOManager* ioManager)
        : m_host(host), m_port(port), m_ioManager(ioManager)
    {
    }

    void connect();

    #define KEY_VALUE_REDIS_FUNC(command) \
    bool command(const char* key, const char* value) {\
        std::stringstream ss;\
        ss << #command << ' ' << key << ' ' << value;\
        return sendCommand(ss.str().c_str());} 


    #define KEY_REDIS_FUNC(command) \
    bool command(const char* key) {\
        std::stringstream ss;\
        ss << #command << ' ' << key;\
        return sendCommand(ss.str().c_str());} 


    #define STRING_BY_KEY_REDIS_FUNC(command) \
    bool command(const char* key, std::string& result) {\
        std::stringstream ss;\
        ss << #command << ' ' << key;\
        return sendCommand(ss.str().c_str(), result);\
    }

    #define INT_BY_KEY_REDIS_FUNC(command) \
    bool command(const char* key, int& result) {\
        std::stringstream ss;\
        ss << #command << ' ' << key;\
        return sendCommand(ss.str().c_str(), result);\
    }

    #define INT_BY_KEY_VALUE_REDIS_FUNC(command) \
    bool command(const char* key, const char* value, int& result) {\
        std::stringstream ss;\
        ss << #command << ' ' << key << ' ' << value;\
        return sendCommand(ss.str().c_str(), result);\
    }

    #define BOOL_BY_KEY_VALUE_REDIS_FUNC(command) \
    bool command(const char* key, const char* value, bool& result) {\
        std::stringstream ss;\
        ss << #command << ' ' << key << ' ' << value;\
        return sendCommand(ss.str().c_str(), result);\
    }

    #define ARRAY_BY_KEY_REDIS_FUNC(command) \
    bool command(const char* key, std::list<std::string>& values) {\
        std::stringstream ss;\
        ss << #command << ' ' << key;\
        return sendCommand(ss.str().c_str(), values);\
    }

    #define ARRAY_BY_TWO_KEYS_REDIS_FUNC(command) \
    bool command(const char* key1, const char* key2, std::list<std::string>& values) {\
        std::stringstream ss;\
        ss << #command << ' ' << key1 << ' ' << key2;\
        return sendCommand(ss.str().c_str(), values);\
    }

    #define STRING_BY_KEY_FIELD_REDIS_FUNC(command) \
    bool command(const char* key, const char* field, std::string& result) {\
        std::stringstream ss;\
        ss << #command << ' ' << key << ' ' << field;\
        return sendCommand(ss.str().c_str(), result);\
    }

    STRING_BY_KEY_REDIS_FUNC(get);
    KEY_VALUE_REDIS_FUNC(set);
    KEY_VALUE_REDIS_FUNC(setnx);
    KEY_REDIS_FUNC(del);
    INT_BY_KEY_REDIS_FUNC(incr);
    INT_BY_KEY_REDIS_FUNC(decr);
    KEY_VALUE_REDIS_FUNC(append);
    bool getInt(const char* key, int& value);
    bool getLong(const char* key, long& value);
    bool getSet(const char* key, const char* value, int& oldValue);
    bool incrBy(const char* key, int count, int& value);
    bool decrBy(const char* key, int count, int& value);
    bool setex(const char* key, int expireSeconds, const char* value);
    bool mget(std::list<std::shared_ptr<std::string> >& values, int number, ...);


    /*
    ** declare functions for list related commands
    */
    KEY_VALUE_REDIS_FUNC(lpush);
    KEY_VALUE_REDIS_FUNC(lpushx);
    KEY_VALUE_REDIS_FUNC(rpush);
    KEY_VALUE_REDIS_FUNC(rpushx);
    KEY_VALUE_REDIS_FUNC(rpoplpush);
    STRING_BY_KEY_REDIS_FUNC(lpop);
    STRING_BY_KEY_REDIS_FUNC(rpop);
    bool lrange(const char* key, int start, int stop, std::list<std::string>& values);
    INT_BY_KEY_REDIS_FUNC(llen);
    bool lrem(const char* key, int position);
    bool lset(const char* key, int position, const char* value);
    bool linsert(const char* key, int position, const char* value);
    bool lindex(const char* key, const char* value, int& position);
    bool ltrim(const char* key, int start, int stop);
    // timout in seconds, return false if not exist before timeout
    bool blpop(const char* key, int timeout, std::string& value);
    bool brpop(const char* key, int timeout, std::string& value);

    /*
    ** declare functions for set related commands
    */
    KEY_VALUE_REDIS_FUNC(sadd);
    INT_BY_KEY_REDIS_FUNC(scard);
    ARRAY_BY_KEY_REDIS_FUNC(smembers);
    STRING_BY_KEY_REDIS_FUNC(spop);
    BOOL_BY_KEY_VALUE_REDIS_FUNC(sismember);
    KEY_VALUE_REDIS_FUNC(srem);
    ARRAY_BY_TWO_KEYS_REDIS_FUNC(sunion);
    ARRAY_BY_TWO_KEYS_REDIS_FUNC(sdiff);
    ARRAY_BY_TWO_KEYS_REDIS_FUNC(sinter);

    /*
    ** declare functions for hash related commands
    */
    KEY_VALUE_REDIS_FUNC(hdel);
    BOOL_BY_KEY_VALUE_REDIS_FUNC(hexists);
    STRING_BY_KEY_FIELD_REDIS_FUNC(hexists);
    ARRAY_BY_KEY_REDIS_FUNC(hgetall);
    ARRAY_BY_KEY_REDIS_FUNC(hkeys);
    ARRAY_BY_KEY_REDIS_FUNC(hvals);
    INT_BY_KEY_REDIS_FUNC(hlen);

    /*
    ** declare functions for sorted set related commands
    */
    INT_BY_KEY_REDIS_FUNC(zcard);
    bool zadd(const char* key, float score, const char* member);
    bool zcount(const char* key, float start, float stop, int& count);
    bool zremRangeByRank(const char* key, float start, float stop);
    bool zrange(const char* key, float start, float stop, std::list<std::string>& values);
    INT_BY_KEY_VALUE_REDIS_FUNC(zrank);
    KEY_VALUE_REDIS_FUNC(zrem);
    bool zscore(const char* key, const char* member, float& score);
    
private:
    void static connectCallback(const redisAsyncContext *c, int status);

    void static disconnectCallback(const redisAsyncContext *c, int status);

    void static read(redisAsyncContext *c, void *reply, void *privdata);

    bool sendCommand(const char* command);
    bool sendCommand(const char* command, std::string& response);
    bool sendCommand(const char* command, int& response);
    bool sendCommand(const char* command, bool& response);
    bool sendCommand(const char* command, std::list<std::string>& values);

    bool bpop(const char* key, int timeout, const char* command, std::string& value);
private:
    std::string m_host;
    unsigned short m_port;
    Mordor::IOManager* m_ioManager;
    redisAsyncContext* m_ac;
    boost::weak_ptr<Mordor::Fiber> m_stallingFiber;
    boost::shared_ptr<Reply> m_reply;
};

}}

#endif
