#include "exception.h"

#include "mordor/log.h"

namespace Mordor {
namespace Redis {

static Logger::ptr g_log = Log::lookup("mordor:redis");

void throwException(const std::string& message)
{
    MORDOR_LOG_ERROR(g_log) << " connection error: " << message;
    MORDOR_THROW_EXCEPTION(ConnectionException(message));
}

}}
