#pragma once

#include <fstream>
#include <string>

#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>

namespace config {
    using namespace google::protobuf;

    inline void read(const char *filename, Message *message) {
        std::ifstream ifs(filename);
        std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
        util::JsonStringToMessage(content, message);
    }
}
