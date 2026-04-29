# TP1 : Calcul GPU avec les Compute Shaders Vulkan

## Objectifs

Ce TP a pour but de vous familiariser avec le **pipeline compute de Vulkan** et son pilotage depuis le CPU. Vous allez apprendre à :

- Allouer des buffers GPU et les remplir depuis le CPU
- Configurer des descriptor sets pour exposer ces buffers à un shader
- Créer et exécuter un compute pipeline
- Enchaîner plusieurs pipelines dans un seul command buffer
- Comprendre pourquoi et comment synchroniser les accès mémoire entre deux passes GPU

---

## Contexte : le pipeline compute Vulkan

Contrairement au pipeline graphique, le **pipeline compute** n'a pas de vertex shader, de rasterizer ou de fragment shader. Il expose directement le GPU comme un processeur massivement parallèle : un shader GLSL (`*.comp`) est lancé sur un grand nombre d'invocations simultanées.

### Les objets clés

| Objet | Rôle |
|---|---|
| `Buffer` | Zone mémoire GPU (lecture/écriture depuis le shader) |
| `DescriptorSetLayout` | Décrit la structure des bindings d'un shader |
| `DescriptorPool` | Réservoir depuis lequel on alloue des descriptor sets |
| `DescriptorSet` | Lie des buffers concrets aux bindings déclarés dans le layout |
| `ComputePipeline` | Compile le shader et le rend exécutable |
| `CommandBuffer` | Enregistre une séquence de commandes à soumettre au GPU |

### Schéma d'une passe compute

```
CPU                              GPU
 |                                |
 |-- [creer buffers]              |
 |-- [creer descriptors]          |
 |-- [creer pipeline]             |
 |-- [enregistrer commandes] -->  |
 |       bind pipeline            |
 |       bind descriptor sets     |
 |       dispatch(N)  ---------> N invocations paralleles du shader
 |-- [soumettre]                  |
 |-- [attendre]    <------------- fin d'execution
 |-- [lire resultat depuis CPU]   |
```

### Dispatch et invocations

`pipeline.dispatch(cmdBuffer, N)` lance `N` invocations du compute shader. Chaque invocation a accès à `gl_GlobalInvocationID.x` pour connaître son identifiant unique (de `0` à `N-1`).

---

## Compilation et exécution

```bash
# Depuis le dossier realtimerendering-students/
mkdir build && cd build
cmake ..

# Compiler et lancer un exercice
make TP1_exercice1
./TP1_exercice1
```

---

## Exercice 1 : Addition de vecteurs (`src/TP1/exercice1.cpp`)

### Objectif

Calculer sur le GPU `C[i] = A[i] + B[i]` pour `i` allant de `0` à `size-1`, puis vérifier le résultat sur le CPU.

### Shader à compléter

Ouvrez `shaders/TP1/addition.comp`. Les bindings sont déjà déclarés. Complétez la fonction `main()` pour lire `_a[i]` et `_b[i]` et écrire le résultat dans `_c[i]`.

### Ce que vous devez implémenter dans `exercice1.cpp`

Le fichier contient 10 TODOs numérotés. Suivez-les dans l'ordre :

1. **Buffers GPU** — créez `ABuffer`, `BBuffer` (depuis les vecteurs CPU) et `CBuffer` (vide, host-accessible pour le readback).
2. **DescriptorSetLayout** — déclarez 3 bindings de type storage buffer accessibles depuis le stage compute.
3. **DescriptorPool** — réservez de la place pour 3 descripteurs et 1 set.
4. **Allocation du descriptor set** — allouez un set depuis le pool.
5. **Mise à jour du descriptor set** — liez `ABuffer` au binding 0, `BBuffer` au binding 1, `CBuffer` au binding 2.
6. **ComputePipeline** — créez le pipeline à partir de `shaders/TP1/addition.comp`.
7. **CommandBuffer** — créez un command buffer avec fence.
8. **Enregistrement** — commencez l'enregistrement, liez le pipeline et les descripteurs, dispatchez pour `size` invocations, terminez.
9. **Soumission** — soumettez à la compute queue et attendez la fin.
10. **Readback** — copiez le contenu de `CBuffer` vers le vecteur CPU `C`.

### Résultat attendu

```
Verification des resultats...
Succes ! Les 1000 resultats sont corrects.
C[0]=0 (attendu 0)
C[999]=2997 (attendu 2997)
```

---

## Exercice 2 : Opérations enchaînées (`src/TP1/exercice2.cpp`)

### Objectif

Enchaîner deux passes compute dans un seul command buffer :

- **Passe 1** (`addition.comp`) : `C[i] = A[i] + B[i]`
- **Passe 2** (`multiply.comp`) : `D[i] = A[i] * C[i]`

La passe 2 **dépend** de la sortie de la passe 1.

### Shader à compléter

Ouvrez `shaders/TP1/multiply.comp`. Les bindings sont déclarés. Complétez `main()` pour calculer `_d[i] = _a[i] * _c[i]`.

### Ce que vous devez implémenter

La solution de l'exercice 1 est déjà fournie. Complétez les 6 TODOs restants :

1. Ajoutez `DBuffer` (même paramètres que `CBuffer`).
2. Adaptez le `DescriptorPool` pour accueillir 2 sets et 4 buffers au total.
3. Créez et liez le descriptor set de la passe multiplication (`A→0`, `C→1`, `D→2`).
4. Créez le `ComputePipeline` de multiplication.
5. Ajoutez le dispatch de la passe multiplication dans le command buffer.
6. Lisez le résultat de `DBuffer` vers le vecteur CPU `D`.

### Première exécution

Compilez et exécutez. Observez le résultat de la vérification de `D`. Est-il correct ?

> **Question** : La passe 2 lit `C[i]`, qui est écrit par la passe 1. Pourquoi le résultat peut-il être incorrect même si les deux dispatches sont dans le même command buffer ?

### Les barrières mémoire

Sur GPU, les écritures d'un shader ne sont **pas automatiquement visibles** pour le shader suivant, même dans le même command buffer. Sans synchronisation explicite, le driver est libre de réordonner ou d'exécuter les deux dispatches en parallèle.

Une **barrière mémoire** force le GPU à attendre que toutes les écritures précédentes soient visibles avant de continuer :

```cpp
vk::MemoryBarrier memoryBarrier{
    vk::AccessFlagBits::eShaderWrite,  // ce qui a ete ecrit
    vk::AccessFlagBits::eShaderRead    // doit etre visible en lecture
};
cmdBuffer.pipelineBarrier(
    vk::PipelineStageFlagBits::eComputeShader,  // apres cette etape
    vk::PipelineStageFlagBits::eComputeShader,  // avant cette etape
    {}, memoryBarrier, {}, {}
);
```

Ajoutez cette barrière entre le dispatch de la passe 1 et celui de la passe 2, puis vérifiez que `D` est maintenant correct.

### Résultat attendu (après ajout de la barrière)

```
Verification de C = A + B...
Succes ! C est correct.
Verification de D = A * C...
Succes ! D est correct.
```

---

## Exercice 3 : Filtre boîte itératif (`src/TP1/exercice3.cpp`)

### Objectif

Appliquer trois fois de suite un **filtre boîte** (moyenne des voisins gauche, centre, droit) sur un signal 1D :

```
B[i] = (A[i-1] + A[i] + A[i+1]) / 3   -- passe 1
C[i] = (B[i-1] + B[i] + B[i+1]) / 3   -- passe 2
D[i] = (C[i-1] + C[i] + C[i+1]) / 3   -- passe 3
```

Ce filtre est omniprésent en rendu temps-réel : blur, bloom, anti-aliasing temporel (TAA), lissage de cartes d'ombres...

### Pourquoi plusieurs passes sont-elles obligatoires ?

La passe 2 calcule `C[5]` à partir de `B[4]`, `B[5]` et `B[6]`. Ces trois valeurs dépendent chacune de 3 éléments de `A`. Si on tentait de fusionner les deux passes, un invocation lirait et écrirait dans la même zone mémoire simultanément — ce qui est un **data hazard** non défini.

Le même pipeline (`boxfilter.comp`) est réutilisé trois fois avec des **descriptor sets différents** (ping-pong entre buffers) :

```
Set 0 : A (in) --> B (out)   passe 1
Set 1 : B (in) --> C (out)   passe 2
Set 2 : C (in) --> D (out)   passe 3
```

### Shader à écrire

Ouvrez `shaders/TP1/boxfilter.comp` et écrivez le shader **depuis zéro** : version GLSL, taille de workgroup, déclaration des bindings, et corps de `main()`.

> **Contrainte** : vous ne devez écrire **qu'un seul shader**. Les trois passes utilisent exactement le même fichier `.comp` — c'est le changement de descriptor set côté C++ qui redirige les données entre les buffers A, B, C, D.

### Ce que vous devez implémenter

Aucun code GPU n'est fourni dans `exercice3.cpp`. Implémentez les 6 étapes en vous appuyant sur ce que vous avez appris dans les exercices précédents :

1. **Buffers GPU** — créez `ABuffer` (depuis le vecteur `A`), `BBuffer`, `CBuffer` et `DBuffer`. `ABuffer` peut être un buffer classique. `BBuffer`, `CBuffer` et `DBuffer` doivent être accessibles depuis le CPU pour le readback : utilisez les flags `eCreateDedicatedMemory | eCreateHostAccessSequentialWrite`.
2. **DescriptorSetLayout et DescriptorPool** — layout avec 2 bindings storage buffer, pool dimensionné pour 3 sets.
3. **Descriptor sets** — allouez et liez les 3 sets en ping-pong : `A→B`, `B→C`, `C→D`.
4. **Pipeline** — créez le `ComputePipeline` depuis `shaders/TP1/boxfilter.comp`.
5. **Command buffer** — enregistrez les 3 dispatches et soumettez. N'oubliez pas les barrières mémoire entre chaque passe (même syntaxe qu'en exercice 2).
6. **Readback** — lisez le contenu de `DBuffer` via `DBuffer.map()` et copiez-le dans le vecteur `D`.

### Résultat attendu

Le signal d'entrée est un créneau. Après trois applications du filtre boîte, les transitions nettes deviennent progressivement arrondies. La fonction `plotSignal` fournie affiche le résultat en ASCII :

```
 *****              **              **              **              **           
 |||||*            *||*            *||*            *||*            *||*          
 ||||||*          *||||*          *||||*          *||||*          *||||*         
 |||||||          ||||||          ||||||          ||||||          ||||||         
 |||||||          ||||||          ||||||          ||||||          ||||||         
 |||||||*        *||||||*        *||||||*        *||||||*        *||||||*        
 ||||||||        ||||||||        ||||||||        ||||||||        ||||||||        
 ||||||||        ||||||||        ||||||||        ||||||||        ||||||||        
 ||||||||        ||||||||        ||||||||        ||||||||        ||||||||        
 ||||||||*      *||||||||*      *||||||||*      *||||||||*      *||||||||*      *
 |||||||||      ||||||||||      ||||||||||      ||||||||||      ||||||||||      |
 |||||||||      ||||||||||      ||||||||||      ||||||||||      ||||||||||      |
 |||||||||*    *||||||||||*    *||||||||||*    *||||||||||*    *||||||||||*    *|
 ||||||||||*  *||||||||||||*  *||||||||||||*  *||||||||||||*  *||||||||||||*  *||
 |||||||||||**||||||||||||||**||||||||||||||**||||||||||||||**||||||||||||||**|||
+--------------------------------------------------------------------------------
```