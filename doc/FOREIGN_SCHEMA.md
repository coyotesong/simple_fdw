# Foreign schema types

This page documents the foreign schema-related classes.

## Structures required for Read-Only access
| Struct                    | Location                         |
|---------------------------|----------------------------------|
| `Alias`                   | `nodes/primnodes.h` (line 36)    |
| `ImportForeignSchemaStmt` | `nodes/parsenodes.h` (line 2953) |
| `RangeVar`                | `nodes/primnodes.h` (line 63)    |

## Alias

```C
/*
 * Alias -
 *    specifies an alias for a range variable; the alias might also
 *    specify renaming of columns within the table.
 *
 * Note: colnames is a list of String nodes.  In Alias structs
 * associated with RTEs, there may be entries corresponding to dropped
 * columns; these are normally empty strings ("").  See parsenodes.h for info.
 */
typedef struct Alias
{
    NodeTag     type;
    char       *aliasname;      /* aliased rel name (never qualified) */
    List       *colnames;       /* optional list of column aliases */
} Alias;
```

## ImportForeignSchemaStmt

This is only required if we provide an import schema. (A very good idea!)

```C
typedef enum ImportForeignSchemaType
{
    FDW_IMPORT_SCHEMA_ALL,      /* all relations wanted */
    FDW_IMPORT_SCHEMA_LIMIT_TO, /* include only listed tables in import */
    FDW_IMPORT_SCHEMA_EXCEPT,   /* exclude listed tables from import */
} ImportForeignSchemaType;
    
typedef struct ImportForeignSchemaStmt
{ 
    NodeTag     type;
    char       *server_name;    /* FDW server name */
    char       *remote_schema;  /* remote schema name to query */
    char       *local_schema;   /* local schema to create objects in */
    ImportForeignSchemaType list_type;  /* type of table list */
    List       *table_list;     /* List of RangeVar */
    List       *options;        /* list of options to pass to FDW */
} ImportForeignSchemaStmt;
```

## RangeVar

```C
/*
 * RangeVar - range variable, used in FROM clauses
 *
 * Also used to represent table names in utility statements; there, the alias
 * field is not used, and inh tells whether to apply the operation
 * recursively to child tables.  In some contexts it is also useful to carry
 * a TEMP table indication here.
 */
typedef struct RangeVar
{
    NodeTag     type;

    /* the catalog (database) name, or NULL */
    char       *catalogname;

    /* the schema name, or NULL */
    char       *schemaname;

    /* the relation/sequence name */
    char       *relname;

    /* expand rel by inheritance? recursively act on children? */
    bool        inh;

    /* see RELPERSISTENCE_* in pg_class.h */
    char        relpersistence;

    /* table alias & optional column aliases */
    Alias      *alias;

    /* token location, or -1 if unknown */
    ParseLoc    location;
} RangeVar;
```