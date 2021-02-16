#pragma once

#include <fstream>
#include <string>

#pragma warning(push, 0)
#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>
#pragma warning(pop)

namespace config {
    using namespace google::protobuf;

    inline void read(const char *filename, Message *message) {
        std::ifstream ifs(filename);
        std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
        util::JsonStringToMessage(content, message);
    }
}
