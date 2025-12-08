// requester.cpp
#include "requester.h"

#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Stringifier.h>
#include <Poco/Exception.h>

#include <iostream>
#include <sstream>

using namespace Poco;
using namespace Poco::JSON;
using namespace Poco::Dynamic;

Requester requester; 

void Requester::run() {
    std::cout << "[Requester] run() called. Demo mode: using stdin/stdout as transport."
              << std::endl;
}

Poco::JSON::Object::Ptr Requester::httprequest() {
    std::cout << "[Requester] Waiting for JSON line on stdin..." << std::endl;
    std::string line;
    if (!std::getline(std::cin, line)) {
        std::cerr << "[Requester] EOF or input error." << std::endl;
        return nullptr;
    }

    if (line.empty()) {
        std::cerr << "[Requester] Empty line received." << std::endl;
        return nullptr;
    }

    try {
        JSON::Parser parser;
        Var result = parser.parse(line);

        if (result.type() == typeid(JSON::Object::Ptr)) {
            JSON::Object::Ptr obj = result.extract<JSON::Object::Ptr>();
            return obj;
        } else {
            std::cerr << "[Requester] Parsed JSON is not an object." << std::endl;
            return nullptr;
        }
    } catch (const Poco::Exception& e) {
        std::cerr << "[Requester] JSON parse error: " << e.displayText() << std::endl;
        return nullptr;
    }
}

Poco::JSON::Object::Ptr Requester::send_check(Poco::JSON::Object::Ptr req) {
    // Demo: just print what we "send" and always return answer = true
    std::ostringstream os;
    if (req) {
        req->stringify(os);
        std::cout << "[Requester] send_check(): " << os.str() << std::endl;
    } else {
        std::cout << "[Requester] send_check(): (null request)" << std::endl;
    }

    JSON::Object::Ptr resp = new JSON::Object();
    resp->set("answer", true); 
    return resp;
}

void Requester::send_checkans(bool ok) {
    // Demo: just print the result
    std::cout << "[Requester] send_checkans(): answer = "
              << (ok ? "true" : "false") << std::endl;
}
