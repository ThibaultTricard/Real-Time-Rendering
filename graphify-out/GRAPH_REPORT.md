# Graph Report - Real-Time-Rendering  (2026-04-30)

## Corpus Check
- 37 files · ~505,719 words
- Verdict: corpus is large enough that graph structure adds value.

## Summary
- 60 nodes · 61 edges · 13 communities detected
- Extraction: 77% EXTRACTED · 23% INFERRED · 0% AMBIGUOUS · INFERRED: 14 edges (avg confidence: 0.8)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- [[_COMMUNITY_Community 0|Community 0]]
- [[_COMMUNITY_Community 1|Community 1]]
- [[_COMMUNITY_Community 2|Community 2]]
- [[_COMMUNITY_Community 3|Community 3]]
- [[_COMMUNITY_Community 4|Community 4]]
- [[_COMMUNITY_Community 5|Community 5]]
- [[_COMMUNITY_Community 6|Community 6]]
- [[_COMMUNITY_Community 7|Community 7]]
- [[_COMMUNITY_Community 8|Community 8]]
- [[_COMMUNITY_Community 9|Community 9]]
- [[_COMMUNITY_Community 10|Community 10]]
- [[_COMMUNITY_Community 11|Community 11]]
- [[_COMMUNITY_Community 12|Community 12]]

## God Nodes (most connected - your core abstractions)
1. `loadModel()` - 9 edges
2. `main()` - 5 edges
3. `loadTexture()` - 5 edges
4. `main()` - 4 edges
5. `main()` - 4 edges
6. `main()` - 4 edges
7. `mipLevelCount()` - 4 edges
8. `generateMipmaps()` - 4 edges
9. `main()` - 3 edges
10. `main()` - 3 edges

## Surprising Connections (you probably didn't know these)
- `main()` --calls--> `loadModel()`  [INFERRED]
  correction/src/TP5/exercice2.cpp → sujets/src/utils/ObjLoader.hpp
- `main()` --calls--> `loadTexture()`  [INFERRED]
  correction/src/TP5/exercice2.cpp → sujets/src/utils/TextureLoader.hpp
- `main()` --calls--> `loadModel()`  [INFERRED]
  sujets/src/TP4/exercice1.cpp → sujets/src/utils/ObjLoader.hpp
- `main()` --calls--> `loadModel()`  [INFERRED]
  sujets/src/TP4/exercice2.cpp → sujets/src/utils/ObjLoader.hpp
- `main()` --calls--> `loadModel()`  [INFERRED]
  sujets/src/TP3/exercice1.cpp → sujets/src/utils/ObjLoader.hpp

## Communities

### Community 0 - "Community 0"
Cohesion: 0.18
Nodes (4): main(), main(), main(), loadTexture()

### Community 1 - "Community 1"
Cohesion: 0.27
Nodes (4): main(), main(), generateMipmaps(), mipLevelCount()

### Community 2 - "Community 2"
Cohesion: 0.36
Nodes (4): main(), computeMeshTangents(), loadModel(), loadScene()

### Community 3 - "Community 3"
Cohesion: 0.83
Nodes (2): main(), plotSignal()

### Community 4 - "Community 4"
Cohesion: 0.67
Nodes (1): main()

### Community 5 - "Community 5"
Cohesion: 0.67
Nodes (1): main()

### Community 6 - "Community 6"
Cohesion: 0.67
Nodes (1): main()

### Community 7 - "Community 7"
Cohesion: 0.67
Nodes (1): main()

### Community 8 - "Community 8"
Cohesion: 0.67
Nodes (1): main()

### Community 9 - "Community 9"
Cohesion: 0.67
Nodes (1): main()

### Community 10 - "Community 10"
Cohesion: 0.67
Nodes (1): Scene

### Community 11 - "Community 11"
Cohesion: 0.67
Nodes (1): main()

### Community 12 - "Community 12"
Cohesion: 0.67
Nodes (1): main()

## Knowledge Gaps
- **Thin community `Community 3`** (4 nodes): `exercice3.cpp`, `exercice3.cpp`, `main()`, `plotSignal()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 4`** (3 nodes): `exercice1.cpp`, `exercice1.cpp`, `main()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 5`** (3 nodes): `exercice2.cpp`, `exercice2.cpp`, `main()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 6`** (3 nodes): `exercice1.cpp`, `exercice1.cpp`, `main()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 7`** (3 nodes): `exercice2.cpp`, `exercice2.cpp`, `main()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 8`** (3 nodes): `exercice3.cpp`, `exercice3.cpp`, `main()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 9`** (3 nodes): `exercice4.cpp`, `exercice4.cpp`, `main()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 10`** (3 nodes): `Scene.hpp`, `Scene.hpp`, `Scene`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 11`** (3 nodes): `exercice1.cpp`, `exercice1.cpp`, `main()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 12`** (3 nodes): `exercice2.cpp`, `exercice2.cpp`, `main()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `loadModel()` connect `Community 2` to `Community 0`, `Community 1`, `Community 4`, `Community 5`?**
  _High betweenness centrality (0.267) - this node is a cross-community bridge._
- **Why does `main()` connect `Community 1` to `Community 2`?**
  _High betweenness centrality (0.140) - this node is a cross-community bridge._
- **Why does `main()` connect `Community 0` to `Community 2`?**
  _High betweenness centrality (0.052) - this node is a cross-community bridge._
- **Are the 7 inferred relationships involving `loadModel()` (e.g. with `main()` and `main()`) actually correct?**
  _`loadModel()` has 7 INFERRED edges - model-reasoned connections that need verification._
- **Are the 3 inferred relationships involving `main()` (e.g. with `loadModel()` and `mipLevelCount()`) actually correct?**
  _`main()` has 3 INFERRED edges - model-reasoned connections that need verification._
- **Are the 3 inferred relationships involving `loadTexture()` (e.g. with `main()` and `main()`) actually correct?**
  _`loadTexture()` has 3 INFERRED edges - model-reasoned connections that need verification._
- **Are the 2 inferred relationships involving `main()` (e.g. with `loadModel()` and `loadTexture()`) actually correct?**
  _`main()` has 2 INFERRED edges - model-reasoned connections that need verification._