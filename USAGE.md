# USAGE

```sql
CREATE EXTENSION simple_fdw;
CREATE SERVER simple_server FOREIGN DATA WRAPPER simple_fdw;
CREATE USER MAPPING FOR postgres SERVER simple_server;

CREATE FOREIGN TABLE simple_table (
       id serial,
       b bool,
       bt bit,
       s int2,
       i int4,
       l int8,
       f float4,
       d float8,
       n numeric,
       str text,
       dt date,
       tm time,
       ts timestamp,
       tstz timestamptz,
       uuid uuid,
       xml xml,
       json json,
       jsonb jsonb,
       jsonp jsonpath
) SERVER simple_server;

SELECT * from simple_table; 
```


## Notes
```
ALTER USER MAPPING FOR some_non_superuser SERVER loopback_nopw
OPTIONS (ADD password_required 'false');
```

postgres_fdw_get_connections(OUT server_name text, OUT valid boolean) returns setof record
This function returns the foreign server names of all the open connections that postgres_fdw established from the local session to the foreign servers. It also returns whether each connection is valid or not. false is returned if the foreign server connection is used in the current local transaction but its foreign server or user mapping is changed or dropped (Note that server name of an invalid connection will be NULL if the server is dropped), and then such invalid connection will be closed at the end of that transaction. true is returned otherwise. If there are no open connections, no record is returned. Example usage of the function:

```
postgres=# SELECT * FROM postgres_fdw_get_connections() ORDER BY 1;
server_name | valid
-------------+-------
loopback1   | t
loopback2   | f
```
postgres_fdw_disconnect(server_name text) returns boolean
This function discards the open connections that are established by postgres_fdw from the local session to the foreign server with the given name. Note that there can be multiple connections to the given server using different user mappings. If the connections are used in the current local transaction, they are not disconnected and warning messages are reported. This function returns true if it disconnects at least one connection, otherwise false. If no foreign server with the given name is found, an error is reported. Example usage of the function:

```
postgres=# SELECT postgres_fdw_disconnect('loopbac``k1');
postgres_fdw_disconnect
-------------------------
t
```
postgres_fdw_disconnect_all() returns boolean
This function discards all the open connections that are established by postgres_fdw from the local session to foreign servers. If the connections are used in the current local transaction, they are not disconnected and warning messages are reported. This function returns true if it disconnects at least one connection, otherwise false. Example usage of the function:

```
postgres=# SELECT postgres_fdw_disconnect_all();
postgres_fdw_disconnect_all
-----------------------------
t
```

-----

postgres_fdw.application_name (string)
Specifies a value for application_name configuration parameter used when postgres_fdw establishes a connection to a foreign server. This overrides application_name option of the server object. Note that change of this parameter doesn't affect any existing connections until they are re-established.

postgres_fdw.application_name can be any string of any length and contain even non-ASCII characters. However when it's passed to and used as application_name in a foreign server, note that it will be truncated to less than NAMEDATALEN characters. Anything other than printable ASCII characters are replaced with C-style hexadecimal escapes. See application_name for details.

% characters begin “escape sequences” that are replaced with status information as outlined below. Unrecognized escapes are ignored. Other characters are copied straight to the application name. Note that it's not allowed to specify a plus/minus sign or a numeric literal after the % and before the option, for alignment and padding.

Escape	Effect
%a	Application name on local server
%c	Session ID on local server (see log_line_prefix for details)
%C	Cluster name on local server (see cluster_name for details)
%u	User name on local server
%d	Database name on local server
%p	Process ID of backend on local server
%%	Literal %
For example, suppose user local_user establishes a connection from database local_db to foreign_db as user foreign_user, the setting 'db=%d, user=%u' is replaced with 'db=local_db, user=local_user'.

F.36.10. Examples
Here is an example of creating a foreign table with postgres_fdw. First install the extension:

```
CREATE EXTENSION postgres_fdw;
-- Then create a foreign server using CREATE SERVER. In this example we wish to connect to a PostgreSQL server on host 192.83.123.89 listening on port 5432. The database to which the connection is made is named foreign_db on the remote server:

CREATE SERVER foreign_server
   FOREIGN DATA WRAPPER postgres_fdw
   OPTIONS (host '192.83.123.89', port '5432', dbname 'foreign_db');
-- A user mapping, defined with CREATE USER MAPPING, is needed as well to identify the role that will be used on the remote server:

CREATE USER MAPPING FOR local_user
   SERVER foreign_server
   OPTIONS (user 'foreign_user', password 'password');
-- Now it is possible to create a foreign table with CREATE FOREIGN TABLE. In this example we wish to access the table named some_schema.some_table on the remote server. The local name for it will be foreign_table:

CREATE FOREIGN TABLE foreign_table (
   id integer NOT NULL,
   data text
) SERVER foreign_server
   OPTIONS (schema_name 'some_schema', table_name 'some_table');
```

It's essential that the data types and other properties of the columns declared in CREATE FOREIGN TABLE match the actual remote table. Column names must match as well, unless you attach column_name options to the individual columns to show how they are named in the remote table. In many cases, use of IMPORT FOREIGN SCHEMA is preferable to constructing foreign table definitions manually.

F.36.11. Author 