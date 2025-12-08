// BlockTree.h
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <filesystem>
#include <cstdint>
#include <random>

// Level 1: City Root Node / Big Object / Child Object / Tiny Object
enum class NodeClass {
    Root,
    Big,
    Child,
    Tiny
};

// Utility function: Convert NodeClass to string for JSON / logging
std::string NodeClassToString(NodeClass cls);
NodeClass NodeClassFromString(const std::string& s);

// A node in the block tree (corresponds to a "digital collection resource")
struct Node {
    std::string id;                     // Globally unique ID, e.g., "001-01-02"
    int index = 0;                      // Index at current level
    std::int64_t timestamp = 0;         // Timestamp in milliseconds
    std::uint32_t nonce = 0;            // Random number for mining
    std::string name;                   // Object name
    std::filesystem::path filePath;     // Resource file path (FBX/GLTF etc.)
    NodeClass cls = NodeClass::Big;     // big / child / tiny / root
    std::string hash;                   // Current node's hash

    std::weak_ptr<Node> parent;         // Parent node
    std::vector<std::shared_ptr<Node>> children; // Child nodes
};

// "Candidate block" received from the network
struct CandidateBlock {
    std::string id;
    std::string parentId;
    int index = 0;
    std::int64_t timestamp = 0;
    std::uint32_t nonce = 0;
    std::string name;
    std::filesystem::path filePath;
    NodeClass cls = NodeClass::Big;
    std::string hash;   // Hash computed by the client for comparison
};

// Core of the tree-structured blockchain: Manages root node + subtree structure
class BlockTree {
public:
    BlockTree();

    // After consensus is reached, formally add the candidate block to the tree
    std::shared_ptr<Node> addNode(const CandidateBlock& block);

    // Local "mining verification": Recalculate hash using the same rules and compare with block.hash
    bool miner(const CandidateBlock& block) const;

    // Verify that the hash of the entire path from a node to the root is consistent
    bool verifyNodeAndAncestors(const std::string& id) const;

    // Verify the entire subtree rooted at a node (first upward, then downward)
    bool verifySubTree(const std::string& id) const;

    // Get root node (unique block tree root)
    std::shared_ptr<Node> root() const { return root_; }

    // Find node by ID (for resource manager / business layer)
    std::shared_ptr<Node> findNode(const std::string& id) const;

private:
    std::shared_ptr<Node> root_;
    std::unordered_map<std::string, std::shared_ptr<Node>> nodes_;
    mutable std::mt19937_64 rng_;

    // Compute hash for a node (includes parent's hash)
    std::string computeHash(const Node& node, const std::string& parentHash) const;

    // Generate random nonce
    std::uint32_t randomNonce();
};