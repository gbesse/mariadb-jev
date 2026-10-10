# mariadb-jev — contrôle d’adoption · adoption check · comprobación de adopción

## Français

Point de départ local, après la préparation indiquée dans le README :

```sh
cat sql/bounded-review-example.sql
```

Filtrez et limitez d’abord les lignes admissibles, puis seulement évaluez les prédicats Jev. Comptez le maximum d’appels payants avant d’exécuter la requête sur un serveur configuré.

## English

Local starting point, after the setup described in the README:

```sh
cat sql/bounded-review-example.sql
```

Filter and limit eligible rows first, then evaluate Jev predicates. Count the maximum paid calls before running the query on a configured server.

## Español

Punto de partida local, después de la preparación descrita en el README:

```sh
cat sql/bounded-review-example.sql
```

Filtre y limite primero las filas elegibles y después evalúe los predicados Jev. Cuente el máximo de llamadas de pago antes de ejecutar la consulta en un servidor configurado.
## Variante synthétique · Synthetic variation · Variante sintética

```text
eligible_rows=5; max_paid_calls=5; null_input_calls=0
```

FR : adaptez une copie de la fixture locale à cette situation, puis vérifiez le comportement décrit ci-dessus. Les valeurs sont illustratives, pas des résultats Jev mesurés.

EN: adapt a copy of the local fixture to this situation, then check the behavior described above. Values are illustrative, not measured Jev output.

ES: adapte una copia de la fixture local a esta situación y compruebe el comportamiento descrito arriba. Los valores son ilustrativos, no resultados Jev medidos.

## Second cas · Second case · Segundo caso

```text
input_text=NULL; predicate=jev_probability
```

**FR :** Une entrée SQL `NULL` doit renvoyer `NULL` sans appel payant. Vérifiez ce comportement sur un serveur configuré avant d’utiliser la fonction dans une requête volumineuse.

**EN:** A SQL `NULL` input should return `NULL` without a paid call. Confirm this on a configured server before using the function in a large query.

**ES:** Una entrada SQL `NULL` debe devolver `NULL` sin llamada de pago. Confírmelo en un servidor configurado antes de usar la función en una consulta grande.
