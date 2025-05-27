//
// Created by bgiles on 5/23/25.
//

#include <postgresql/17/server/c.h>
#include <postgresql/17/server/postgres.h>
#include <postgresql/17/server/postgres_ext.h>
#include <postgresql/17/server/access/tupdesc.h>
#include <postgresql/17/server/catalog/pg_attribute.h>
#include <postgresql/17/server/catalog/pg_type_d.h>
#include <postgresql/17/server/executor/tuptable.h>
#include <postgresql/17/server/nodes/execnodes.h>
#include <postgresql/17/server/nodes/pathnodes.h>
#include <postgresql/17/server/utils/elog.h>
#include <postgresql/17/server/utils/palloc.h>
#include <postgresql/17/server/utils/relcache.h>

#include "simplefdw_api.h"

/*
 * Possibly useful functions (from foreign/foreign.c)
 *
 *
 * extern FdwRoutine *GetFdwRoutine(Oid fdwhandler);
 * extern Oid  GetForeignServerIdByRelId(Oid relid);
 * extern FdwRoutine *GetFdwRoutineByServerId(Oid serverid);
 * extern FdwRoutine *GetFdwRoutineByRelId(Oid relid);
 * extern FdwRoutine *GetFdwRoutineForRelation(Relation relation, bool makecopy);
 * extern bool IsImportableForeignTable(const char *tablename,
 *                                     ImportForeignSchemaStmt *stmt);
 * extern Path *GetExistingLocalJoinPath(RelOptInfo *joinrel);
 */

extern SimpleFdwPlanState *create_blackhole_fdw_plan_state(PlannerInfo *root,
                           RelOptInfo *baserel,
                           Oid foreigntableid);

extern SimpleFdwScanState *create_blackhole_fdw_scan_state(ForeignScanState *node,
                          int eflags);

/*
extern const SimpleFdwOps *create_simple_fdw_blackhole_ops(void) {
    SimpleFdwOps *ops = palloc0(sizeof(SimpleFdwOps));
    ops->create_simple_fdw_plan_state = create_blackhole_fdw_plan_state;
    ops->create_simple_fdw_scan_state = create_blackhole_fdw_scan_state;

    return ops;
}
*/

// ------------------------------------------------------------
// Implementations...
// ------------------------------------------------------------

static void blackholeOpenPlan(struct simple_fdw_plan_state *);
static void blackholeClosePlan(struct simple_fdw_plan_state *);

static void blackholeOpenScan(SimpleFdwScanState *);
static void blackholeNextScan(SimpleFdwScanState *);
static void blackholeResetScan(SimpleFdwScanState *);
static void blackholeCloseScan(SimpleFdwScanState *);

/*
 * Create a new PlanState
 */
extern SimpleFdwPlanState *create_blackhole_fdw_plan_state(PlannerInfo *root,
                           RelOptInfo *baserel,
                           Oid foreigntableid) {

    SimpleFdwPlanState *plan_state = palloc0(sizeof(SimpleFdwPlanState));

    elog(NOTICE, "entering function %s", __func__);

    plan_state->open = blackholeOpenPlan;
    plan_state->close = blackholeClosePlan;

    /*
    reltarget - Default Path output tlist for this rel; normally contains
              Var and PlaceHolderVar nodes for the values we need to
              output from this relation.
              List is in no particular order, but all rels of an
              appendrel set must use corresponding orders.
              NOTE: in an appendrel child relation, may contain
              arbitrary expressions pulled up from a subquery!
    pathlist - List of Path nodes, one for each potentially useful
               method of generating the relation
*/
    plan_state->serverid = baserel->serverid; // or InvalidOid
    plan_state->userid = baserel->userid; // InvalidOid = current user
    plan_state->useridiscurrent = baserel->useridiscurrent;
    plan_state->tableid = foreigntableid;
    plan_state->rows = 0;

    plan_state->plan_private  = NULL;

    elog(NOTICE, "serverid: %d", plan_state->serverid);  // 16388
    elog(NOTICE, "userid  : %d", plan_state->userid);    // 0
    elog(NOTICE, "tableid : %d", plan_state->tableid);   // 16394

    // root->distinct_pathkeys.

    return plan_state;
}

static const char *toTypeName(int atttypid) {
    switch (atttypid) {
        case BOOLOID: return "bool";
        case BYTEAOID: return "bytea";
        case CHAROID: return "char";
        case NAMEOID: return "name";
        case INT8OID: return "int8";
        case INT4OID: return "int4";
        case FLOAT8OID: return "float8";
        case FLOAT4OID: return "float4";
        case TEXTOID: return "text";
        case OIDOID: return "oid";
        case JSONOID: return "json";
        case XMLOID: return "xml";
        case DATEOID: return "date";
        case TIMEOID: return "time";
        case TIMESTAMPOID: return "timestamp";
        case TIMESTAMPTZOID: return "timestamptz";
        case BITOID: return "bit";
        case NUMERICOID: return "numeric";
        case UUIDOID: return "uuid";
        case JSONBOID: return "jsonb";
        case JSONPATHOID: return "jsonpath";
    }

    return "unknown";

// BPCHAROID, VARCHAROID
    }

/**
 * Create a new ScanState
 */
extern SimpleFdwScanState *create_blackhole_fdw_scan_state(ForeignScanState *node,
                          int eflags) {
    SimpleFdwScanState *scan_state = palloc0(sizeof(SimpleFdwScanState));

    // elog(NOTICE, "tableid: %d", node->ss.ps.
    // Relation relation = node->ss.ss_currentRelation;
    // struct TableScanDescData *desc = node->ss.ss_currentScanDesc;
    TupleTableSlot *slot = node->ss.ss_ScanTupleSlot;

    // relation->
    // desc->
    TupleDesc tdesc = slot->tts_tupleDescriptor;

    elog(NOTICE, "entering function %s", __func__);

    scan_state->open = blackholeOpenScan;
    scan_state->next = blackholeNextScan;
    scan_state->reset = blackholeResetScan;
    scan_state->close = blackholeCloseScan;

    scan_state->scan_private = NULL;

    // catalog/pg_type_d.h

    // BOOLOID, BYTEA, CHAR, NAME, INT8, INT2, REGPROC, TEXT, JSON, XML, FLOAT4, FLOAT8, UNKNOWN, DATE, TIME, TIMESTAMP, TIMESTAMPTZ, BIT. NUMERIC
    // UUID, TSVECTOR, GTSVECTOR, TSQUERY, JSONB, JSONPATH, plus may more...


    elog(NOTICE, "table %d {", slot->tts_tableOid);
    for (int i = 0; i < tdesc->natts; i++) {
        FormData_pg_attribute attr = tdesc->attrs[i];
        elog(NOTICE, "  %-10.10s %-10.10s, -- %c  %d", attr.attname.data, toTypeName(attr.atttypid), attr.attstorage, attr.attnum);
    }
    elog(NOTICE, "}");

    /*
    ScanState   ss;             -- its first field is NodeTag --
    ExprState  *fdw_recheck_quals;  -- original quals not in ss.ps.qual --
    Size        pscan_len;      -- size of parallel coordination information --
    ResultRelInfo *resultRelInfo;   -- result rel info, if UPDATE or DELETE --

    typedef struct ScanState
    {
        PlanState   ps;             -- its first field is NodeTag --
        Relation    ss_currentRelation;
        struct TableScanDescData *ss_currentScanDesc;
        TupleTableSlot *ss_ScanTupleSlot;
    } ScanState;
    */

    return scan_state;
}

// ------------------------------------------------------------
// Implementations
// ------------------------------------------------------------
static void blackholeOpenPlan(SimpleFdwPlanState *plan_state) {
    elog(NOTICE, "entering function   %s", __func__);
}

static void blackholeClosePlan(SimpleFdwPlanState *plan_state) {
    elog(NOTICE, "entering function   %s", __func__);
}

static void blackholeOpenScan(SimpleFdwScanState *scan_state) {
    elog(NOTICE, "entering function   %s", __func__);

    scan_state->current_pos = 0;
    if (scan_state->is_open) {
        // TODO - report problem
    } else {
        scan_state->is_open = true;
    }
    // open, do not read first record
}

static void blackholeNextScan(SimpleFdwScanState *scan_state) {
    elog(NOTICE, "entering function   %s", __func__);

    if (!scan_state->is_open) {
        // TODO - report problem
        scan_state->current_pos = 0;
        scan_state->current_row->is_valid = false;
        return;
    }

//  read(currentPos);
    scan_state->current_pos++;
}

static void blackholeResetScan(SimpleFdwScanState *scan_state) {
    elog(NOTICE, "entering function   %s", __func__);

    scan_state->current_pos = 0;
    scan_state->current_row->is_valid = false;
    if (!scan_state->is_open) {
        // TODO - report problem
    }
}

static void blackholeCloseScan(SimpleFdwScanState *scan_state) {
    elog(NOTICE, "entering function   %s", __func__);

    scan_state->current_pos = 0;
    if (!scan_state->is_open) {
        // TODO - report problem
        scan_state->is_open = false;
    }
    scan_state->is_open = false;
    // close();
}