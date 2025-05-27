/*-------------------------------------------------------------------------
 *
 *                foreign-data wrapper  simple
 *
 * Copyright (c) 2025, Bear Giles <bgiles@coyotesong.com>
 *
 * This software is released under the PostgreSQL Licence
 *
 * Author:  Bear Giles <bgiles@coyotesong.com>
 *
 * IDENTIFICATION
 *                simple_fdw/=sql/simple_fdw.sql
 *
 *-------------------------------------------------------------------------
 *
 *                foreign-data wrapper  simple
 *
 * Copyright (c) 2013, PostgreSQL Global Development Group
 *
 * This software is released under the PostgreSQL Licence
 *
 * Author:  Andrew Dunstan <andrew@dunslane.net>
 *
 * IDENTIFICATION
 *                simple_fdw/=sql/simple_fdw.sql
 *
 *-------------------------------------------------------------------------
 */

CREATE FUNCTION simple_fdw_handler()
    RETURNS fdw_handler
AS '$libdir/simple_fdw'
LANGUAGE C STRICT;

CREATE FUNCTION simple_fdw_validator(text[], oid)
    RETURNS void
AS '$libdir/simple_fdw'
LANGUAGE C STRICT;

CREATE
FOREIGN DATA WRAPPER simple_fdw
  HANDLER simple_fdw_handler
  VALIDATOR simple_fdw_validator;

-- CREATE EXTENSION simple_fdw;
