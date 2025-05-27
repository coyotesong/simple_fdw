# Structures

Relations appear to be the tuple's cached representation.

## Structures
 
| Struct         | Location                        |
|----------------|---------------------------------|
| `PathTarget`   | `nodes/pathnodes.h` (line 1595) |
| `RelationData` | `utils/reldata.h` (line 52)     |
| `RelOptInfo`   | `nodes/pathnodes.h` (line 601)  |

## Special note for Foreign Data Wrappers

From the `RelOptInfo` comments:

If the relation is either a foreign table or a join of foreign tables that
all belong to the same foreign server and are assigned to the same user to
check access permissions as (cf checkAsUser), these fields will be set:
   
| Name               | Description                                                      |
|--------------------|------------------------------------------------------------------|
| `serverid`         | `OID` of foreign server, if foreign table (else InvalidOid)      |
| `userid`           | `OID` of user to check access as (InvalidOid means current user) |
| `useridiscurrent ` | we've assumed that userid equals current user                    |
| `fdwroutine`       | function hooks for FDW, if foreign table (else NULL)             |
| `fdw_private`      | private state for FDW, if foreign table (else NULL)              |

## PathTarget

```C

/*
 * PathTarget
 *
 * This struct contains what we need to know during planning about the
 * targetlist (output columns) that a Path will compute.  Each RelOptInfo
 * includes a default PathTarget, which its individual Paths may simply
 * reference.  However, in some cases a Path may compute outputs different
 * from other Paths, and in that case we make a custom PathTarget for it.
 * For example, an indexscan might return index expressions that would
 * otherwise need to be explicitly calculated.  (Note also that "upper"
 * relations generally don't have useful default PathTargets.)
 *  
 * exprs contains bare expressions; they do not have TargetEntry nodes on top,
 * though those will appear in finished Plans.
 *  
 * sortgrouprefs[] is an array of the same length as exprs, containing the
 * corresponding sort/group refnos, or zeroes for expressions not referenced
 * by sort/group clauses.  If sortgrouprefs is NULL (which it generally is in
 * RelOptInfo.reltarget targets; only upper-level Paths contain this info),
 * we have not identified sort/group columns in this tlist.  This allows us to
 * deal with sort/group refnos when needed with less expense than including
 * TargetEntry nodes in the exprs list.
 */
typedef struct PathTarget
{
    pg_node_attr(no_copy_equal, no_read, no_query_jumble)
    
    NodeTag     type;
    
    /* list of expressions to be computed */
    List       *exprs;
    
    /* corresponding sort/group refnos, or 0 */
    Index      *sortgrouprefs pg_node_attr(array_size(exprs));

    /* cost of evaluating the expressions */
    QualCost    cost;
    
    /* estimated avg width of result tuples */
    int         width;
    
    /* indicates if exprs contain any volatile functions */
    VolatileFunctionStatus has_volatile_expr;
} PathTarget;
```

## Relation and RelationData

See source file for additional information.

```C
typedef struct RelationData
{
    RelFileLocator rd_locator;  /* relation physical identifier */
    SMgrRelation rd_smgr;       /* cached file handle, or NULL */
    int         rd_refcnt;      /* reference count */
    ProcNumber  rd_backend;     /* owning backend's proc number, if temp rel */
    bool        rd_islocaltemp; /* rel is a temp rel of this session */
    bool        rd_isnailed;    /* rel is nailed in cache */
    bool        rd_isvalid;     /* relcache entry is valid */
    bool        rd_indexvalid;  /* is rd_indexlist valid? (also rd_pkindex and
                                 * rd_replidindex) */
    bool        rd_statvalid;   /* is rd_statlist valid? */

    /*----------
     * rd_createSubid is the ID of the highest subtransaction the rel has
     * survived into or zero if the rel or its storage was created before the
     * current top transaction.  (IndexStmt.oldNumber leads to the case of a new
     * rel with an old rd_locator.)  rd_firstRelfilelocatorSubid is the ID of the
     * highest subtransaction an rd_locator change has survived into or zero if
     * rd_locator matches the value it had at the start of the current top
     * transaction.  (Rolling back the subtransaction that
     * rd_firstRelfilelocatorSubid denotes would restore rd_locator to the value it
     * had at the start of the current top transaction.  Rolling back any
     * lower subtransaction would not.)  Their accuracy is critical to
     * RelationNeedsWAL().
     *
     * rd_newRelfilelocatorSubid is the ID of the highest subtransaction the
     * most-recent relfilenumber change has survived into or zero if not changed
     * in the current transaction (or we have forgotten changing it).  This
     * field is accurate when non-zero, but it can be zero when a relation has
     * multiple new relfilenumbers within a single transaction, with one of them
     * occurring in a subsequently aborted subtransaction, e.g.
     *      BEGIN;
     *      TRUNCATE t;
     *      SAVEPOINT save;
     *      TRUNCATE t;
     *      ROLLBACK TO save;
     *      -- rd_newRelfilelocatorSubid is now forgotten
     *
     * If every rd_*Subid field is zero, they are read-only outside
     * relcache.c.  Files that trigger rd_locator changes by updating
     * pg_class.reltablespace and/or pg_class.relfilenode call
     * RelationAssumeNewRelfilelocator() to update rd_*Subid.
     *
     * rd_droppedSubid is the ID of the highest subtransaction that a drop of
     * the rel has survived into.  In entries visible outside relcache.c, this
     * is always zero.
     */
    SubTransactionId rd_createSubid;    /* rel was created in current xact */
    SubTransactionId rd_newRelfilelocatorSubid; /* highest subxact changing
                                                 * rd_locator to current value */
    SubTransactionId rd_firstRelfilelocatorSubid;   /* highest subxact
                                                     * changing rd_locator to
                                                     * any value */
    SubTransactionId rd_droppedSubid;   /* dropped with another Subid set */

    Form_pg_class rd_rel;       /* RELATION tuple */
    TupleDesc   rd_att;         /* tuple descriptor */
    Oid         rd_id;          /* relation's object id */
    LockInfoData rd_lockInfo;   /* lock mgr's info for locking relation */
    RuleLock   *rd_rules;       /* rewrite rules */
    MemoryContext rd_rulescxt;  /* private memory cxt for rd_rules, if any */
    TriggerDesc *trigdesc;      /* Trigger info, or NULL if rel has none */
    /* use "struct" here to avoid needing to include rowsecurity.h: */
    struct RowSecurityDesc *rd_rsdesc;  /* row security policies, or NULL */

    /* data managed by RelationGetFKeyList: */
    List       *rd_fkeylist;    /* list of ForeignKeyCacheInfo (see below) */
    bool        rd_fkeyvalid;   /* true if list has been computed */

    /* data managed by RelationGetPartitionKey: */
    PartitionKey rd_partkey;    /* partition key, or NULL */
    MemoryContext rd_partkeycxt;    /* private context for rd_partkey, if any */

    /* data managed by RelationGetPartitionDesc: */
    PartitionDesc rd_partdesc;  /* partition descriptor, or NULL */
    MemoryContext rd_pdcxt;     /* private context for rd_partdesc, if any */

    /*
     * pg_inherits.xmin of the partition that was excluded in
     * rd_partdesc_nodetached.  This informs a future user of that partdesc:
     * if this value is not in progress for the active snapshot, then the
     * partdesc can be used, otherwise they have to build a new one.  (This
     * matches what find_inheritance_children_extended would do).
     */
    TransactionId rd_partdesc_nodetached_xmin;

    /* data managed by RelationGetPartitionQual: */
    List       *rd_partcheck;   /* partition CHECK quals */
    bool        rd_partcheckvalid;  /* true if list has been computed */
    MemoryContext rd_partcheckcxt;  /* private cxt for rd_partcheck, if any */

    /* data managed by RelationGetIndexList: */
    List       *rd_indexlist;   /* list of OIDs of indexes on relation */
    Oid         rd_pkindex;     /* OID of (deferrable?) primary key, if any */
    bool        rd_ispkdeferrable;  /* is rd_pkindex a deferrable PK? */
    Oid         rd_replidindex; /* OID of replica identity index, if any */

    /* data managed by RelationGetStatExtList: */
    List       *rd_statlist;    /* list of OIDs of extended stats */

    /* data managed by RelationGetIndexAttrBitmap: */
    bool        rd_attrsvalid;  /* are bitmaps of attrs valid? */
    Bitmapset  *rd_keyattr;     /* cols that can be ref'd by foreign keys */
    Bitmapset  *rd_pkattr;      /* cols included in primary key */
    Bitmapset  *rd_idattr;      /* included in replica identity index */
    Bitmapset  *rd_hotblockingattr; /* cols blocking HOT update */
    Bitmapset  *rd_summarizedattr;  /* cols indexed by summarizing indexes */

    PublicationDesc *rd_pubdesc;    /* publication descriptor, or NULL */

    /*
     * rd_options is set whenever rd_rel is loaded into the relcache entry.
     * Note that you can NOT look into rd_rel for this data.  NULL means "use
     * defaults".
     */
    bytea      *rd_options;     /* parsed pg_class.reloptions */

    /*
     * Oid of the handler for this relation. For an index this is a function
     * returning IndexAmRoutine, for table like relations a function returning
     * TableAmRoutine.  This is stored separately from rd_indam, rd_tableam as
     * its lookup requires syscache access, but during relcache bootstrap we
     * need to be able to initialize rd_tableam without syscache lookups.
     */
    Oid         rd_amhandler;   /* OID of index AM's handler function */

    /*
     * Table access method.
     */
    const struct TableAmRoutine *rd_tableam;

    /* These are non-NULL only for an index relation: */
    Form_pg_index rd_index;     /* pg_index tuple describing this index */
    /* use "struct" here to avoid needing to include htup.h: */
    struct HeapTupleData *rd_indextuple;    /* all of pg_index tuple */

    /*
     * index access support info (used only for an index relation)
     *
     * Note: only default support procs for each opclass are cached, namely
     * those with lefttype and righttype equal to the opclass's opcintype. The
     * arrays are indexed by support function number, which is a sufficient
     * identifier given that restriction.
     */
    MemoryContext rd_indexcxt;  /* private memory cxt for this stuff */
    /* use "struct" here to avoid needing to include amapi.h: */
    struct IndexAmRoutine *rd_indam;    /* index AM's API struct */
    Oid        *rd_opfamily;    /* OIDs of op families for each index col */
    Oid        *rd_opcintype;   /* OIDs of opclass declared input data types */
    RegProcedure *rd_support;   /* OIDs of support procedures */
    struct FmgrInfo *rd_supportinfo;    /* lookup info for support procedures */
    int16      *rd_indoption;   /* per-column AM-specific flags */
    List       *rd_indexprs;    /* index expression trees, if any */
    List       *rd_indpred;     /* index predicate tree, if any */
    Oid        *rd_exclops;     /* OIDs of exclusion operators, if any */
    Oid        *rd_exclprocs;   /* OIDs of exclusion ops' procs, if any */
    uint16     *rd_exclstrats;  /* exclusion ops' strategy numbers, if any */
    Oid        *rd_indcollation;    /* OIDs of index collations */
    bytea     **rd_opcoptions;  /* parsed opclass-specific options */


    /*
     * rd_amcache is available for index and table AMs to cache private data
     * about the relation.  This must be just a cache since it may get reset
     * at any time (in particular, it will get reset by a relcache inval
     * message for the relation).  If used, it must point to a single memory
     * chunk palloc'd in CacheMemoryContext, or in rd_indexcxt for an index
     * relation.  A relcache reset will include freeing that chunk and setting
     * rd_amcache = NULL.
     */
    void       *rd_amcache;     /* available for use by index/table AM */

    /*
     * foreign-table support
     *
     * rd_fdwroutine must point to a single memory chunk palloc'd in
     * CacheMemoryContext.  It will be freed and reset to NULL on a relcache
     * reset.
     */

    /* use "struct" here to avoid needing to include fdwapi.h: */
    struct FdwRoutine *rd_fdwroutine;   /* cached function pointers, or NULL */

    /*
     * Hack for CLUSTER, rewriting ALTER TABLE, etc: when writing a new
     * version of a table, we need to make any toast pointers inserted into it
     * have the existing toast table's OID, not the OID of the transient toast
     * table.  If rd_toastoid isn't InvalidOid, it is the OID to place in
     * toast pointers inserted into this rel.  (Note it's set on the new
     * version of the main heap, not the toast table itself.)  This also
     * causes toast_save_datum() to try to preserve toast value OIDs.
     */
    Oid         rd_toastoid;    /* Real TOAST table's OID, or InvalidOid */

    bool        pgstat_enabled; /* should relation stats be counted */
    /* use "struct" here to avoid needing to include pgstat.h: */
    struct PgStat_TableStatus *pgstat_info; /* statistics collection area */
} RelationData;
```

## RelOptInfo

Note in `nodes/pathnodes.h`. Pay particular attention to the final extract.

    RelOptInfo
         Per-relation information for planning/optimization

    For planning purposes, a "base rel" is either a plain relation (a table)
    or the output of a sub-SELECT or function that appears in the range table.
    In either case it is uniquely identified by an RT index.  A "joinrel"
    is the joining of two or more base rels.  A joinrel is identified by
    the set of RT indexes for its component baserels, along with RT indexes
    for any outer joins it has computed.  We create RelOptInfo nodes for each
    baserel and joinrel, and store them in the PlannerInfo's simple_rel_array
    and join_rel_list respectively.

    ...

    Parts of this data structure are specific to various scan and join
    mechanisms.  It didn't seem worth creating new node types for them.
   
         relids - Set of relation identifiers (RT indexes).  This is a base
                  relation if there is just one, a join relation if more;
                  in the join case, RT indexes of any outer joins formed
                  at or below this join are included along with baserels
         rows - estimated number of tuples in the relation after restriction
                clauses have been applied (ie, output rows of a plan for it)
         consider_startup - true if there is any value in keeping plain paths for
                            this rel on the basis of having cheap startup cost
         consider_param_startup - the same for parameterized paths
         reltarget - Default Path output tlist for this rel; normally contains
                     Var and PlaceHolderVar nodes for the values we need to
                     output from this relation.
                     List is in no particular order, but all rels of an
                     appendrel set must use corresponding orders.
                     NOTE: in an appendrel child relation, may contain
                     arbitrary expressions pulled up from a subquery!
         pathlist - List of Path nodes, one for each potentially useful
                    method of generating the relation
         ppilist - ParamPathInfo nodes for parameterized Paths, if any
         cheapest_startup_path - the pathlist member with lowest startup cost
             (regardless of ordering) among the unparameterized paths;
             or NULL if there is no unparameterized path
         cheapest_total_path - the pathlist member with lowest total cost
             (regardless of ordering) among the unparameterized paths;
             or if there is no unparameterized path, the path with lowest
             total cost among the paths with minimum parameterization
         cheapest_unique_path - for caching cheapest path to produce unique
             (no duplicates) output from relation; NULL if not yet requested
         cheapest_parameterized_paths - best paths for their parameterizations;
             always includes cheapest_total_path, even if that's unparameterized
         direct_lateral_relids - rels this rel has direct LATERAL references to
         lateral_relids - required outer rels for LATERAL, as a Relids set
             (includes both direct and indirect lateral references)
   
    If the relation is a base relation it will have these fields set:
   
         relid - RTE index (this is redundant with the relids field, but
                 is provided for convenience of access)
         rtekind - copy of RTE's rtekind field
         min_attr, max_attr - range of valid AttrNumbers for rel
         attr_needed - array of bitmapsets indicating the highest joinrel
                 in which each attribute is needed; if bit 0 is set then
                 the attribute is needed as part of final targetlist
         attr_widths - cache space for per-attribute width estimates;
                       zero means not computed yet
         nulling_relids - relids of outer joins that can null this rel
         lateral_vars - lateral cross-references of rel, if any (list of
                        Vars and PlaceHolderVars)
         lateral_referencers - relids of rels that reference this one laterally
                 (includes both direct and indirect lateral references)
         indexlist - list of IndexOptInfo nodes for relation's indexes
                     (always NIL if it's not a table or partitioned table)
         pages - number of disk pages in relation (zero if not a table)
         tuples - number of tuples in relation (not considering restrictions)
         allvisfrac - fraction of disk pages that are marked all-visible
         eclass_indexes - EquivalenceClasses that mention this rel (filled
                          only after EC merging is complete)
         subroot - PlannerInfo for subquery (NULL if it's not a subquery)
         subplan_params - list of PlannerParamItems to be passed to subquery
   
         Note: for a subquery, tuples and subroot are not set immediately
         upon creation of the RelOptInfo object; they are filled in when
         set_subquery_pathlist processes the object.
   
         For otherrels that are appendrel members, these fields are filled
         in just as for a baserel, except we don't bother with lateral_vars.

    ...

    If the relation is either a foreign table or a join of foreign tables that
    all belong to the same foreign server and are assigned to the same user to
    check access permissions as (cf checkAsUser), these fields will be set:
   
         serverid - OID of foreign server, if foreign table (else InvalidOid)
         userid - OID of user to check access as (InvalidOid means current user)
         useridiscurrent - we've assumed that userid equals current user
         fdwroutine - function hooks for FDW, if foreign table (else NULL)
         fdw_private - private state for FDW, if foreign table (else NULL)
   
```C
typedef struct RelOptInfo
{
    pg_node_attr(no_copy_equal, no_read, no_query_jumble)

    NodeTag     type;

    RelOptKind  reloptkind;

    /*
     * all relations included in this RelOptInfo; set of base + OJ relids
     * (rangetable indexes)
     */
    Relids      relids;

    /*
     * size estimates generated by planner
     */
    /* estimated number of result tuples */
    Cardinality rows;

    /*
     * per-relation planner control flags
     */
    /* keep cheap-startup-cost paths? */
    bool        consider_startup;
    /* ditto, for parameterized paths? */
    bool        consider_param_startup;
    /* consider parallel paths? */
    bool        consider_parallel;

    /*
     * default result targetlist for Paths scanning this relation; list of
     * Vars/Exprs, cost, width
     */
    struct PathTarget *reltarget;

    /*
     * materialization information
     */
    List       *pathlist;       /* Path structures */
    List       *ppilist;        /* ParamPathInfos used in pathlist */
    List       *partial_pathlist;   /* partial Paths */
    struct Path *cheapest_startup_path;
    struct Path *cheapest_total_path;
    struct Path *cheapest_unique_path;
    List       *cheapest_parameterized_paths;

    /*
     * parameterization information needed for both base rels and join rels
     * (see also lateral_vars and lateral_referencers)
     */
    /* rels directly laterally referenced */
    Relids      direct_lateral_relids;
    /* minimum parameterization of rel */
    Relids      lateral_relids;

    /*
     * information about a base rel (not set for join rels!)
     */
    Index       relid;
    /* containing tablespace */
    Oid         reltablespace;
    /* RELATION, SUBQUERY, FUNCTION, etc */
    RTEKind     rtekind;
    /* smallest attrno of rel (often <0) */
    AttrNumber  min_attr;
    /* largest attrno of rel */
    AttrNumber  max_attr;
    /* array indexed [min_attr .. max_attr] */
    Relids     *attr_needed pg_node_attr(read_write_ignore);
    /* array indexed [min_attr .. max_attr] */
    int32      *attr_widths pg_node_attr(read_write_ignore);
    /*
     * Zero-based set containing attnums of NOT NULL columns.  Not populated
     * for rels corresponding to non-partitioned inh==true RTEs.
     */
    Bitmapset  *notnullattnums;
    /* relids of outer joins that can null this baserel */
    Relids      nulling_relids;
    /* LATERAL Vars and PHVs referenced by rel */
    List       *lateral_vars;
    /* rels that reference this baserel laterally */
    Relids      lateral_referencers;
    /* list of IndexOptInfo */
    List       *indexlist;
    /* list of StatisticExtInfo */
    List       *statlist;
    /* size estimates derived from pg_class */
    BlockNumber pages;
    Cardinality tuples;
    double      allvisfrac;
    /* indexes in PlannerInfo's eq_classes list of ECs that mention this rel */
    Bitmapset  *eclass_indexes;
    PlannerInfo *subroot;       /* if subquery */
    List       *subplan_params; /* if subquery */
    /* wanted number of parallel workers */
    int         rel_parallel_workers;
    /* Bitmask of optional features supported by the table AM */
    uint32      amflags;

    /*
     * Information about foreign tables and foreign joins
     */
    /* identifies server for the table or join */
    Oid         serverid;
    /* identifies user to check access as; 0 means to check as current user */
    Oid         userid;
    /* join is only valid for current user */
    bool        useridiscurrent;
    /* use "struct FdwRoutine" to avoid including fdwapi.h here */
    struct FdwRoutine *fdwroutine pg_node_attr(read_write_ignore);
    void       *fdw_private pg_node_attr(read_write_ignore);

    /*
     * cache space for remembering if we have proven this relation unique
     */
    /* known unique for these other relid set(s) */
    List       *unique_for_rels;
    /* known not unique for these set(s) */
    List       *non_unique_for_rels;

    /*
     * used by various scans and joins:
     */
    /* RestrictInfo structures (if base rel) */
    List       *baserestrictinfo;
    /* cost of evaluating the above */
    QualCost    baserestrictcost;
    /* min security_level found in baserestrictinfo */
    Index       baserestrict_min_security;
    /* RestrictInfo structures for join clauses involving this rel */
    List       *joininfo;
    /* T means joininfo is incomplete */
    bool        has_eclass_joins;

    /*
     * used by partitionwise joins:
     */
    /* consider partitionwise join paths? (if partitioned rel) */
    bool        consider_partitionwise_join;

    /*
     * inheritance links, if this is an otherrel (otherwise NULL):
     */
    /* Immediate parent relation (dumping it would be too verbose) */
    struct RelOptInfo *parent pg_node_attr(read_write_ignore);
    /* Topmost parent relation (dumping it would be too verbose) */
    struct RelOptInfo *top_parent pg_node_attr(read_write_ignore);
    /* Relids of topmost parent (redundant, but handy) */
    Relids      top_parent_relids;

    /*
     * used for partitioned relations:
     */
    /* Partitioning scheme */
    PartitionScheme part_scheme pg_node_attr(read_write_ignore);

    /*
     * Number of partitions; -1 if not yet set; in case of a join relation 0
     * means it's considered unpartitioned
     */
    int         nparts;
    /* Partition bounds */
    struct PartitionBoundInfoData *boundinfo pg_node_attr(read_write_ignore);
    /* True if partition bounds were created by partition_bounds_merge() */
    bool        partbounds_merged;
    /* Partition constraint, if not the root */
    List       *partition_qual;

    /*
     * Array of RelOptInfos of partitions, stored in the same order as bounds
     * (don't print, too bulky and duplicative)
     */
    struct RelOptInfo **part_rels pg_node_attr(read_write_ignore);

    /*
     * Bitmap with members acting as indexes into the part_rels[] array to
     * indicate which partitions survived partition pruning.
     */
    Bitmapset  *live_parts;
    /* Relids set of all partition relids */
    Relids      all_partrels;

    /*
     * These arrays are of length partkey->partnatts, which we don't have at
     * hand, so don't try to print
     */

    /* Non-nullable partition key expressions */
    List      **partexprs pg_node_attr(read_write_ignore);
    /* Nullable partition key expressions */
    List      **nullable_partexprs pg_node_attr(read_write_ignore);
} RelOptInfo;
```
