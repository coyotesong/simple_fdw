# Foreign scan types

This page documents the foreign scan-related classes.

## Structures required for Read-Only access
| Struct              | Location                        |
|---------------------|---------------------------------|
| `ForeignScanState`  | `nodes/execnodes.h` (line 2034) |

## ForeignScanState

```C
typedef struct ForeignScanState
{
    ScanState   ss;             /* its first field is NodeTag */
    ExprState  *fdw_recheck_quals;  /* original quals not in ss.ps.qual */
    Size        pscan_len;      /* size of parallel coordination information */
    ResultRelInfo *resultRelInfo;   /* result rel info, if UPDATE or DELETE */
    /* use struct pointer to avoid including fdwapi.h here */
    struct FdwRoutine *fdwroutine;
    void       *fdw_state;      /* foreign-data wrapper can keep state here */
} ForeignScanState;
```
