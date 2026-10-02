# MariaDB Jev

Native MariaDB functions for semantic predicates backed by [TypeSafe Jev](https://docs.typesafe.ai/api). One row can carry up to eight natural-language conditions in **one** Jev request. This is an initial open-source release, not a claim of state-of-the-art throughput or accuracy.

## Français

### Requête

```sql
SELECT id, review
FROM reviews
WHERE rating >= 3
  AND jev_all(review, JSON_ARRAY(
    'La critique parle de la fin du film',
    'La critique recommande le film'
  )) = 1;
```

`jev_all(texte, tableau_json)` exige que toutes les conditions aient une probabilité Jev d'au moins 0,5. `jev_any` en exige une seule. `jev_probability(texte, condition)` renvoie la probabilité brute. `NULL` produit `NULL` sans appel réseau. Ce ne sont pas des résultats déterministes ni une recherche vectorielle.

### Exemple borné

[`sql/bounded-review-example.sql`](sql/bounded-review-example.sql) matérialise d'abord au plus cinq lignes admissibles dans une table temporaire, puis appelle `jev_probability` une fois par ligne retenue. La requête suppose la table `reviews(id, review, rating)` ; elle nécessite un serveur configuré et engendre jusqu'à cinq appels payants. Ne traitez pas la probabilité comme une décision automatique.

### Installer et vérifier

Sur Linux avec MariaDB, CMake, libcurl et Jansson installés :

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Copier `build/mariadb_jev.so` dans le répertoire indiqué par `SHOW VARIABLES LIKE 'plugin_dir'`, puis exécuter `sql/install.sql` avec un compte administrateur. Définir `JEV_API_KEY` dans **l'environnement du serveur MariaDB** avant son démarrage. Une variable définie seulement dans le client SQL n'est pas visible du serveur. Pour retirer les fonctions, exécuter `sql/uninstall.sql` avant de retirer la bibliothèque.

### Coût et limites

Le cache est limité à chaque expression SQL : mêmes texte, conditions et modèle = verdict réutilisé. Chaque ligne non mise en cache déclenche un appel HTTPS ; il n'y a ni index sémantique ni traitement par lots entre lignes. `JEV_MAX_REQUESTS` vaut 1 000 appels par expression par défaut ; `JEV_TIMEOUT_MS` vaut 10 000 ms. Maximum : 32 Kio de texte, 8 conditions de 8 Kio, réponse de 1 Mio. Filtrer d'abord avec les colonnes classiques et mesurer le plan. Les textes et conditions envoyés à Jev quittent le serveur MariaDB. Éviter les écritures automatiques et la réplication fondée sur les instructions SQL avec une fonction d'inférence.

## English

### Query

```sql
SELECT id, review
FROM reviews
WHERE rating >= 3
  AND jev_all(review, JSON_ARRAY(
    'The review discusses the ending',
    'The review recommends the movie'
  )) = 1;
```

`jev_all(text, json_array)` requires every condition to have Jev probability at least 0.5. `jev_any` requires one. `jev_probability(text, condition)` returns the raw probability. `NULL` yields `NULL` without a network call. These are model judgments, not deterministic facts or vector search.

### Bounded example

[`sql/bounded-review-example.sql`](sql/bounded-review-example.sql) first materializes at most five eligible rows in a temporary table, then calls `jev_probability` once per retained row. It assumes `reviews(id, review, rating)` and a configured server, and can make up to five paid calls. Do not treat the probability as an automatic decision.

### Install and verify

On Linux with MariaDB, CMake, libcurl and Jansson installed:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Copy `build/mariadb_jev.so` to the directory reported by `SHOW VARIABLES LIKE 'plugin_dir'`, then run `sql/install.sql` as an administrator. Set `JEV_API_KEY` in the **MariaDB server process environment** before starting it; setting it only in a SQL client's shell does not configure the server. Run `sql/uninstall.sql` before removing the library.

### Cost and limits

The cache is scoped to each SQL expression: identical text, conditions and model reuse a verdict. Each uncached row makes one HTTPS request; there is no semantic index or cross-row batching. `JEV_MAX_REQUESTS` defaults to 1,000 requests per expression and `JEV_TIMEOUT_MS` to 10,000 ms. Limits: 32 KiB text, eight conditions of 8 KiB each, 1 MiB response. Narrow candidates with ordinary indexed columns and inspect the plan. Text and conditions sent to Jev leave the MariaDB server. Avoid unattended writes and statement-based replication with an inference function.

## Español

### Consulta

```sql
SELECT id, review
FROM reviews
WHERE rating >= 3
  AND jev_all(review, JSON_ARRAY(
    'La reseña comenta el final de la película',
    'La reseña recomienda la película'
  )) = 1;
```

`jev_all(texto, arreglo_json)` exige que todas las condiciones tengan una probabilidad Jev de al menos 0,5. `jev_any` exige una. `jev_probability(texto, condición)` devuelve la probabilidad. `NULL` produce `NULL` sin llamada de red. Son juicios del modelo, no hechos deterministas ni búsqueda vectorial.

### Ejemplo acotado

[`sql/bounded-review-example.sql`](sql/bounded-review-example.sql) materializa primero un máximo de cinco filas aptas en una tabla temporal y luego llama a `jev_probability` una vez por fila conservada. Supone `reviews(id, review, rating)` y un servidor configurado; puede realizar hasta cinco llamadas de pago. No trate la probabilidad como una decisión automática.

### Instalar y comprobar

En Linux con MariaDB, CMake, libcurl y Jansson instalados:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Copie `build/mariadb_jev.so` al directorio indicado por `SHOW VARIABLES LIKE 'plugin_dir'` y ejecute `sql/install.sql` como administrador. Configure `JEV_API_KEY` en **el entorno del proceso servidor MariaDB** antes de iniciarlo; configurarla solo en el cliente SQL no sirve. Ejecute `sql/uninstall.sql` antes de retirar la biblioteca.

### Coste y límites

La caché pertenece a cada expresión SQL: textos, condiciones y modelo idénticos reutilizan el veredicto. Cada fila no almacenada genera una llamada HTTPS; no hay índice semántico ni procesamiento por lotes entre filas. `JEV_MAX_REQUESTS` permite 1.000 llamadas por expresión de forma predeterminada y `JEV_TIMEOUT_MS` vale 10.000 ms. Límites: texto de 32 KiB, ocho condiciones de 8 KiB, respuesta de 1 MiB. Reduzca las filas candidatas con columnas indexadas normales y revise el plan. Los textos y condiciones enviados a Jev salen del servidor MariaDB. Evite escrituras automáticas y replicación basada en instrucciones SQL con una función de inferencia.

## Configuration / Configuration / Configuración

| Variable | Default / Défaut / Predeterminado |
| --- | --- |
| `JEV_API_KEY` or `TYPESAFE_API_KEY` | required / requis / obligatorio |
| `JEV_MODEL` | `jev-1.13.0` |
| `JEV_API_URL` | `https://api.typesafe.ai/v1/systemone` |
| `JEV_TIMEOUT_MS` | `10000` |
| `JEV_MAX_REQUESTS` | `1000` |

The endpoint protocol follows the [TypeSafe API reference](https://docs.typesafe.ai/api.md). The tests use synthetic data and a mock endpoint, including a SQL test inside MariaDB in CI; they do not measure live Jev accuracy or throughput.

Le protocole suit la [référence TypeSafe](https://docs.typesafe.ai/api.md). Les tests utilisent des données synthétiques et une API simulée, y compris un test SQL dans MariaDB en CI ; ils ne mesurent ni la précision ni le débit de Jev en production.

El protocolo sigue la [referencia de TypeSafe](https://docs.typesafe.ai/api.md). Las pruebas usan datos sintéticos y una API simulada, incluida una prueba SQL en MariaDB en CI; no miden la precisión ni el rendimiento reales de Jev.

Licence MIT. Sans affiliation avec TypeSafe ou MariaDB.

MIT license. Not affiliated with TypeSafe or MariaDB.

Licencia MIT. Sin afiliación con TypeSafe ni MariaDB.
