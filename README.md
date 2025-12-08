# 2023-ICBC_Cup-National-College-Financial-Technology-Creative-Design-Contest
_**First Prize**_ in the 2023 "ICBC Cup" National College Financial Technology Creative Design Contest _**(Provincial)**_

# 🌆 Heritage City BlockTree

> A tree-structured blockchain + game resource manager for digital heritage sandboxes.  
> A small C++ engine for managing intangible cultural heritage (ICH) digital collections & sandbox city resources via a tree-structured blockchain.

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![JSON](https://img.shields.io/badge/JSON-Poco-yellowgreen.svg)](https://pocoproject.org/)
[![Status](https://img.shields.io/badge/status-experimental-orange.svg)](#)
[![Shields.io](https://img.shields.io/badge/badges-shields.io-brightgreen)](https://shields.io/) <!-- cool badges per shields best-practice -->

---

## ✨ What is this?

This repo is a **minimal backend engine** for a sandbox-style digital heritage city:

- A **tree-structured blockchain (`BlockTree`)** describes the city → building → room → tiny-object hierarchy, each as a hashed block. :contentReference[oaicite:1]{index=1} :contentReference[oaicite:2]{index=2}  
- A **resource manager (`ResourceManager`)** uses that tree to **stream 3D assets** on demand (big objects, child objects, tiny objects).   
- A tiny **JSON-over-stdin “network layer” (`Requester`)** simulates consensus requests, so you can run everything locally first.   
- `main.cpp` wires them into an **event loop** with 3 modes: `add`, `check`, `view_node`. :contentReference[oaicite:5]{index=5}  

Designed as the underlying architecture for projects such as **"Intangible Cultural Heritage (ICH) Digital Collection Sandbox City"**, it leverages a block tree to ensure the on-chain integrity of the entire hierarchy spanning **"a city, a building, an exhibition cabinet, and an artifact"**, while simultaneously driving the loading of 3D resources within the game.

---

## 🧱 Core Ideas

### 1. Tree-Structured Blockchain

`BlockTree` manages a Merkle-like tree where each node is a **block**:

- `Node`  
  - `id` — global ID, e.g. `001-01-02`  
  - `cls` — `Root / Big / Child / Tiny`  
  - `hash` — computed from its data + parent hash  
  - `children` — child nodes (next level of the city / building) :contentReference[oaicite:6]{index=6}  
- `CandidateBlock`  
  - A “pending” block from the network (JSON) waiting to be mined / verified. :contentReference[oaicite:7]{index=7}  
- `BlockTree::miner`  
  - Recomputes the hash locally and compares with the incoming one. :contentReference[oaicite:8]{index=8}  
- `verifyNodeAndAncestors` / `verifySubTree`  
  - Verify the hash chain from any node up to root and/or down its subtree. :contentReference[oaicite:9]{index=9}  

> Think: **each building, room, and artifact is a block**; changing any detail breaks the entire path to the root.

---

### 2. Resource-Aware Loading

`ResourceManager` binds each blockchain node to a **game resource** (FBX/GLTF/UE asset path):   

- `registerNode(node)`  
  - Called when a block is accepted into `BlockTree`, records its resource path.
- `preloadBigObjects()`  
  - Preloads all `Big` nodes: city shells / major buildings.
- `ensureLoadedForView(id, tree)`  
  - When the player focuses a node:
    - Load that node’s asset
    - Load its ancestors (city → district → building)
    - Optionally preload one level of children (rooms / tiny artifacts)

> So your **world streaming** is literally driven by the **blockchain topology**.

---

### 3. JSON Protocol & Event Loop

`main.cpp` exposes a super simple JSON “protocol” over stdin/stdout:   

#### Modes

- `mode: "add"`  
  Add a new block to the tree.
- `mode: "check"`  
  Another node asks you to verify a block (you respond with `send_checkans`).
- `mode: "view_node"`  
  Game client wants to view one node (trigger resource loading).

#### Example Request: add a building block

```json
{
  "mode": "add",
  "id": "001",
  "parent_id": "root",
  "index": 1,
  "timestamp": 1733630000000,
  "rand": 123456,
  "name": "Jiading Bamboo Courtyard",
  "ele": "objects/jiading_bamboo.fbx",
  "class": "big",
  "hash": "deadbeefcafebabe" // client-computed hash
}
