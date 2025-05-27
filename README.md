# Simplified Implementation (SIMPLE) of PostgreSQL FDW

## Executive Summary

This is a skeleton implementation of a PostgresSQL FDW, with the distinction
of using abstraction layers to isolate the "business logic" from the
PostgreSQL specific code to the maximum extent possible).

At the moment the implementation is limited to "read-only" operations.
That's sufficient for many external services.

## The Need

The PostgreSQL Foreign Data Wrapper (FDW) has a deadly trap for the unwary -
it is far too easy to mix the "business logic" with the implementation of
the FDW API.

This has several drawbacks:

- it requires the developer and maintainer to have a relatively deep understanding
  of the PostgreSQL backend to reduce the risk of a seemingly innocent bit of code
  compromising the server.

- it complicates maintenance of the PostgreSQL FDW API code. There's very few
  changes to the existing API but a dev team may wish to implement additional
  functionality over time, e.g., enabling 'push down' queries for better performance,
  and the need to work around the existing code could be difficult. This is much
  easier to implement if the FDW API can identify the key elements (ideally into
  a single function call) and leave the implementation to the team responsible
  for maintaining the business logic.

- It ties the implementation to a single language - C. Strictly speaking it is
  possible to call the conversion functions (e.g., JNI) directly but this greatly
  complicates the FDW. This is especially true of the second language requires
  additional steps during initialization and termination.

## The Solution

The solution is to examine the minimal implementation of the `blackhole_fdw` to

- Determine the custom `state` used in each function. This is stored in the
  fdw_private pointer or `List`.

  Read-only examples:

    - `simple_fdw_plan_state` (for `FdwPlanScate`)
    - `simple_fdw_scan_state` (for `FdWScanState`)

  Both will need to have their own `void *fdw_private` field.

- Determine the action performed in each function and what new information is
  available, if any.
 
- Determine the data structures required.

  **Plan state**
  ```C
  typedef struct {
      void (*open(void);
      void (*close)(void);
  
      Cardinality rows;
 
      // I have no additional information yet.
  
      void *simple_plan_private;
  } simple_fdw_plan_state;
  ```

  **(Table) scan**

  ```C
  typedef struct simple_fdw_relation_attribute;
  typedef struct simple_fdw_relation_row;
  typedef struct simple_fdw_relation_column;
  
  typedef struct {
    void (*open)(void);
    void (*next)(void);
    void (*reset)(void);  // rescan(void) ?
    void (*close)(void);
  
    simple_fdw_relation_attribute *table_attributes;
    simple_fdw_relation_row *current_row;
  
    void *simple_scan_private;
  } simple_fdw_scan_state;
  ```

  **Table Attributes**
  ```C
  typedef struct {
      const char *name;
      simple_fdw_column_attribute *column_attributes;
      int        column_count;
  } simple_fdw_relation_attribute;
  ```    

  **Column Attributes**
  ```C
  typedef struct {
      int        colno;
      const char *name;
      int        sqltype;
      bool       not_null;
      ...
  } simple_fdw_relation_attribute;
  ```

  **Row**
  ```C
  typedef struct {
      bool       is_valid;   // false if end of scan
      int        rowno;
      simple_fdw_column *columns;
      int        column_count;
  } simple_fdw_relation_row;
  ```
  
  **Column**
  ```C
  #define MAX_STRLEN 128
  typedef struct {
      bool       is_null;
      union {
         char        *s_ptr;  // must be null-padded
         size_t      strlen;
  
         c           char;
         s           short;
         i           int;
         l           long;
         f           float;
         d           double;
         str         char[MAX_STRLEN];
         t           time_t;
         ...
      }          value;
  } simple_fdw_column;
  ```

- Flesh out the existing skeleton code so it:

    - Collects the required information
    - Calls the appropriate SIMPLE API function
    - Populates the corresponding PostgreSQL objects

- Finally add another class that holds the constructors for each of the
  states, e.g., `simpleImpl->create_fdw_plan_state()`.
 
The last step allows us to have confident that have captured the entire
interaction between the PostgreSQL FDW API and our implementation. This
is a step beyond making nearly all of our functions `static`.

## Placeholder Implementation

The `Placeholder` is an implementation with a single purpose - to allow
us to actually verify our integration with the PostgreSQL FDW API is correct.

Fortunately we don't need much:

- a simple `struct`, e.g.,

    ```C
    #define MAX_STRING_LEN 20
    
    typedef struct {
        int id,
        char s[MAX_STRING_LEN],
        time_t t
    } SimpleTestType; 
    ```

- at least one way to populate an array of test values.

- a way to keep track of the current state of the scan (updating position
  within the row).

- an implementation of `next()` that copies the current row to the
  `simple_fdw_relation_row` object.

- for UPDATE operations we need to be able to copy *some* values from
  the `simple_fdw_relation_row` object into our cached values.

- for INSERT operations we need a way to create a new row and populate
  it with the values in the `simple_row_relation_row` object.

- for DELETE operations we need a way to remove an entire row from
  the internal cache.