// main.cpp
#include <iostream>
#include <filesystem>

#include "BlockTree.h"
#include "ResourceManager.h"

// Use Poco JSON as the JSON library (your original pseudocode closely resembles Poco's style)
#include <Poco/JSON/Object.h>
#include <Poco/Dynamic/Var.h>

#include "requester.h"  // Use your existing network interface

namespace JSON   = Poco::JSON;
namespace Dynamic = Poco::Dynamic;

// ---------------- JSON → CandidateBlock Parsing ----------------

CandidateBlock fromJson(const JSON::Object::Ptr& obj) {
    CandidateBlock b;

    // id: Globally unique identifier, e.g., "001-01-02"
    b.id       = obj->optValue<std::string>("id", "");
    b.parentId = obj->optValue<std::string>("parent_id", "root");

    b.index     = obj->optValue<int>("index", 0);

    long long ts   = obj->optValue<long long>("timestamp", 0);
    long long rand = obj->optValue<long long>("rand", 0);

    b.timestamp = static_cast<std::int64_t>(ts);
    b.nonce     = static_cast<std::uint32_t>(rand);

    b.name      = obj->optValue<std::string>("name", "");
    std::string elePath = obj->optValue<std::string>("ele", "");
    b.filePath  = std::filesystem::path(elePath);

    std::string clazz = obj->optValue<std::string>("class", "big");
    b.cls = NodeClassFromString(clazz);

    b.hash = obj->optValue<std::string>("hash", "");

    return b;
}

// ---------------- Local Mining (BlockTree-only) ----------------

bool localMiner(const JSON::Object::Ptr& obj, BlockTree& tree) {
    CandidateBlock block = fromJson(obj);
    return tree.miner(block);
}

// ---------------- Consensus Interaction with Other Nodes ----------------

// Assume requester.send_check(...) accepts a JSON and returns a JSON
bool checkWithOthers(const JSON::Object::Ptr& obj) {
    JSON::Object::Ptr resp = requester.send_check(obj);
    if (!resp) return false;

    return resp->optValue<bool>("answer", false);
}

// ---------------- Complete Process for Adding a New Block ----------------

void handleAdd(const JSON::Object::Ptr& obj,
               BlockTree& tree,
               ResourceManager& resMgr) {
    CandidateBlock block = fromJson(obj);

    // 1. Local mining verification
    if (!tree.miner(block)) {
        std::cerr << "[handleAdd] local miner failed, reject block id="
                  << block.id << '\n';
        return;
    }

    // 2. Consortium chain / other nodes consensus
    if (!checkWithOthers(obj)) {
        std::cerr << "[handleAdd] remote check failed, reject block id="
                  << block.id << '\n';
        return;
    }

    // 3. Write to tree-structured blockchain
    auto node = tree.addNode(block);

    // 4. Register resource (hand over to ResourceManager for game integration)
    resMgr.registerNode(node);

    std::cout << "[handleAdd] block accepted, id=" << node->id
              << ", hash=" << node->hash << '\n';
}

// ---------------- Event Loop Entry: main ----------------

int main() {
    // Block tree + resource manager
    BlockTree       tree;
    ResourceManager resMgr(std::filesystem::path("objects")); 
    // "objects" directory as resource root, adjustable based on actual needs

    // Start network
    requester.run();

    while (true) {
        JSON::Object::Ptr request = requester.httprequest();
        if (!request) continue;

        // Convention: The request contains a field "mode"
        std::string mode = request->optValue<std::string>("mode", "add");

        if (mode == "add") {
            // Frontend / other node request: Add a new block
            handleAdd(request, tree, resMgr);

        } else if (mode == "check") {
            // Other node query: Help verify if this block is valid
            bool ok = localMiner(request, tree);
            requester.send_checkans(ok);

        } else if (mode == "view_node") {
            // Client only wants to view resources corresponding to a building / room / tiny object
            std::string id = request->optValue<std::string>("id", "");
            if (!id.empty()) {
                resMgr.ensureLoadedForView(id, tree);
            }
            // If you want to send a "resource ready" response to the frontend,
            // you can define an additional interface like send_view_result(...) in requester.h

        } else {
            std::cerr << "[main] unknown mode: " << mode << '\n';
        }
    }

    return 0;
}