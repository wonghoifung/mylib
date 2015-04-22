#include "curl/curl.h"
#include "curl/easy.h"
#include "json/json.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

std::string JsonEncode(const Json::Value& data) {
    Json::FastWriter writer;
    return writer.write(data);
}

int writer(char* data, size_t size, size_t nmemb, std::string* writerData) {
    unsigned long sizes = size * nmemb;

    if (writerData == NULL) 
    {
        return 0;
    }
    writerData->append(data, sizes);
    return sizes;
}

int atoi_s(const std::string& str) {
    return str.empty() ? 0 : atoi(str.c_str());
}

int32_t JsonToInt(const Json::Value &object,const char* str)
{
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

int callphp() {
    CURL* curl;

    Json::Value param;
    param["method"] = "test";
    param["id"] = 123;
    param["param"] = 456;
    param["key"] = "xxxxxxxxx";
    char curl_data[1024] = {0};
    snprintf(curl_data,1024,"",JsonEncode(param).c_str());

    std::string retbuffer;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    curl_easy_setopt(curl,CURLOPT_URL,"http://www.baidu.com");
    curl_easy_setopt(curl,CURLOPT_POST,1);
    curl_easy_setopt(curl,CURLOPT_COPYPOSTFIELDS,curl_data);
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,writer);
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,&retbuffer);
    curl_easy_setopt(curl,CURLOPT_USERAGENT,"libcurl-agent/1.0");
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    if (retbuffer.find("ret")==std::string::npos) {
        printf("no ret in retbuffer:\"%s\"\n",retbuffer.c_str());
        return -1;
    }

    Json::Reader reader;
    Json::Value json_object;
    if (!reader.parse(retbuffer,json_object)) {
        printf("reader parse error\n");
        return -1;
    }

    if (json_object.size() < 1) {
        printf("json object size less than 1\n");
        return -1;
    }

    int ret = JsonToInt(json_object,"ret");
    if (ret!=0) {
        printf("php return %d\n",ret);
        return -1;
    }

    printf("php return 0, success\n",ret);
    return 0;
}

int main() {
    callphp();
    return 0;
}

