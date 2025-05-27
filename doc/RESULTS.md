
## ResultRelInfo

```C
*
 * ResultRelInfo
 *
 * Whenever we update an existing relation, we have to update indexes on the
 * relation, and perhaps also fire triggers.  ResultRelInfo holds all the
 * information needed about a result relation, including indexes.
 *
 * Normally, a ResultRelInfo refers to a table that is in the query's range
 * table; then ri_RangeTableIndex is the RT index and ri_RelationDesc is
 * just a copy of the relevant es_relations[] entry.  However, in some
 * situations we create ResultRelInfos for relations that are not in the
 * range table, namely for targets of tuple routing in a partitioned table,
 * and when firing triggers in tables other than the target tables (See
 * ExecGetTriggerResultRel).  In these situations, ri_RangeTableIndex is 0
 * and ri_RelationDesc is a separately-opened relcache pointer that needs to
 * be separately closed.
 */
typedef struct ResultRelInfo
{
    NodeTag     type;

    /* result relation's range table index, or 0 if not in range table */
    Index       ri_RangeTableIndex;

    /* relation descriptor for result relation */
    Relation    ri_RelationDesc;

    /* # of indices existing on result relation */
    int         ri_NumIndices;

    /* array of relation descriptors for indices */
    RelationPtr ri_IndexRelationDescs;

    /* array of key/attr info for indices */
    IndexInfo **ri_IndexRelationInfo;

    /*
     * For UPDATE/DELETE result relations, the attribute number of the row
     * identity junk attribute in the source plan's output tuples
     */
    AttrNumber  ri_RowIdAttNo;

    /* For UPDATE, attnums of generated columns to be computed */
    Bitmapset  *ri_extraUpdatedCols;

    /* Projection to generate new tuple in an INSERT/UPDATE */
    ProjectionInfo *ri_projectNew;
    /* Slot to hold that tuple */
    TupleTableSlot *ri_newTupleSlot;
    /* Slot to hold the old tuple being updated */
    TupleTableSlot *ri_oldTupleSlot;
    /* Have the projection and the slots above been initialized? */
    bool        ri_projectNewInfoValid;

    /* updates do LockTuple() before oldtup read; see README.tuplock */
    bool        ri_needLockTagTuple;

    /* triggers to be fired, if any */
    TriggerDesc *ri_TrigDesc;

    /* cached lookup info for trigger functions */
    FmgrInfo   *ri_TrigFunctions;

    /* array of trigger WHEN expr states */
    ExprState **ri_TrigWhenExprs;

    /* optional runtime measurements for triggers */
    Instrumentation *ri_TrigInstrument;

    /* On-demand created slots for triggers / returning processing */
    TupleTableSlot *ri_ReturningSlot;   /* for trigger output tuples */
    TupleTableSlot *ri_TrigOldSlot; /* for a trigger's old tuple */
    TupleTableSlot *ri_TrigNewSlot; /* for a trigger's new tuple */

    /* FDW callback functions, if foreign table */
    struct FdwRoutine *ri_FdwRoutine;

    /* available to save private state of FDW */
    void       *ri_FdwState;

    /* true when modifying foreign table directly */
    bool        ri_usesFdwDirectModify;
    
    /* batch insert stuff */
    int         ri_NumSlots;    /* number of slots in the array */
    int         ri_NumSlotsInitialized; /* number of initialized slots */
    int         ri_BatchSize;   /* max slots inserted in a single batch */
    TupleTableSlot **ri_Slots;  /* input tuples for batch insert */
    TupleTableSlot **ri_PlanSlots;

    /* list of WithCheckOption's to be checked */
    List       *ri_WithCheckOptions;

    /* list of WithCheckOption expr states */
    List       *ri_WithCheckOptionExprs;

    /* array of constraint-checking expr states */
    ExprState **ri_ConstraintExprs;

    /* arrays of stored generated columns expr states, for INSERT and UPDATE */
    ExprState **ri_GeneratedExprsI;
    ExprState **ri_GeneratedExprsU;

    /* number of stored generated columns we need to compute */
    int         ri_NumGeneratedNeededI;
    int         ri_NumGeneratedNeededU;

    /* list of RETURNING expressions */
    List       *ri_returningList;

    /* for computing a RETURNING list */
    ProjectionInfo *ri_projectReturning;

    /* list of arbiter indexes to use to check conflicts */
    List       *ri_onConflictArbiterIndexes;

    /* ON CONFLICT evaluation state */
    OnConflictSetState *ri_onConflict;

    /* for MERGE, lists of MergeActionState (one per MergeMatchKind) */
    List       *ri_MergeActions[NUM_MERGE_MATCH_KINDS];

    /* for MERGE, expr state for checking the join condition */
    ExprState  *ri_MergeJoinCondition;

    /* partition check expression state (NULL if not set up yet) */
    ExprState  *ri_PartitionCheckExpr;

    /*
     * Map to convert child result relation tuples to the format of the table
     * actually mentioned in the query (called "root").  Computed only if
     * needed.  A NULL map value indicates that no conversion is needed, so we
     * must have a separate flag to show if the map has been computed.
     */
    TupleConversionMap *ri_ChildToRootMap;
    bool        ri_ChildToRootMapValid;

    /*
     * As above, but in the other direction.
     */
    TupleConversionMap *ri_RootToChildMap;
    bool        ri_RootToChildMapValid;

    /*
     * Information needed by tuple routing target relations
     *
     * RootResultRelInfo gives the target relation mentioned in the query, if
     * it's a partitioned table. It is not set if the target relation
     * mentioned in the query is an inherited table, nor when tuple routing is
     * not needed.
     *
     * PartitionTupleSlot is non-NULL if RootToChild conversion is needed and
     * the relation is a partition.
     */
    struct ResultRelInfo *ri_RootResultRelInfo;
    TupleTableSlot *ri_PartitionTupleSlot;

    /* for use by copyfrom.c when performing multi-inserts */
    struct CopyMultiInsertBuffer *ri_CopyMultiInsertBuffer;

    /*
     * Used when a leaf partition is involved in a cross-partition update of
     * one of its ancestors; see ExecCrossPartitionUpdateForeignKey().
     */
    List       *ri_ancestorResultRels;
} ResultRelInfo;
```