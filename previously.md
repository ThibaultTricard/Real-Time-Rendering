# Previously

## 2026-04-30 (session 2)
- Dans `TD/sujets/CMakeLists.txt`, seul TP1_exercice1 est décommenté — c'est **intentionnel** : les étudiants décommentent les exercices au fur et à mesure de leur avancement. Ne pas "corriger" cela.
- Workflow de conception d'un TP (ordre obligatoire) :
  1. Définir les objectifs pédagogiques
  2. Rédiger et valider la correction (code qui compile et fonctionne)
  3. Créer le code à trou pour les étudiants à partir de la correction
  4. Écrire le sujet en `.md`
  Ne jamais commencer par le `.md` ni par le code à trou.

## 2026-04-30
- Initialized graphify knowledge graph (`graphify update .`)
- Created `.graphifyignore` to exclude `Cours/_book/` and `Cours/cours_files/` from indexing (were polluting the graph with minified JS)
- Rebuilt the graph: went from 713 noisy nodes down to 60 clean nodes representing actual source files
- Updated CLAUDE.md long-term memory instructions to be more explicit and actionable