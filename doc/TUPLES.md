# Tuples

Tuples are largely the column's metadata, with the actual data
stored in the `TupleTableSlot`.

## Structures required for Read-Only access

| Struct                  | Location                         |
|-------------------------|----------------------------------|
| `ItemPointerData`       | `storage/itemptr.h` (line 20)    |
| `MemoryContextData`     | `nodes/memnodes.h` (line 117)    |
| `TupleDesc`             | `access/tupdesc.h` (line 48)     | 
| `TupleTableSlot`        | `executor/tuptable.h` (line 131) |

A custom `TupleTableSlot` struct can be implemented using the
`TupleTableSlotOps` API. This may be useful with memory-mapped
resources since they:

- eliminate the need to maintain an additional buffer
- are much faster since they've skipped an unnecessary file copy


## TupleDesc

```C
/*
 * This struct is passed around within the backend to describe the structure
 * of tuples.  For tuples coming from on-disk relations, the information is
 * collected from the pg_attribute, pg_attrdef, and pg_constraint catalogs.
 * Transient row types (such as the result of a join query) have anonymous
 * TupleDesc structs that generally omit any constraint info; therefore the
 * structure is designed to let the constraints be omitted efficiently.
 *
 * Note that only user attributes, not system attributes, are mentioned in
 * TupleDesc.
 *
 * If the tupdesc is known to correspond to a named rowtype (such as a table's
 * rowtype) then tdtypeid identifies that type and tdtypmod is -1.  Otherwise
 * tdtypeid is RECORDOID, and tdtypmod can be either -1 for a fully anonymous
 * row type, or a value >= 0 to allow the rowtype to be looked up in the
 * typcache.c type cache.
 *
 * Note that tdtypeid is never the OID of a domain over composite, even if
 * we are dealing with values that are known (at some higher level) to be of
 * a domain-over-composite type.  This is because tdtypeid/tdtypmod need to
 * match up with the type labeling of composite Datums, and those are never
 * explicitly marked as being of a domain type, either.
 *
 * Tuple descriptors that live in caches (relcache or typcache, at present)
 * are reference-counted: they can be deleted when their reference count goes
 * to zero.  Tuple descriptors created by the executor need no reference
 * counting, however: they are simply created in the appropriate memory
 * context and go away when the context is freed.  We set the tdrefcount
 * field of such a descriptor to -1, while reference-counted descriptors
 * always have tdrefcount >= 0.
 */
```

```C
typedef struct TupleDescData
{
    int         natts;          /* number of attributes in the tuple */
    Oid         tdtypeid;       /* composite type ID for tuple type */
    int32       tdtypmod;       /* typmod for tuple type */
    int         tdrefcount;     /* reference count, or -1 if not counting */
    TupleConstr *constr;        /* constraints, or NULL if none */
    /* attrs[N] is the description of Attribute Number N+1 */
    FormData_pg_attribute attrs[FLEXIBLE_ARRAY_MEMBER];
}           TupleDescData;
typedef struct TupleDescData *TupleDesc;
```

## TupleTableSlot

There are four types of TupleTableSlot:

- `Virtual`
- `Heap`
- `BufferHeap`
- `Minimal`

```C
typedef struct TupleTableSlot
{
    NodeTag     type;
#define FIELDNO_TUPLETABLESLOT_FLAGS 1
    uint16      tts_flags;      /* Boolean states */
#define FIELDNO_TUPLETABLESLOT_NVALID 2
    AttrNumber  tts_nvalid;     /* # of valid values in tts_values */
    const TupleTableSlotOps *const tts_ops; /* implementation of slot */
#define FIELDNO_TUPLETABLESLOT_TUPLEDESCRIPTOR 4
    TupleDesc   tts_tupleDescriptor;    /* slot's tuple descriptor */
#define FIELDNO_TUPLETABLESLOT_VALUES 5
    Datum      *tts_values;     /* current per-attribute values */
#define FIELDNO_TUPLETABLESLOT_ISNULL 6
    bool       *tts_isnull;     /* current per-attribute isnull flags */
    MemoryContext tts_mcxt;     /* slot itself is in this context */
    ItemPointerData tts_tid;    /* stored tuple's tid */
    Oid         tts_tableOid;   /* table oid of tuple */
} TupleTableSlot;
```

### VirtualTupleTableSlot

```C
typedef struct VirtualTupleTableSlot
{
    pg_node_attr(abstract)

    TupleTableSlot base;

    char       *data;           /* data for materialized slots */
} VirtualTupleTableSlot;
```


### HeapTupleTableSlot

```C
typedef struct HeapTupleTableSlot
{
    pg_node_attr(abstract)

    TupleTableSlot base;

#define FIELDNO_HEAPTUPLETABLESLOT_TUPLE 1
    HeapTuple   tuple;          /* physical tuple */
#define FIELDNO_HEAPTUPLETABLESLOT_OFF 2
    uint32      off;            /* saved state for slot_deform_heap_tuple */
    HeapTupleData tupdata;      /* optional workspace for storing tuple */
} HeapTupleTableSlot;
```

### BufferHeapTupleTableSlot

```C
typedef struct BufferHeapTupleTableSlot
{
    pg_node_attr(abstract)

    HeapTupleTableSlot base;

    /*
     * If buffer is not InvalidBuffer, then the slot is holding a pin on the
     * indicated buffer page; drop the pin when we release the slot's
     * reference to that buffer.  (TTS_FLAG_SHOULDFREE should not be set in
     * such a case, since presumably base.tuple is pointing into the buffer.)
     */
    Buffer      buffer;         /* tuple's buffer, or InvalidBuffer */
} BufferHeapTupleTableSlot;
```

### MinimalTupleTableSlot

```C
typedef struct MinimalTupleTableSlot
{
    pg_node_attr(abstract)

    TupleTableSlot base;

    /*
     * In a minimal slot tuple points at minhdr and the fields of that struct
     * are set correctly for access to the minimal tuple; in particular,
     * minhdr.t_data points MINIMAL_TUPLE_OFFSET bytes before mintuple.  This
     * allows column extraction to treat the case identically to regular
     * physical tuples.
     */
#define FIELDNO_MINIMALTUPLETABLESLOT_TUPLE 1
    HeapTuple   tuple;          /* tuple wrapper */
    MinimalTuple mintuple;      /* minimal tuple, or NULL if none */
    HeapTupleData minhdr;       /* workspace for minimal-tuple-only case */
#define FIELDNO_MINIMALTUPLETABLESLOT_OFF 4
    uint32      off;            /* saved state for slot_deform_heap_tuple */
} MinimalTupleTableSlot;
```

## Custom implementation

A custom implementation must implement these functions. See header file for
more detailed documentation.

```C
struct TupleTableSlotOps
{
    /* Minimum size of the slot */
    size_t      base_slot_size;

    /* Initialization. */
    void        (*init) (TupleTableSlot *slot);

    /* Destruction. */
    void        (*release) (TupleTableSlot *slot);

    /* Reset */
    void        (*clear) (TupleTableSlot *slot);

    void        (*getsomeattrs) (TupleTableSlot *slot, int natts);
    Datum       (*getsysattr) (TupleTableSlot *slot, int attnum, bool *isnull);
    bool        (*is_current_xact_tuple) (TupleTableSlot *slot);
    void        (*materialize) (TupleTableSlot *slot);
    void        (*copyslot) (TupleTableSlot *dstslot, TupleTableSlot *srcslot);
    HeapTuple   (*get_heap_tuple) (TupleTableSlot *slot);
    MinimalTuple (*get_minimal_tuple) (TupleTableSlot *slot);
    HeapTuple   (*copy_heap_tuple) (TupleTableSlot *slot);
    MinimalTuple (*copy_minimal_tuple) (TupleTableSlot *slot);
};
```

### ItemPointerData

```C
/*
 * ItemPointer:
 *
 * This is a pointer to an item within a disk page of a known file
 * (for example, a cross-link from an index to its parent table).
 * ip_blkid tells us which block, ip_posid tells us which entry in
 * the linp (ItemIdData) array we want.
 *
 * Note: because there is an item pointer in each tuple header and index
 * tuple header on disk, it's very important not to waste space with
 * structure padding bytes.  The struct is designed to be six bytes long
 * (it contains three int16 fields) but a few compilers will pad it to
 * eight bytes unless coerced.  We apply appropriate persuasion where
 * possible.  If your compiler can't be made to play along, you'll waste
 * lots of space.
 */
typedef struct ItemPointerData
{
    BlockIdData ip_blkid;
    OffsetNumber ip_posid;
}
```

### MemoryContext and MemoryContextData

From `utils/palloc.h`:

```C
typedef struct MemoryContextData *MemoryContext;
```

Definition.

```C
typedef struct MemoryContextData
{
    pg_node_attr(abstract)      /* there are no nodes of this type */

    NodeTag     type;           /* identifies exact kind of context */
    /* these two fields are placed here to minimize alignment wastage: */
    bool        isReset;        /* T = no space alloced since last reset */
    bool        allowInCritSection; /* allow palloc in critical section */
    Size        mem_allocated;  /* track memory allocated for this context */
    const MemoryContextMethods *methods;    /* virtual function table */
    MemoryContext parent;       /* NULL if no parent (toplevel context) */
    MemoryContext firstchild;   /* head of linked list of children */
    MemoryContext prevchild;    /* previous child of same parent */
    MemoryContext nextchild;    /* next child of same parent */
    const char *name;           /* context name (just for debugging) */
    const char *ident;          /* context ID if any (just for debugging) */
    MemoryContextCallback *reset_cbs;   /* list of reset/delete callbacks */
} MemoryContextData;
```