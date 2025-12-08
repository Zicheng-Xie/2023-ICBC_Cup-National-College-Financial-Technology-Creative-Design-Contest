// BlockTree.cpp
#include "BlockTree.h"

#include <chrono>
#include <sstream>
#include <iomanip>
#include <functional>
#include <stdexcept>

namespace {

// Current timestamp (in milliseconds)
std::int64_t NowMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()
    ).count();
}

} // anonymous namespace

// ---------------- NodeClass <-> String Conversion ----------------

std::string NodeClassToString(NodeClass cls) {
    switch (cls) {
        case NodeClass::Root:  return "root";
        case NodeClass::Big:   return "big";
        case NodeClass::Child: return "child";
        case NodeClass::Tiny:  return "tiny";
    }
    return "unknown";
}

NodeClass NodeClassFromString(const std::string& s) {
    if (s == "big")   return NodeClass::Big;
    if (s == "child") return NodeClass::Child;
    if (s == "tiny")  return NodeClass::Tiny;
    if (s == "root")  return NodeClass::Root;
    // Default to root to avoid crash
    return NodeClass::Root;
}

// ---------------- BlockTree Implementation ----------------

BlockTree::BlockTree()
    : rng_(std::random_device{}()) {

    // Create unique root node
    root_ = std::make_shared<Node>();
    root_->id        = "root";
    root_->index     = 0;
    root_->timestamp = NowMs();
    root_->nonce     = 0;
    root_->name      = "root";
    root_->filePath  = std::filesystem::path();
    root_->cls       = NodeClass::Root;
    root_->hash      = computeHash(*root_, "");

    nodes_.emplace(root_->id, root_);
}

std::shared_ptr<Node> BlockTree::addNode(const CandidateBlock& block) {
    auto itParent = nodes_.find(block.parentId);
    if (itParent == nodes_.end()) {
        throw std::runtime_error("Parent not found: " + block.parentId);
    }

    auto parent = itParent->second;

    auto node = std::make_shared<Node>();
    node->id        = block.id;
    node->index     = block.index;
    node->timestamp = block.timestamp == 0 ? NowMs() : block.timestamp;
    node->nonce     = block.nonce == 0 ? randomNonce() : block.nonce;
    node->name      = block.name;
    node->filePath  = block.filePath;
    node->cls       = block.cls;
    node->parent    = parent;

    node->hash = computeHash(*node, parent->hash);

    parent->children.push_back(node);
    nodes_.emplace(node->id, node);

    return node;
}

bool BlockTree::miner(const CandidateBlock& block) const {
    auto itParent = nodes_.find(block.parentId);
    if (itParent == nodes_.end()) {
        return false;
    }

    auto parent = itParent->second;

    Node temp;
    temp.id        = block.id;
    temp.index     = block.index;
    temp.timestamp = block.timestamp;
    temp.nonce     = block.nonce;
    temp.name      = block.name;
    temp.filePath  = block.filePath;
    temp.cls       = block.cls;

    const std::string expected = computeHash(temp, parent->hash);
    return expected == block.hash;
}

bool BlockTree::verifyNodeAndAncestors(const std::string& id) const {
    auto it = nodes_.find(id);
    if (it == nodes_.end()) return false;

    std::shared_ptr<Node> current = it->second;

    while (current) {
        std::string parentHash;
        if (auto parent = current->parent.lock()) {
            parentHash = parent->hash;
        }

        const std::string expected = computeHash(*current, parentHash);
        if (expected != current->hash) {
            return false;
        }

        if (current->cls == NodeClass::Root) {
            break;
        }

        current = current->parent.lock();
    }

    return true;
}

bool BlockTree::verifySubTree(const std::string& id) const {
    auto rootNode = findNode(id);
    if (!rootNode) return false;

    // First verify upward from current node to root
    if (!verifyNodeAndAncestors(id)) return false;

    // Then verify entire subtree downward via DFS
    std::vector<std::shared_ptr<Node>> stack;
    stack.push_back(rootNode);

    while (!stack.empty()) {
        auto node = stack.back();
        stack.pop_back();

        std::string parentHash;
        if (auto parent = node->parent.lock()) {
            parentHash = parent->hash;
        }

        const std::string expected = computeHash(*node, parentHash);
        if (expected != node->hash) return false;

        for (auto& child : node->children) {
            stack.push_back(child);
        }
    }

    return true;
}

std::shared_ptr<Node> BlockTree::findNode(const std::string& id) const {
    auto it = nodes_.find(id);
    if (it == nodes_.end()) return nullptr;
    return it->second;
}

std::string BlockTree::computeHash(const Node& node, const std::string& parentHash) const {
    std::ostringstream oss;
    oss << node.id << '|'
        << node.index << '|'
        << node.timestamp << '|'
        << node.nonce << '|'
        << node.name << '|'
        << node.filePath.string() << '|'
        << NodeClassToString(node.cls) << '|'
        << parentHash;

    const std::string data = oss.str();
    const std::size_t h = std::hash<std::string>{}(data);

    std::ostringstream hex;
    hex << std::hex << std::setw(sizeof(h) * 2)
        << std::setfill('0') << h;

    return hex.str();
}

std::uint32_t BlockTree::randomNonce() {
    std::uniform_int_distribution<std::uint32_t> dist;
    return dist(rng_);
}