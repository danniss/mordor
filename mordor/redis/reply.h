#ifndef __MORDOR_REDIS_REPLY_H__
#define __MORDOR_REDIS_REPLY_H__

#include <list>
#include <string>
#include <boost/shared_ptr.hpp>

struct redisReply;

namespace Mordor {
namespace Redis {

enum ReplyType {
    REPLY_STRING,
    REPLY_ARRAY,
    REPLY_INTEGER,
    REPLY_NIL,
    REPLY_STATUS,
    REPLY_ERROR
};


class Reply {
public:
    typedef boost::shared_ptr<Reply> ptr;
    virtual enum ReplyType getReplyType() = 0;
};

class ErrorReply : public Reply {
public:
    ErrorReply(const char* message) : m_message(message) {}

    virtual enum ReplyType getReplyType() { return REPLY_ERROR; }

    std::string getError() { return m_message; }

private:
    std::string m_message;
};

class StringReply : public Reply {
public:
    StringReply(const char* content) : m_content(content) {}

    virtual enum ReplyType getReplyType() { return REPLY_STRING; }

    std::string getString() { return m_content; }

private:
    std::string m_content;
};

class NilReply : public Reply {
public:
    virtual enum ReplyType getReplyType() { return REPLY_NIL; }
};

class IntegerReply : public Reply {
public:
    IntegerReply(int value) : m_value(value) {}

    virtual enum ReplyType getReplyType() { return REPLY_INTEGER; }

    int getInteger() { return m_value; }

private:
    int m_value;
};


class ArrayReply : public Reply {
public:
    ArrayReply(redisReply **element, int size);

    virtual enum ReplyType getReplyType() { return REPLY_ARRAY; }

    std::list<Reply::ptr> getReplies() { return m_replies; }

private:
    std::list<boost::shared_ptr<Reply> > m_replies;
};


class StatusReply : public Reply {
public:

    virtual enum ReplyType getReplyType() { return REPLY_STATUS; }
};


class ReplyFactory {
public:
    static boost::shared_ptr<Reply> create(redisReply* redisReply);
};


}}

#endif
