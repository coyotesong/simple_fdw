# General types

## Structures

All definitions are in `c.h`
 
| Struct       | Definition     |
|--------------|----------------|
| `AttrNumber` | `int16`        |
| `Buffer`     | `int`          |
| `Datum`      | `uintprt_t`    |
| `Index`      | `unigned int`  |
| `Oid`        | `unsigned int` |
| `Size`       | `size_t`       |

| Struct       | Location                    |
|--------------|-----------------------------|
| `List`       | `nodes/pg_list.h` (line 98) |
| `NameData`   | `c.h` (line 736)            |
| `NodeTag`    | `nodes/nodes.h` (line 17)   |

```C
/*
 * historical - basically a fully populated C string
 */
typedef struct nameData {
    char        data[NAMEDATALEN];
   } NameData;
typedef NameData *Name;

#define NameStr(name)   ((name).data)
```

## List

    One important concession to the original implementation is that an empty
    list is always represented by a null pointer (preferentially written NIL).
    Non-empty lists have a header, which will not be relocated as long as the
    list remains non-empty, and an expansible data array.
   
    We support four types of lists:
   
     T_List: lists of pointers
         (in practice usually pointers to Nodes, but not always;
         declared as "void *" to minimize casting annoyances)
     T_IntList: lists of integers
     T_OidList: lists of Oids
     T_XidList: lists of TransactionIds
         (the XidList infrastructure is less complete than the other cases)
   
    (At the moment, ints, Oids, and XIDs are the same size, but they may not
    always be so; be careful to use the appropriate list type for your data.)

```C
typedef union ListCell
{
    void       *ptr_value;
    int         int_value;
    Oid         oid_value;
    TransactionId xid_value;
} ListCell;
```

```C
typedef struct List
{
    NodeTag     type;           /* T_List, T_IntList, T_OidList, or T_XidList */
    int         length;         /* number of elements currently present */
    int         max_length;     /* allocated length of elements[] */
    ListCell   *elements;       /* re-allocatable array of cells */
    /* We may allocate some cells along with the List header: */
    ListCell    initial_elements[FLEXIBLE_ARRAY_MEMBER];
    /* If elements == initial_elements, it's not a separate allocation */
} List;
```

## NamedData

```C
/*
 * Representation of a Name: effectively just a C string, but null-padded to
 * exactly NAMEDATALEN bytes.  The use of a struct is historical.
 */
typedef struct nameData
{
    char        data[NAMEDATALEN];
} NameData;
typedef NameData *Name;

#define NameStr(name)   ((name).data)
```

## NodeTag

```C
/*
 * The first field of every node is NodeTag. Each node created (with makeNode)
 * will have one of the following tags as the value of its first field.
 *
 * Note that inserting or deleting node types changes the numbers of other
 * node types later in the list.  This is no problem during development, since
 * the node numbers are never stored on disk.  But don't do it in a released
 * branch, because that would represent an ABI break for extensions.
 */
typedef enum NodeTag
{
    T_Invalid = 0,

#include "nodes/nodetags.h"
} NodeTag;
```