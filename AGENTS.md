# AGENTS.md

## Objet
Ce document résume l'analyse technique du dépôt `MBES-lib`, les incohérences code/documentation constatées, et les actions recommandées.

## Périmètre analysé
- Documentation: `README.md`, `Doxyfile`
- Build/CI: `CMakeLists.txt`, `Makefile`, `MakefileWindows`, `Jenkinsfile`, `Scripts/*`
- Code source: `src/datagrams/*`, `src/examples/*`, `src/hydroblock/*`, `src/examples/overlap/*`
- Tests: `test/main.cpp`, `test/*`

## Vue rapide du dépôt
- Langage principal: C++
- Domaine: parsing de datagrammes MBES, géoréférencement, raytracing, outillage de conversion
- Formats gérés dans le code:
- `.all`, `.xtf`, `.s7k`, `.kmall`
- dossier Hydroblock (détection par répertoire)

## Dépendances
### Build et test (requis)
- `gcc`, `g++`, `make` (paquet Ubuntu: `build-essential`)
- `cmake` et `ctest`
- `libeigen3-dev` (Eigen >= 3.3)
- `pkg-config`

### Documentation (requis pour `make doc`)
- `doxygen`
- `graphviz` (`dot`)

### Qualité / couverture (requis pour `make coverage`)
- `cppcheck`
- `gcovr`
- `python3` (si `gcovr` est installé via `pip`)

### Optionnel (cibles spécifiques)
- `libpcl-dev` et `qtbase5-dev` pour les cibles de visualisation/overlap (non nécessaires au build principal)

## Incohérences validées

### 1) Documentation des formats incomplète
- Constat: le README annonce 4 formats (`.all`, `.kmall`, `.s7k`, `.xtf`) mais ne mentionne pas le mode Hydroblock par répertoire.
- Preuve:
- `README.md:5`
- `src/datagrams/DatagramParserFactory.cpp:17`
- Impact: utilisateur peut penser qu'un dossier Hydroblock n'est pas supporté.

### 2) Documentation des exécutables incomplète
- Constat: le README décrit 5 programmes, mais CMake en construit plus (`raytrace`, `datagram-raytracer`, `wgs2lgf`, `lgf2wgs`).
- Preuve:
- `README.md:11`
- `CMakeLists.txt:31`
- `CMakeLists.txt:35`
- `CMakeLists.txt:55`
- `CMakeLists.txt:59`
- Impact: fonctionnalités disponibles non documentées.

### 3) Versioning éclaté entre plusieurs fichiers
- Constat: version définie différemment selon les outils.
- Preuve:
- `Makefile:4` (`0.1.0`)
- `MakefileWindows:4` (`0.1.0`)
- `Jenkinsfile:4` et `Jenkinsfile:6` (`0.1.$BUILD_ID`)
- `Doxyfile:41` (`CURRENT_VERSION_OF_MBES_LIB`, placeholder)
- `CMakeLists.txt:3` (pas de `project(... VERSION ...)`)
- Impact: artefacts/doc potentiellement désynchronisés.

### 4) Cibles overlap/viewer obsolètes ou cassées
- Constat: une dépendance de code référencée n'existe plus.
- Preuve:
- `src/examples/overlap/overlap.cpp:43` inclut `../viewer/smallUtilityFunctions.hpp`
- `src/examples/viewer` absent dans le dépôt
- `MakefileWindows:47` référence `src/examples/viewer/`
- Impact: build des cibles `overlap` / `pcl-viewer` non fiable.

### 5) Incohérence standard C++ entre racine et sous-projet overlap
- Constat: racine en C++17, sous-projet overlap en C++11.
- Preuve:
- `CMakeLists.txt:8`
- `src/examples/overlap/CMakeLists.txt:2`
- Impact: comportement/build variables selon la cible.

### 6) Message d'erreur incorrect dans `georeference`
- Constat: le nom de fichier n'est pas interpolé dans le message d'exception.
- Preuve:
- `src/examples/georeference.cpp:222`
- Impact: diagnostic terrain moins utile.

### 7) Sortie binaire CMake incomplète pour certaines cibles
- Constat: `wgs2lgf` et `lgf2wgs` sont construites mais absentes de `set_target_properties(... RUNTIME_OUTPUT_DIRECTORY ...)`.
- Preuve:
- `CMakeLists.txt:56`
- `CMakeLists.txt:60`
- `CMakeLists.txt:65`
- Impact: emplacement de sortie potentiellement différent selon la cible.

### 8) Couverture de tests partielle par agrégation
- Constat: `GeorefPCLviewerTest.hpp` existe mais n'est pas inclus dans `test/main.cpp`.
- Preuve:
- `test/GeorefPCLviewerTest.hpp:1`
- `test/main.cpp:5`
- Impact: tests présents mais non exécutés par le binaire principal.

### 9) CTest ne voit aucun test
- Constat: CMake construit bien l'exécutable `tests`, mais aucun test n'est enregistré côté CTest.
- Preuve:
- `CMakeLists.txt:70`
- `Scripts/linuxBuildAndTest.bash:8`
- Résultat observé: `ctest --test-dir build` retourne `No tests were found!!!`.
- Impact: `ctest` ne valide rien, alors que les scripts projet exécutent `build/test/tests` directement.

### 10) Configuration Doxygen partiellement obsolète
- Constat: plusieurs clés Doxygen présentes sont obsolètes pour Doxygen 1.9.8.
- Preuve:
- `Doxyfile:247`
- `Doxyfile:1108`
- `Doxyfile:1244`
- `Doxyfile:1525`
- `Doxyfile:1813`
- `Doxyfile:1829`
- `Doxyfile:1895`
- `Doxyfile:1993`
- `Doxyfile:2173`
- `Doxyfile:2186`
- `Doxyfile:2195`
- `Doxyfile:2237`
- `Doxyfile:2244`
- `Doxyfile:2470`
- Impact: bruit de warnings dans la génération de doc et maintenance plus difficile du fichier de config.

### 11) Option gcovr dépréciée dans le Makefile
- Constat: la couverture utilise `gcovr --branches`, option dépréciée.
- Preuve:
- `Makefile:94`
- `Makefile:95`
- Impact: avertissements en CI et risque de rupture future lors des upgrades de `gcovr`.

### 12) Avertissements de robustesse mémoire détectés
- Constat: suppression polymorphe non sûre et `new[]/delete` incohérent dans les tests.
- Preuve:
- `src/svp/SvpSelectionStrategy.hpp:18` (pas de destructeur virtuel)
- `test/RayTracerAppTest.hpp:120` (`delete svpStrategy`)
- `test/KongsbergParserTest.hpp:224` (`new[]`)
- `test/KongsbergParserTest.hpp:232` (`delete` au lieu de `delete[]`)
- Impact: comportement indéfini possible (principalement dans tests), qualité de code abaissée.

## Validation d'exécution
- Date de validation: `2026-02-19 17:08:43 UTC`
- Build CMake: OK (`cmake -S . -B build && cmake --build build`)
- Test binaire: OK (`./build/test/tests -r compact`) => 120 cas, 687 assertions, tous passés
- CTest: KO côté découverte (`No tests were found!!!`)
- Documentation (`make doc`): OK, avec warnings Doxygen (tags obsolètes + incohérence de paramètre documenté)
- Couverture (`make coverage`): OK, avec warnings (`gcovr --branches` déprécié, warnings compilateur)
- Sortie binaire confirmée:
- `build/bin/*` pour la plupart des outils
- `build/wgs2lgf` et `build/lgf2wgs` (hors `build/bin`, conforme à l'incohérence #7)

## Actions recommandées (ordre prioritaire)
1. Définir une source unique de version dans CMake (`project(MBES-lib VERSION x.y.z)`) et propager vers Jenkins/Doxygen.
2. Mettre à jour `README.md` (formats réellement supportés, liste complète des outils, prérequis build).
3. Corriger ou retirer les cibles obsolètes (`viewer`/`overlap`) tant que les sources requises sont absentes.
4. Harmoniser la sortie binaire de toutes les cibles CMake, y compris `wgs2lgf` et `lgf2wgs`.
5. Enregistrer les tests dans CTest (`enable_testing()` + `add_test(...)`) pour fiabiliser la commande `ctest`.
6. Mettre à jour `Doxyfile` via `doxygen -u`, puis corriger les warnings de doc restants.
7. Corriger les points de robustesse mémoire et les warnings compilateur les plus critiques.
