#include "reply.h"

#include <iostream>

#include <hiredis/hiredis.h>

namespace Mordor {
namespace Redis {


ArrayReply::ArrayReply(redisReply **element, int size) {
    for (int i = 0; i < size; ++i) {
        m_replies.push_back(ReplyFactory::create(*(element + i)));
    }

}



boost::shared_ptr<Reply>
ReplyFactory::create(redisReply* redisReply) {
    switch (redisReply->type) {
    case REDIS_REPLY_STRING:
        std::cout<<"return string reply"<<std::endl;
        return boost::shared_ptr<Reply>((Reply*)(new StringReply(redisReply->str)));
    case REDIS_REPLY_ERROR:
        std::cout<<"return error reply"<<std::endl;
        return boost::shared_ptr<Reply>((Reply*)(new ErrorReply(redisReply->str)));
    case REDIS_REPLY_INTEGER:
        std::cout<<"return integer reply"<<std::endl;
        return boost::shared_ptr<Reply>((Reply*)(new IntegerReply(redisReply->integer)));
    case REDIS_REPLY_ARRAY:
        std::cout<<"return array reply"<<std::endl;
        return boost::shared_ptr<Reply>((Reply*)(new ArrayReply(redisReply->element, redisReply->elements)));
    case REDIS_REPLY_NIL:
        std::cout<<"return nil reply"<<std::endl;
        return boost::shared_ptr<Reply>((Reply*)(new NilReply()));
    case REDIS_REPLY_STATUS:
        std::cout<<"return status reply"<<std::endl;
        return boost::shared_ptr<Reply>((Reply*)(new StatusReply()));
    }
    std::cout<<"return empty reply"<<std::endl;
    return boost::shared_ptr<Reply>();
}



}}
