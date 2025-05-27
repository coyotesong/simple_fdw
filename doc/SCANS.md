# Foreign scan types

This page documents the scan-related classes.

## Structures required for Read-Only access
| Struct              | Location                        |
|---------------------|---------------------------------|
| `ExprState`         | `nodes/execnodes.h` (line 78)   |
| `PlanState`         | `nodes/execnodes.h` (line 1108) |                       
| `ScanState`         | `nodes/execnodes.h` (line 1552) |
| `TableScanDescData` | `access/relscan.h`	 (line 31)   |

## ExprState

```C
typedef struct ExprState
{
    NodeTag     type;

    uint8       flags;          /* bitmask of EEO_FLAG_* bits, see above */

    /*
     * Storage for result value of a scalar expression, or for individual
     * column results within expressions built by ExecBuildProjectionInfo().
     */
#define FIELDNO_EXPRSTATE_RESNULL 2
    bool        resnull;
#define FIELDNO_EXPRSTATE_RESVALUE 3
    Datum       resvalue;

    /*
     * If projecting a tuple result, this slot holds the result; else NULL.
     */
#define FIELDNO_EXPRSTATE_RESULTSLOT 4
    TupleTableSlot *resultslot;

    /*
     * Instructions to compute expression's return value.
     */ 
    struct ExprEvalStep *steps;
    
    /*
     * Function that actually evaluates the expression.  This can be set to
     * different values depending on the complexity of the expression.
     */
    ExprStateEvalFunc evalfunc;

    /* original expression tree, for debugging only */
    Expr       *expr;

    /* private state for an evalfunc */
    void       *evalfunc_private;

    /*
     * XXX: following fields only needed during "compilation" (ExecInitExpr);
     * could be thrown away afterwards.
     */

    int         steps_len;      /* number of steps currently */
    int         steps_alloc;    /* allocated length of steps array */

#define FIELDNO_EXPRSTATE_PARENT 11
    struct PlanState *parent;   /* parent PlanState node, if any */
    ParamListInfo ext_params;   /* for compiling PARAM_EXTERN nodes */

    Datum      *innermost_caseval;
    bool       *innermost_casenull;

    Datum      *innermost_domainval;
    bool       *innermost_domainnull;

    /*
     * For expression nodes that support soft errors. Should be set to NULL if
     * the caller wants errors to be thrown. Callers that do not want errors
     * thrown should set it to a valid ErrorSaveContext before calling
     * ExecInitExprRec().
     */
    ErrorSaveContext *escontext;
} ExprState;
```

## PlanState

```C
/*
 * We never actually instantiate any PlanState nodes; this is just the common
 * abstract superclass for all PlanState-type nodes.
 * ----------------
 */
typedef struct PlanState
{
    pg_node_attr(abstract)

    NodeTag     type;

    Plan       *plan;           /* associated Plan node */

    EState     *state;          /* at execution time, states of individual
                                 * nodes point to one EState for the whole
                                 * top-level plan */

    ExecProcNodeMtd ExecProcNode;   /* function to return next tuple */
    ExecProcNodeMtd ExecProcNodeReal;   /* actual function, if above is a
                                         * wrapper */

    Instrumentation *instrument;    /* Optional runtime stats for this node */
    WorkerInstrumentation *worker_instrument;   /* per-worker instrumentation */

    /* Per-worker JIT instrumentation */
    struct SharedJitInstrumentation *worker_jit_instrument;

    /*
     * Common structural data for all Plan types.  These links to subsidiary
     * state trees parallel links in the associated plan tree (except for the
     * subPlan list, which does not exist in the plan tree).
     */
    ExprState  *qual;           /* boolean qual condition */
    struct PlanState *lefttree; /* input plan tree(s) */
    struct PlanState *righttree;

    List       *initPlan;       /* Init SubPlanState nodes (un-correlated expr
                                 * subselects) */
    List       *subPlan;        /* SubPlanState nodes in my expressions */

    /*
     * State for management of parameter-change-driven rescanning
     */
    Bitmapset  *chgParam;       /* set of IDs of changed Params */

    /*
     * Other run-time state needed by most if not all node types.
     */
    TupleDesc   ps_ResultTupleDesc; /* node's return type */
    TupleTableSlot *ps_ResultTupleSlot; /* slot for my result tuples */
    ExprContext *ps_ExprContext;    /* node's expression-evaluation context */
    ProjectionInfo *ps_ProjInfo;    /* info for doing tuple projection */

    bool        async_capable;  /* true if node is async-capable */

    /*
     * Scanslot's descriptor if known. This is a bit of a hack, but otherwise
     * it's hard for expression compilation to optimize based on the
     * descriptor, without encoding knowledge about all executor nodes.
     */
    TupleDesc   scandesc;

    /*
     * Define the slot types for inner, outer and scanslots for expression
     * contexts with this state as a parent.  If *opsset is set, then
     * *opsfixed indicates whether *ops is guaranteed to be the type of slot
     * used. That means that every slot in the corresponding
     * ExprContext.ecxt_*tuple will point to a slot of that type, while
     * evaluating the expression.  If *opsfixed is false, but *ops is set,
     * that indicates the most likely type of slot.
     *
     * The scan* fields are set by ExecInitScanTupleSlot(). If that's not
     * called, nodes can initialize the fields themselves.
     *
     * If outer/inneropsset is false, the information is inferred on-demand
     * using ExecGetResultSlotOps() on ->righttree/lefttree, using the
     * corresponding node's resultops* fields.
     *
     * The result* fields are automatically set when ExecInitResultSlot is
     * used (be it directly or when the slot is created by
     * ExecAssignScanProjectionInfo() /
     * ExecConditionalAssignProjectionInfo()).  If no projection is necessary
     * ExecConditionalAssignProjectionInfo() defaults those fields to the scan
     * operations.
     */
    const TupleTableSlotOps *scanops;
    const TupleTableSlotOps *outerops;
    const TupleTableSlotOps *innerops;
    const TupleTableSlotOps *resultops;
    bool        scanopsfixed;
    bool        outeropsfixed;
    bool        inneropsfixed;
    bool        resultopsfixed;
    bool        scanopsset;
    bool        outeropsset;
    bool        inneropsset;
    bool        resultopsset;
} PlanState;
```

## ScanState

```C
/* ----------------
 *   ScanState information
 *
 *      ScanState extends PlanState for node types that represent
 *      scans of an underlying relation.  It can also be used for nodes
 *      that scan the output of an underlying plan node --- in that case,
 *      only ScanTupleSlot is actually useful, and it refers to the tuple
 *      retrieved from the subplan.
 *
 *      currentRelation    relation being scanned (NULL if none)
 *      currentScanDesc    current scan descriptor for scan (NULL if none)
 *      ScanTupleSlot      pointer to slot in tuple table holding scan tuple
 * ----------------
 */
typedef struct ScanState
{
    PlanState   ps;             /* its first field is NodeTag */
    Relation    ss_currentRelation;
    struct TableScanDescData *ss_currentScanDesc;
    TupleTableSlot *ss_ScanTupleSlot;
} ScanState;
```

## TableScanDescData

```C
/*
 * Generic descriptor for table scans. This is the base-class for table scans,
 * which needs to be embedded in the scans of individual AMs.
 */
typedef struct TableScanDescData
{
    /* scan parameters */
    Relation    rs_rd;          /* heap relation descriptor */
    struct SnapshotData *rs_snapshot;   /* snapshot to see */
    int         rs_nkeys;       /* number of scan keys */
    struct ScanKeyData *rs_key; /* array of scan key descriptors */

    /* Range of ItemPointers for table_scan_getnextslot_tidrange() to scan. */
    ItemPointerData rs_mintid;
    ItemPointerData rs_maxtid;

    /*
     * Information about type and behaviour of the scan, a bitmask of members
     * of the ScanOptions enum (see tableam.h).
     */
    uint32      rs_flags;

    struct ParallelTableScanDescData *rs_parallel;  /* parallel scan
                                                     * information */
} TableScanDescData;
typedef struct TableScanDescData *TableScanDesc;
```
