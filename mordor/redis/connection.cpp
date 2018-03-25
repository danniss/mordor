#include "connection.h"
#include "exception.h"
#include "libmordor.h"

#include <iostream>
#include <string.h>

#include <boost/lexical_cast.hpp>

#include <hiredis/hiredis.h>

#include <mordor/fiber.h>
#include <mordor/iomanager.h>
#include <mordor/scheduler.h>

namespace Mordor {
namespace Redis {

void
Connection::connect() {
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    m_ac = redisAsyncConnect(m_host.c_str(), m_port);
    if (m_ac == NULL || m_ac->err) {
        std::stringstream ss;
        if (m_ac) {
            ss << "Connection error: " << m_ac->errstr;
            redisAsyncFree(m_ac);
        } else {
            ss << "Connection error: can't allocate redis context";
        }
        throwException(ss.str());
    }
    redisMordorAttach(m_ac, m_ioManager, this);
    redisAsyncSetConnectCallback(m_ac, &Connection::connectCallback);
    redisAsyncSetDisconnectCallback(m_ac, &Connection::disconnectCallback);
    m_stallingFiber = Mordor::Fiber::getThis();
    Mordor::Scheduler::yieldTo();
}


bool
Connection::getInt(const char* key, int& value) {
    std::string str;
    if (!get(key, str)) {
        return false;
    }
    try {
        value = boost::lexical_cast<int>(str.c_str());
    } catch (boost::bad_lexical_cast&) {
        return false;        
    }
    return true;
}

bool
Connection::getLong(const char* key, long& value) {
    std::string str;
    if (!get(key, str)) {
        return false;
    }
    try {
        value = boost::lexical_cast<long>(str.c_str());
    } catch (boost::bad_lexical_cast&) {
        return false;        
    }
    return true;
}

bool
Connection::getSet(const char* key, const char* value, int& oldValue) {
    std::stringstream ss;
    ss << "GETSET " << key << ' ' << value;
    return sendCommand(ss.str().c_str(), oldValue);
}

bool
Connection::incrBy(const char* key, int count, int& value) {
    std::stringstream ss;
    ss << "INCRBY " << key << ' ' << count;
    return sendCommand(ss.str().c_str(), value);
}

bool
Connection::decrBy(const char* key, int count, int& value) {
    std::stringstream ss;
    ss << "DECRBY " << key << ' ' << count;
    return sendCommand(ss.str().c_str(), value);
}

bool
Connection::setex(const char* key, int expireSeconds, const char* value) {
    std::stringstream ss;
    ss << "SETEX " << key << ' ' << expireSeconds << ' ' << value;
    return sendCommand(ss.str().c_str());

}

bool
Connection::mget(std::list<std::shared_ptr<std::string> >& values, int number, ...) {
    va_list valist;
    va_start(valist, number);
    std::stringstream ss;
    ss << "MGET";
    for (int i = 0; i < number; i++)
    {
       ss << ' ' << va_arg(valist, char*);
    }
    va_end(valist);
    redisAsyncCommand(m_ac, &Connection::read, NULL, ss.str().c_str());
    m_stallingFiber = Mordor::Fiber::getThis();
    Mordor::Scheduler::yieldTo();
    if (m_reply->getReplyType() != Mordor::Redis::REPLY_ARRAY) {
        return false;
    }
    std::list<Reply::ptr> replies = boost::dynamic_pointer_cast<ArrayReply>(m_reply)->getReplies();
    std::list<Reply::ptr>::iterator it = replies.begin();
    for (; it != replies.end(); ++it) {
        if ((*it)->getReplyType() != Mordor::Redis::REPLY_STRING) {
            values.push_back(std::shared_ptr<std::string>());
        } else {
            values.push_back(std::shared_ptr<std::string>(new std::string(boost::dynamic_pointer_cast<StringReply>(*it)->getString())));
        }
    }
    return true;
}

bool
Connection::lrange(const char* key, int start, int stop, std::list<std::string>& values) {
    std::stringstream ss;
    ss << "LRANGE " << key << ' ' << start << ' ' << stop;
    redisAsyncCommand(m_ac, &Connection::read, NULL, ss.str().c_str());
    m_stallingFiber = Mordor::Fiber::getThis();
    Mordor::Scheduler::yieldTo();
    if (m_reply->getReplyType() != Mordor::Redis::REPLY_ARRAY) {
        return false;
    }
    std::list<Reply::ptr> replies = boost::dynamic_pointer_cast<ArrayReply>(m_reply)->getReplies();
    std::list<Reply::ptr>::iterator it = replies.begin();
    for (; it != replies.end(); ++it) {
        if ((*it)->getReplyType() != Mordor::Redis::REPLY_STRING) {
            return false;
        }
        values.push_back(boost::dynamic_pointer_cast<StringReply>(*it)->getString());
    }
    return true;
}

bool
Connection::lrem(const char* key, int position) {
    std::stringstream ss;
    ss << "LREM " << key << ' ' << position;
    return sendCommand(ss.str().c_str());
}

bool
Connection::lset(const char* key, int position, const char* value) {
    std::stringstream ss;
    ss << "LSET " << key << ' ' << position << ' ' << value;
    return sendCommand(ss.str().c_str());
}

bool
Connection::linsert(const char* key, int position, const char* value) {
    std::stringstream ss;
    ss << "LINSERT " << key << ' ' << position << ' ' << value;
    return sendCommand(ss.str().c_str());
}

bool
Connection::lindex(const char* key, const char* value, int& position) {
    std::stringstream ss;
    ss << "LINDEX " << key << ' ' << value;
    return sendCommand(ss.str().c_str(), position);
}

bool
Connection::ltrim(const char* key, int start, int stop) {
    std::stringstream ss;
    ss << "LINDEX " << key << ' ' << start << ' ' << stop;
    return sendCommand(ss.str().c_str());
}

bool
Connection::blpop(const char* key, int timeout, std::string& value) {
    return bpop(key, timeout, "BLPOP", value);
}

bool
Connection::brpop(const char* key, int timeout, std::string& value) {
    return bpop(key, timeout, "BRPOP", value);
}

bool
Connection::zadd(const char* key, float score, const char* member) {
    std::stringstream ss;
    ss << "ZADD " << key << ' ' << score << ' ' << member;
    return sendCommand(ss.str().c_str());
}


bool
Connection::zcount(const char* key, float min, float max, int& count) {
    std::stringstream ss;
    ss << "ZADD " << key << ' ' << min << ' ' << max;
    return sendCommand(ss.str().c_str(), count);
}

bool
Connection::zremRangeByRank(const char* key, float start, float stop) {
    std::stringstream ss;
    ss << "ZREMRANGEBYRANK " << key << ' ' << start << ' ' << stop;
    return sendCommand(ss.str().c_str());
}

bool
Connection::zscore(const char* key, const char* member, float& score) {
    std::stringstream ss;
    ss << "ZSCORE " << key << ' ' << member;
    std::string value;
    if (!sendCommand(ss.str().c_str(), value)) {
        return false;
    }
    try {
        score = boost::lexical_cast<float>(value.c_str());
    } catch (boost::bad_lexical_cast&) {
        return false;        
    }
}

bool
Connection::zrange(const char* key, float start, float stop, std::list<std::string>& values) {
    std::stringstream ss;
    ss << "ZRANGE " << key << ' ' << start << ' ' << stop;
    redisAsyncCommand(m_ac, &Connection::read, NULL, ss.str().c_str());
    m_stallingFiber = Mordor::Fiber::getThis();
    Mordor::Scheduler::yieldTo();
    if (m_reply->getReplyType() != Mordor::Redis::REPLY_ARRAY) {
        return false;
    }
    std::list<Reply::ptr> replies = boost::dynamic_pointer_cast<ArrayReply>(m_reply)->getReplies();
    std::list<Reply::ptr>::iterator it = replies.begin();
    for (; it != replies.end(); ++it) {
        if ((*it)->getReplyType() != Mordor::Redis::REPLY_STRING) {
            return false;
        }
        values.push_back(boost::dynamic_pointer_cast<StringReply>(*it)->getString());
    }
    return true;
}


// private functions
void
Connection::connectCallback(const redisAsyncContext *ac, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", ac->errstr);
        return;
    }
    printf("Connected...\n");
    redisMordorEvents* events = (redisMordorEvents*)(ac->ev.data);
    Scheduler::getThis()->schedule(events->conn->m_stallingFiber.lock());
    printf("wake up waiting\n");
}

void
Connection::disconnectCallback(const redisAsyncContext *ac, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", ac->errstr);
        return;
    }
    printf("Disconnected...\n");
}

void
Connection::read(redisAsyncContext *ac, void *reply, void *privdata) {
    printf("reply: %s\n", ((redisReply*)reply)->str);
    redisMordorEvents* events = (redisMordorEvents*)(ac->ev.data);
    Connection* conn = events->conn;
    conn->m_reply = ReplyFactory::create((redisReply*)reply);
    Scheduler::getThis()->schedule(conn->m_stallingFiber.lock());
}

bool
Connection::sendCommand(const char* command) {
    redisAsyncCommand(m_ac, &Connection::read, NULL, command);
    m_stallingFiber = Mordor::Fiber::getThis();
    Mordor::Scheduler::yieldTo();
    return m_reply->getReplyType() != Mordor::Redis::REPLY_ERROR;
}

bool
Connection::sendCommand(const char* command, std::string& response) {
    redisAsyncCommand(m_ac, &Connection::read, NULL, command);
    m_stallingFiber = Mordor::Fiber::getThis();
    Mordor::Scheduler::yieldTo();
    if (m_reply->getReplyType() == Mordor::Redis::REPLY_STRING) {
        response.assign(boost::dynamic_pointer_cast<StringReply>(m_reply)->getString());
        return true;
    } else {
        return false;
    }
}

bool
Connection::sendCommand(const char* command, int& response) {
    redisAsyncCommand(m_ac, &Connection::read, NULL, command);
    m_stallingFiber = Mordor::Fiber::getThis();
    Mordor::Scheduler::yieldTo();
    if (m_reply->getReplyType() == Mordor::Redis::REPLY_INTEGER) {
        response = boost::dynamic_pointer_cast<IntegerReply>(m_reply)->getInteger();
        return true;
    } else {
        return false;
    }
}

bool
Connection::sendCommand(const char* command, bool& response) {
    redisAsyncCommand(m_ac, &Connection::read, NULL, command);
    m_stallingFiber = Mordor::Fiber::getThis();
    Mordor::Scheduler::yieldTo();
    if (m_reply->getReplyType() == Mordor::Redis::REPLY_INTEGER) {
        response = (bool)boost::dynamic_pointer_cast<IntegerReply>(m_reply)->getInteger();
        return true;
    } else {
        return false;
    }
}

bool
Connection::sendCommand(const char* command, std::list<std::string>& values) {
    redisAsyncCommand(m_ac, &Connection::read, NULL, command);
    m_stallingFiber = Mordor::Fiber::getThis();
    Mordor::Scheduler::yieldTo();
    if (m_reply->getReplyType() != Mordor::Redis::REPLY_ARRAY) {
        return false;
    }
    std::list<Reply::ptr> replies = boost::dynamic_pointer_cast<ArrayReply>(m_reply)->getReplies();
    std::list<Reply::ptr>::iterator it = replies.begin();
    for (; it != replies.end(); ++it) {
        if ((*it)->getReplyType() != Mordor::Redis::REPLY_STRING) {
            return false;
        }
        values.push_back(boost::dynamic_pointer_cast<StringReply>(*it)->getString());
    }
    return true;
}

bool
Connection::bpop(const char* key, int timeout, const char* command, std::string& value) {
    std::stringstream ss;
    ss << command << ' ' << key << ' ' << timeout;
    redisAsyncCommand(m_ac, &Connection::read, NULL, ss.str().c_str());
    m_stallingFiber = Mordor::Fiber::getThis();
    Mordor::Scheduler::yieldTo();
    if (m_reply->getReplyType() == Mordor::Redis::REPLY_ARRAY) {
        std::list<Reply::ptr> replies = boost::dynamic_pointer_cast<ArrayReply>(m_reply)->getReplies();
        // this replies should contain key and value
        // so only a list with length 2 is allowed
        if (replies.size() != 2) {
            return false;
        }
        Reply::ptr reply = *(++replies.begin());
        if (reply->getReplyType() != Mordor::Redis::REPLY_STRING) {
            return false;
        }
        value = boost::dynamic_pointer_cast<StringReply>(reply)->getString();
        return true;
    } else {
        return false;
    }
}

}}
