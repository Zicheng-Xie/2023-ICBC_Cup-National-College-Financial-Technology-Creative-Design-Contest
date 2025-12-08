// ResourceManager.cpp
#include "ResourceManager.h"

#include <iostream>

ResourceManager::ResourceManager(const std::filesystem::path& baseDir)
    : baseDir_(baseDir) {}

void ResourceManager::registerNode(const std::shared_ptr<Node>& node) {
    if (!node) return;

    GameResource res;
    res.id  = node->id;
    res.cls = node->cls;

    if (node->filePath.is_absolute()) {
        res.path = node->filePath;
    } else {
        res.path = baseDir_ / node->filePath;
    }

    res.loaded = false;

    resources_[res.id] = std::move(res);
}

void ResourceManager::preloadBigObjects() {
    for (auto& kv : resources_) {
        GameResource& res = kv.second;
        if (res.cls == NodeClass::Big) {
            loadResource(res);
        }
    }
}

void ResourceManager::ensureLoadedForView(const std::string& id, const BlockTree& tree) {
    auto node = tree.findNode(id);
    if (!node) {
        std::cerr << "[ResourceManager] node not found: " << id << '\n';
        return;
    }

    // 1. Current node
    auto it = resources_.find(node->id);
    if (it != resources_.end()) {
        loadResource(it->second);
    }

    // 2. Upward: Ensure all parent nodes are loaded (city block / building shell)
    auto current = node->parent.lock();
    while (current) {
        auto it2 = resources_.find(current->id);
        if (it2 != resources_.end()) {
            loadResource(it2->second);
        }
        current = current->parent.lock();
    }

    // 3. Downward: If it's big/child, preload one level of child nodes (room / tiny object)
    if (node->cls == NodeClass::Big || node->cls == NodeClass::Child) {
        for (auto& child : node->children) {
            auto it3 = resources_.find(child->id);
            if (it3 != resources_.end()) {
                loadResource(it3->second);
            }
        }
    }
}

void ResourceManager::loadResource(GameResource& res) {
    if (res.loaded) return;

    if (!std::filesystem::exists(res.path)) {
        std::cerr << "[ResourceManager] file not found: " << res.path << '\n';
        return;
    }

    // ⚠️ This is "abstract loading":
    // In real projects, replace with your game engine's loading function (UE5 Asset / GLTF / FBX etc.)
    std::cout << "[ResourceManager] loading " << res.path
              << " (id=" << res.id << ")\n";

    // TODO: Replace with real loading logic
    res.loaded = true;
}