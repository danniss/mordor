#ifndef __MORDOR_REDIS_EXCEPTION_H__
#define __MORDOR_REDIS_EXCEPTION_H__


#include "mordor/exception.h"

namespace Mordor {
namespace Redis {

struct Exception : virtual ::Mordor::Exception
{
    Exception(const std::string &message)
        : m_message(message)
    {}
    Exception()
    {}
    ~Exception() throw () {}

    const char *what() const throw () { return m_message.c_str(); }
private:
    std::string m_message;
};

#define DEFINE_MORDOR_REDIS_EXCEPTION(Exception, Base)                             \
struct Exception : Base                                                         \
{                                                                               \
    Exception()                                                                 \
    {}                                                                          \
    Exception(const std::string &message)                                       \
        : Base(message)                                                         \
    {}                                                                          \
};

DEFINE_MORDOR_REDIS_EXCEPTION(ConnectionException, Exception);

DEFINE_MORDOR_REDIS_EXCEPTION(DataException, Exception);

void throwException(const std::string& message);

}}

#endif
