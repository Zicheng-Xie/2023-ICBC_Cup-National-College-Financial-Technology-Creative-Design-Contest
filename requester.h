// requester.h
#pragma once

#include <Poco/JSON/Object.h>
#include <Poco/Dynamic/Var.h>

/// Simple requester interface used by main.cpp
/// In your real project, you can replace the implementation
/// in requester.cpp with real HTTP/Socket logic.
class Requester {
public:
    /// Initialize network / connections.
    /// In the demo implementation this just prints a message.
    void run();

    /// Wait for one incoming "HTTP request" and return it
    /// as a JSON object.
    ///
    /// Demo implementation:
    ///   - Read one line from std::cin
    ///   - Parse it as JSON
    ///   - Return JSON::Object::Ptr
    ///
    /// If parsing fails, returns a nullptr.
    Poco::JSON::Object::Ptr httprequest();

    /// Send a "check" request to other nodes and wait for the
    /// consensus result.
    ///
    /// Return value format (demo):
    ///   {
    ///     "answer": true/false
    ///   }
    ///
    /// In a real distributed environment, you should:
    ///   - broadcast `req` to other nodes,
    ///   - collect responses,
    ///   - summarize to a single boolean "answer".
    Poco::JSON::Object::Ptr send_check(Poco::JSON::Object::Ptr req);

    /// Send back the local check result to whoever asked us.
    ///
    /// Demo implementation just prints the result.
    void send_checkans(bool ok);
};

/// Global requester instance used by main.cpp
extern Requester requester;
