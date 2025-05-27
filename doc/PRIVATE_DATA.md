# Private data

Private data is left entirely to the developer. Typical definitions follow.

The function pointers provide an abstraction with two major benefits:

- it provides an abstraction layer between the PostgreSQL-specific code in
  the barebones FDW implementation and the 'business logic' of the FDW.
- it defines the functions that must be implemented.

The actual construction can occur in either the FDW function or an
intermediate builder. In general the latter approach is preferable
if more than a single table is supported.

## Limitations

It is not clear to me how to use the plans in:

```C
void
GetForeignPaths(
    PlannerInfo *root,
    RelOptInfo *rel,
    Oid foreigntableid)
    
ForeignScan *
GetForeignPlan(
    PlannerInfo *root,
    const RelOptInfo *rel,
    Oid foreigntableid,
    ForeignPath *best_path,
    List *tlist,
    List *restrictinfo_list,
    Plan *outer_plan)
```

## Constuctors

```C
BlackholeFdwPlanState *create_blackhole_plan_state(
    PlannerInfo *root,
    RelOptInfo *rel;
    Oid foreigntableoid);

BlackholeFdwScanState *create_blackhole_scan_state(
    ForeignScanState *node,
    int eflags);
    
// TBD
BlackholeFdwModifyState *create_blackhole_modify_state(void);
```

## FdwPlanState

```C
/*
 * The plan state is set up in blackholeGetForeignRelSize and stashed away in
 * baserel->fdw_private and fetched in blackholeGetForeignPaths.
 */
typedef struct {
    void (*open)(void);   // GetForeignRelSize
    void (*close)(void);  // ?

    char *foo;
    int bar;
} BlackholeFdwPlanState;
```

## FdwScanState

```C
/*
 * The scan state is for maintaining state for a scan, eiher for a
 * SELECT or UPDATE or DELETE.
 *
 * It is set up in blackholeBeginForeignScan and stashed in node->fdw_state
 * and subsequently used in blackholeIterateForeignScan,
 * blackholeEndForeignScan and blackholeReScanForeignScan.
 */
typedef struct {
    void (*open)(void);                  // BeginForeignScan
    void (*next)(TupleTableSlot *slot);  // BeginIterateForeignScan
    void (*reset)(void);                 // ReScanForeignScan
    void (*close)(void);                 // EndForeignScan
    
    char *baz;
    int blurfl;
} BlackholeFdwScanState;
```

## FdwModifyState

```C
/*
 * The modify state is for maintaining state of modify operations.
 *
 * It is set up in blackholeBeginForeignModify and stashed in
 * rinfo->ri_FdwState and subsequently used in blackholeExecForeignInsert,
 * blackholeExecForeignUpdate, blackholeExecForeignDelete and
 * blackholeEndForeignModify.
 */
typedef struct {
    void (*open)(void);
    void (*close)(void);
    
    char *chimp;
    int chump;
} BlackholeFdwModifyState;
```