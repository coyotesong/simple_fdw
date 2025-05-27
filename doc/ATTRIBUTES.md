# Attributes

## Structures

| Struct                  | Location                           |
|-------------------------|------------------------------------|
| `FormData_pg_attribute` | `catalog/pg_attribute.h` (line 28) |

## FormData_pg_attribute

Attribute details. THe unusual name is due to postgresql implementation details.

See header file for more detailed documentation.

```C
CATALOG(pg_attribute,1249,AttributeRelationId) BKI_BOOTSTRAP BKI_ROWTYPE_OID(75,AttributeRelation_Rowtype_Id) BKI_SCHEMA_MACRO
{
    Oid         attrelid BKI_LOOKUP(pg_class);     // OID of relation
    NameData    attname;                           // name
    Oid         atttypid BKI_LOOKUP_OPT(pg_type);  // Catalog Class pg_type
    int16       attlen;                            // pg_type->typlen
    int16       attnum;
    
    int32       attcacheoff BKI_DEFAULT(-1);
    int32       atttypmod BKI_DEFAULT(-1);
    
    int16       attndims;                          // 0, unless Array type
    bool        attbyval;                          // pg_type->typbyval
    char        attalign;                          // pg_type->typalign
    
    char        attstorage;                        // for VARLENA attributes
    char        attcompression BKI_DEFAULT('\0');
    
    bool        attnotnull;                        // 'NOT NULL' constraint
    bool        atthasdef BKI_DEFAULT(f);          // has 'DEFAULT'
    bool        atthasmissing BKI_DEFAULT(f);      // allows null
    
    char        attidentity BKI_DEFAULT('\0');
    char        attgenerated BKI_DEFAULT('\0');
    bool        attisdropped BKI_DEFAULT(f);
    bool        attislocal BKI_DEFAULT(t);
    int16       attinhcount BKI_DEFAULT(0);
    Oid         attcollation BKI_LOOKUP_OPT(pg_collation);

#ifdef CATALOG_VARLEN           
    /* variable-length/nullable fields start here */
    /* NOTE: The following fields are not present in tuple descriptors. */

    int16       attstattarget BKI_DEFAULT(_null_) BKI_FORCE_NULL;
    aclitem     attacl[1] BKI_DEFAULT(_null_);
    text        attoptions[1] BKI_DEFAULT(_null_);
    text        attfdwoptions[1] BKI_DEFAULT(_null_);
    anyarray    attmissingval BKI_DEFAULT(_null_);
#endif

} FormData_pg_attribute;
``