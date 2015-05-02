#include "json/json.h"
#include <stdio.h>
#include <string>
#include <map>
#include <hiredis/hiredis.h> 

std::string JsonEncode(const Json::Value& data) {
    Json::FastWriter writer;
    return writer.write(data);
}

int atoi_s(const std::string& str) {
    return str.empty() ? 0 : atoi(str.c_str());
}

int32_t JsonToInt(const Json::Value& object,const char* str) {
    if(object.isNull() || object.type() != Json::objectValue)
        return -1;

    if(object.isMember(str) == false)
        return -1;

    if(object[str].type() == Json::intValue)
        return object[str].asInt();
    else if (object[str].type() == Json::stringValue)
        return atoi_s(object[str].asString());
    else
        return -1;
}

bool writeToRedis(const std::string& key, const std::string& value) {
    redisContext* c = redisConnect("127.0.0.1", 6379);
    if (c->err) {
        redisFree(c);
        return false;
    }
    char* command = (char*)malloc(128 + key.size() + value.size());
    sprintf(command, "set %s %s", key.c_str(), value.c_str());
    printf("command:%s\n",command);
    redisReply* r = (redisReply*)redisCommand(c, command);
    if (NULL==r) {
        redisFree(c);
        free(command);
        return false;
    }
    if( !(r->type == REDIS_REPLY_STATUS && strcasecmp(r->str,"OK")==0)) {  
        freeReplyObject(r);  
        redisFree(c);  
        free(command);
        return false;  
    } 
    freeReplyObject(r);
    redisFree(c);
    free(command);
    return true; 
}

std::pair<std::string,bool> readFromRedis(const std::string& key) {
    redisContext* c = redisConnect("127.0.0.1", 6379);
    if (c->err) {
        redisFree(c);
        printf("cannot get context\n");
        return std::make_pair("",false);
    }
    char* command = (char*)malloc(128 + key.size());
    sprintf(command, "get %s", key.c_str());
    printf("command:%s\n",command);
    redisReply* r = (redisReply*)redisCommand(c, command);
    if (NULL==r) {
        redisFree(c);
        free(command);
        printf("cannot get reply\n");
        return std::make_pair("",false);
    }
    if (r->type != REDIS_REPLY_STRING) {
        freeReplyObject(r);  
        redisFree(c);  
        free(command);
        printf("cannot get REPLY NIL\n");
        return std::make_pair("",false);
    } 
    std::string str = r->str;
    freeReplyObject(r);
    redisFree(c);
    free(command);
    return std::make_pair(str,true);
}

int main() {
    Json::Value param;
    param["method"] = "test";
    param["id"] = 123;
    param["param"] = 456;
    param["key"] = "xxxxxxxxx";
    char curl_data[1024] = {0};
    snprintf(curl_data,1024,"%s",JsonEncode(param).c_str());
    printf("%s\n", curl_data);

    writeToRedis("testkey", curl_data);
    std::pair<std::string,bool> ret = readFromRedis("testkey");
    std::string jsondata = ret.first;

    Json::Reader reader;
    Json::Value json_object;
    if (!reader.parse(jsondata,json_object)) {
        printf("reader parse error, [%s]\n",jsondata.c_str());
        return -1;
    }    

    printf("after parse:\n");
    printf("method:%s\n",json_object["method"].asString().c_str());
    printf("id:%d\n",JsonToInt(json_object, "id"));
    printf("param:%d\n",JsonToInt(json_object, "param"));
    printf("key:%s\n",json_object["key"].asString().c_str());
    return 0;
}

