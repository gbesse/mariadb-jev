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

## Contrôle d’adoption · Adoption check · Comprobación de adopción

[Français : essayer un cas concret](examples/adoption-check.md) · [English: try a concrete case](examples/adoption-check.md) · [Español: pruebe un caso concreto](examples/adoption-check.md).

## Strict text preflight / Prévalidation du texte / Validación previa del texto

EN: Invalid UTF-8 in SQL values, conditions or model names now fails before HTTP dispatch instead of becoming an omitted JSON field. Rejected values consume no request budget and cannot warm the cache. Engine regression assertions stay active in Release builds, so the default CI build actually checks the positive routing/cache assertions. Native Linux tests and MariaDB mock integration are required before release; no production database or live account is used.

FR : Le UTF-8 invalide dans les valeurs SQL, conditions ou noms de modèle échoue désormais avant l’envoi HTTP au lieu de devenir un champ JSON omis. Un rejet ne consomme aucun budget et n’alimente pas le cache. Les assertions moteur restent actives en build Release : le CI par défaut vérifie réellement routage et cache. Tests Linux natifs et intégration MariaDB simulée sont requis avant release ; aucune base de production ni compte réel n’est utilisé.

ES: UTF-8 incorrecto en valores SQL, condiciones o nombres de modelo falla antes de enviar HTTP en lugar de omitir campos JSON. Un rechazo no consume presupuesto ni alimenta la caché. Aserciones del motor siguen activas en Release: el CI comprueba realmente rutas y caché. Se requieren pruebas Linux nativas e integración MariaDB simulada antes de publicar; no usa bases de producción ni cuentas reales.


FR : Le parseur exige 1–8 questions et exactement autant de réponses avant allocation du vecteur de scores. Le client refuse un modèle déclaré différent du modèle demandé, y compris un modèle déclaré nul. L’absence historique du champ `model` reste acceptée : ce contrôle ne prouve donc pas l’identité du modèle distant. La CI compile en Release et teste le moteur ainsi que les UDF dans un vrai MariaDB avec HTTP simulé ; pas de modèle réel ni de mesure de débit.

EN: The parser requires 1–8 questions and exactly that many answers before allocating score storage. Clients reject a declared model differing from the requested model, including a declared null model. Legacy replies omitting `model` remain accepted, so this does not prove remote model identity. CI builds Release and tests the engine and UDFs in actual MariaDB with mock HTTP; no live model or throughput measurement.

ES: El parser exige 1–8 preguntas y exactamente tantas respuestas antes de reservar puntuaciones. El cliente rechaza un modelo declarado distinto del solicitado, incluido un modelo nulo. Se siguen aceptando respuestas antiguas sin `model`; no demuestra identidad del modelo remoto. CI compila Release y prueba motor y UDF en MariaDB real con HTTP simulado; sin modelo real ni medir rendimiento.

## Destination HTTP parsée / Parsed HTTP endpoint / Destino HTTP analizado

FR — La destination est validée avec l’analyseur libcurl (minimum 7.62), dans la configuration d’environnement et dans le constructeur C++. HTTPS est autorisé ; HTTP exige un hôte exactement `127.0.0.1`. Les identifiants intégrés, fragments, paramètres de transport invalides et clés contenant CR/LF sont refusés avant toute requête. Un préfixe ressemblant à une adresse locale ne suffit plus. Aucun corps, URL ou secret invalide n’est reproduit dans l’erreur.

EN — Endpoints are validated with the libcurl parser (minimum 7.62), both for environment settings and direct C++ construction. HTTPS is allowed; HTTP requires exactly the host `127.0.0.1`. Embedded credentials, fragments, invalid transport limits and CR/LF-bearing keys are rejected before requests. A loopback-looking prefix is insufficient. Errors reproduce no invalid body, URL or secret.

ES — Destinos se validan con el analizador libcurl (mínimo 7.62), tanto en entorno como constructor C++. HTTPS está permitido; HTTP exige host exactamente `127.0.0.1`. Credenciales integradas, fragmentos, límites inválidos y claves con CR/LF se rechazan antes de peticiones. Un prefijo que parezca local no basta. Errores no reproducen cuerpo, URL ni secreto inválido.
