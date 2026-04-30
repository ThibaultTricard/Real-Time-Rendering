# Next

## Graphify
- Add .qmd course files to the knowledge graph (requires LLM semantic extraction — need to investigate the right graphify command or workflow)

## Course / TDs

### CMake correction
- [ ] Vérifier que `TD/correction/CMakeLists.txt` couvre TP4 et TP5

### TP5 — Displacement / Normal / Parallax mapping
- [ ] Rédiger `TD/sujets/TP5.md` (fichier vide)
- [ ] Créer `TD/sujets/src/TP5/exercice2.cpp`
- [ ] Ajouter TP5 dans `TD/sujets/CMakeLists.txt`

### TP6 — BRDF & Microfacets (GGX isotrope + anisotrope)
- [ ] Rédiger `TD/sujets/TP6.md`
- [ ] Créer code sujets (`TD/sujets/src/TP6/`)
- [ ] Créer code correction (`TD/correction/src/TP6/`)
- [ ] Créer shaders sujets et correction
- [ ] Trouver un modèle/texture PBR (roughness + metallic) pour tester GGX
- [ ] Ajouter dans les deux CMakeLists

### TP7 — Chargement de scènes glTF 2.0
- [ ] Créer code sujets (`TD/sujets/src/TP7/`) — sujet rédigé, code absent
- [ ] Créer code correction (`TD/correction/src/TP7/`)
- [ ] Créer shaders sujets et correction
- [ ] Trouver une scène glTF 2.0 avec textures (ex. DamagedHelmet.glb ou SciFiHelmet.glb)
- [ ] Ajouter dans les deux CMakeLists

### TP8 — Rayons secondaires (Shadows & Reflections)
- [ ] Rédiger `TD/sujets/TP8.md` (fichier vide)
- [ ] Créer code sujets et correction
- [ ] Créer shaders
- [ ] Trouver un modèle adapté aux ombres/réflexions
- [ ] Ajouter dans les deux CMakeLists

### TP9 — Deferred Shading & Screen Space Effects
- [ ] Rédiger `TD/sujets/TP9.md` (fichier vide)
- [ ] Créer code sujets et correction
- [ ] Créer shaders (G-buffer, composition, screen-space effects)
- [ ] Trouver une scène multi-objets pour tester le G-buffer
- [ ] Ajouter dans les deux CMakeLists

### Assets à trouver
- [ ] TP6 : sphère ou mesh avec cartes roughness + metallic (PBR)
- [ ] TP7 : scène glTF 2.0 — DamagedHelmet.glb ou SciFiHelmet.glb (glTF-Sample-Models)
- [ ] TP8 : scène adaptée shadows/reflections (ex. Sponza simplifié)
- [ ] TP9 : scène multi-objets (ex. Sponza, ou scène custom)