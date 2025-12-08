// ResourceManager.h
#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>
#include <memory>

#include "BlockTree.h"

// Game resource description: 3D resource / UI resource corresponding to a block node
struct GameResource {
    std::string id;                  // ID corresponding to Node
    NodeClass   cls;                 // big / child / tiny
    std::filesystem::path path;      // Resource file path
    bool loaded = false;             // Whether loaded into the engine
};

// Resource Manager: Load / preload resources on demand based on BlockTree nodes
class ResourceManager {
public:
    explicit ResourceManager(const std::filesystem::path& baseDir);

    // Register a resource record when the block is formally added to BlockTree
    void registerNode(const std::shared_ptr<Node>& node);

    // Preload all "big objects" (city skeleton / building shell) when starting the game
    void preloadBigObjects();

    // Ensure all related resources are loaded when the player views a node (building / room / tiny object)
    void ensureLoadedForView(const std::string& id, const BlockTree& tree);

private:
    std::filesystem::path baseDir_;
    std::unordered_map<std::string, GameResource> resources_;

    void loadResource(GameResource& res);
};