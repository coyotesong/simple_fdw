
/*
 * "Glue" that links the internal PostgreSQL FDW structures to our
 * database-ignorant backend. This eliminates a lot of the noise
 * if we try to mix the `FdwRoutines` implementation with the
 * actual backend.
 */
#ifndef SIMPLE_FWD_GLUE_H
#define SIMPLE_FWD_GLUE_H

#include <postgres.h>
#include <executor/tuptable.h>
#include <nodes/execnodes.h>
#include <nodes/pathnodes.h>

/*
 * Describes the valid options for objects that use this wrapper.
 */
struct simpleFdwOption {
    const char *optname;
    Oid optcontext; /* Oid of catalog in which option may appear */
};

/*
 * The plan state is set up in GetForeignRelSize and stashed away in
 * rel->fdw_private and fetched in GetForeignPaths.
 *
 * The constructor must take accept the RelOptInfo and Oid in order
 * to determine the proper functions to associate with the struct.s
 */
struct simple_fdw_plan_state;
typedef struct simple_fdw_plan_state SimpleFdwPlanState;

struct simple_fdw_plan_state {
    void (*open)(SimpleFdwPlanState *plan_state);   // GetForeignRelSize
    void (*close)(SimpleFdwPlanState *plan_state);  // ?

    Oid         serverid; // or InvalidOid
    Oid         userid;   // InvalidOid = current user
    bool        useridiscurrent;
    Oid         tableid;

    List        *options;
    BlockNumber pages;
    Cardinality rows;

    void        *plan_private;
};

typedef struct {
    Index       colno;
    const char  *name;
    int         sqltype;
    bool        not_null;
    // ...
} SimpleFdwRelationAttributeColumn;

typedef struct {
    SimpleFdwRelationAttributeColumn *columns;
    Cardinality column_count;
} SimpleFdwRelationAttributeRow;

typedef struct {
    const char  *name;
    SimpleFdwRelationAttributeRow row; // not a pointer!

    // additional information provided by FDW API...
} SimpleFdwRelationAttribute;

#define MAX_COLUMN_STRLEN 128
typedef struct {
    bool       is_null;
    int        pgtypoid;
    union {
        char        *s_ptr;  // must be null-padded
        size_t      strlen;

        char        c;
        short       s;
        int         i;
        long        l;
        float       f;
        double      d;
        char        str[MAX_COLUMN_STRLEN];

        time_t      t;        // seconds
        useconds_t  useconds; // microseconds - timestamp supports microsecond resolution (or better?)
        // ...
     }          value;
} SimpleFdwRelationValueColumn;

typedef struct {
    bool        is_valid;   // false if end of scan
    Index       row_no;
    SimpleFdwRelationValueColumn *columns;
    Cardinality column_count;
} SimpleFdwRelationValueRow;

/*
 * The scan state is for maintaining state for a scan, either for a
 * SELECT or UPDATE or DELETE.
 *
 * It is set up in simpleBeginForeignScan and stashed in node->fdw_state
 * and subsequently used in simpleIterateForeignScan,
 * simpleEndForeignScan and simpleReScanForeignScan.
 *
 * Note: the 'next()` must always fully populate the `TupleTableSlot`.
 *
 * Note: the `slot` is normally extracted from the `ForeignScanState`
 * object but using an explicit parameter gives us more options.
 */
struct simple_fdw_scan_state;
typedef struct simple_fdw_scan_state SimpleFdwScanState;

struct simple_fdw_scan_state {
    void (*open)(SimpleFdwScanState *scan_state);    // BeginForeignScan
    void (*next)(SimpleFdwScanState *scan_state);    // BeginIterateForeignScan - copy data into 'current_row'
    void (*reset)(SimpleFdwScanState *scan_state);   // ReScanForeignScan
    void (*close)(SimpleFdwScanState *scan_state);   // EndForeignScan

    List  *options;
    bool  is_open;
    Index current_pos;

    // todo - don't use ptrs?
    SimpleFdwRelationAttribute *table_attributes;
    SimpleFdwRelationValueRow *current_row;

    void *scan_private;
};

/**
 * Container for simple FDW implementation.
 */
typedef struct {;
    SimpleFdwPlanState *(*create_simple_fdw_plan_state)(
        PlannerInfo *root,
        RelOptInfo *rel,
        Oid foreigntableoid);

    SimpleFdwScanState *(*create_simple_fdw_scan_state)(
        ForeignScanState *node,
        int eflags);
} SimpleFdwOps;

extern SimpleFdwPlanState *create_blackhole_fdw_plan_state (
    PlannerInfo *root,
    RelOptInfo *rel,
    Oid foreigntableoid);

extern SimpleFdwScanState *create_blackhole_fdw_scan_state (
    ForeignScanState *node,
    int eflags);

// extern const SimpleFdwOps *create_simple_fdw_blackhole_ops(void);

extern void copyToTuple(SimpleFdwRelationValueRow *row, TupleTableSlot *slot);
extern void copyFromTuple(TupleTableSlot *slot, SimpleFdwRelationValueRow *row);

extern void copyAttributeFromPg(void *, SimpleFdwRelationAttributeRow *attrs);

// copyAttributeToPg() will be used with ImportForeignSchemaStmt...

#endif //SIMPLE_FWD_GLUE_H
