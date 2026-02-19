# versions.md

## Contexte de capture
- Dépôt: `MBES-lib`
- Branche: `master`
- Commit: `b44f136`
- Date de capture: `2026-02-19 17:08:43 UTC`

## Versions déclarées dans le dépôt
| Élément | Version / Valeur | Source |
|---|---|---|
| CMake minimum | `3.7.1` | `CMakeLists.txt:1` |
| Standard C++ (racine) | `C++17` | `CMakeLists.txt:8` |
| Eigen requis | `>= 3.3` | `CMakeLists.txt:5` |
| Version Makefile Linux | `0.1.0` | `Makefile:4` |
| Version Makefile Windows | `0.1.0` | `MakefileWindows:4` |
| Version Jenkins | `0.1.$BUILD_ID` | `Jenkinsfile:4`, `Jenkinsfile:6`, `Jenkinsfile:8` |
| Version Doxygen projet | `CURRENT_VERSION_OF_MBES_LIB` (placeholder) | `Doxyfile:41` |
| Framework de test embarqué | Catch2 `2.13.10` | `test/catch.hpp:2`, `test/catch.hpp:16` |
| Standard C++ sous-projet overlap | `C++11` | `src/examples/overlap/CMakeLists.txt:2` |

## Versions réellement détectées dans l’environnement
| Outil | Version détectée |
|---|---|
| `cmake` | `3.28.3` |
| `ctest` | `3.28.3` |
| `g++` | `13.3.0` |
| `make` | `4.3` |
| `doxygen` | `1.9.8` |
| `cppcheck` | `2.13.0` |
| `gcovr` | `8.6` |
| `python3` | `3.12.3` |
| `pkg-config` | `1.8.1` |

## Validation build/tests/doc
- Build CMake: OK (`cmake -S . -B build && cmake --build build`)
- Exécutable de tests: OK (`./build/test/tests -r compact`)  
  Résultat: 120 test cases, 687 assertions, tout passe.
- CTest: KO pour la découverte (`No tests were found!!!`)  
  Référence structurelle: binaire de test créé (`CMakeLists.txt:70`) mais pas d’enregistrement `add_test`.
- Documentation (`make doc`): OK, avec warnings Doxygen (tags obsolètes).
- Coverage (`make coverage`): OK, avec warning de dépréciation `gcovr --branches`.

## Incohérences de versioning
1. Pas de version unique du projet.
2. CMake ne porte pas de version formelle (`project(... VERSION ...)` absent).
3. Jenkins fabrique une version dynamique (`0.1.$BUILD_ID`) différente de `0.1.0` dans les Makefiles.
4. Doxygen utilise un placeholder non résolu (`CURRENT_VERSION_OF_MBES_LIB`).
5. Standard C++ incohérent entre racine (`C++17`) et `overlap` (`C++11`).

## Recommandation de normalisation
1. Déclarer une version canonique dans CMake: `project(MBES-lib VERSION X.Y.Z)`.
2. Réutiliser cette version dans Jenkins (pipeline), Doxygen et packaging.
3. Éviter les versions codées en dur dans plusieurs fichiers (`Makefile`, `MakefileWindows`, `Jenkinsfile`, `Doxyfile`).
4. Uniformiser le standard C++ (idéalement `C++17`) sur tous les sous-projets actifs.
