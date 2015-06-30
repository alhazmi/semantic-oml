/*
 * Copyright 2010 TU Berlin, Germany (Ruben Merz)
 * Copyright 2010-2014 National ICT Australia Limited (NICTA)
 *
 * This software may be used and distributed solely under the terms of
 * the MIT license (License).  You should find a copy of the License in
 * COPYING or at http://opensource.org/licenses/MIT. By downloading or
 * using this software you accept the terms and the liability disclaimer
 * in the License.
 */
/** \file psql_adapter.c
 * \brief Adapter code for the PostgreSQL database backend.
 */

#define _GNU_SOURCE  /* For NAN */
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <libpq-fe.h>
#include <time.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include "ocomm/o_log.h"
#include "mem.h"
#include "mstring.h"
#include "guid.h"
#include "json.h"
#include "oml_value.h"
#include "oml_util.h"
#include "database.h"
#include "database_adapter.h"
#include "psql_adapter.h"

static char backend_name[] = "psql";
/* Cannot be static due to the way the server sets its parameters */
char *pg_host = DEFAULT_PG_HOST;
char *pg_port = DEFAULT_PG_PORT;
char *pg_user = DEFAULT_PG_USER;
char *pg_pass = DEFAULT_PG_PASS;
char *pg_conninfo = DEFAULT_PG_CONNINFO;

/** Mapping between OML and PostgreSQL data types
 * \see psql_type_to_oml, psql_oml_to_type
 */
static db_typemap psql_type_pair[] = {
  { OML_DB_PRIMARY_KEY, "SERIAL PRIMARY KEY"}, /* We might need BIGSERIAL at some point. */
  { OML_LONG_VALUE,     "INT4" },
  { OML_DOUBLE_VALUE,   "FLOAT8" },
  { OML_STRING_VALUE,   "TEXT" },
  { OML_BLOB_VALUE,     "BYTEA" },
  { OML_INT32_VALUE,    "INT4" },
  { OML_UINT32_VALUE,   "INT8" }, /* PG doesn't support unsigned types --> promote */
  { OML_INT64_VALUE,    "INT8" },
  { OML_UINT64_VALUE,   "BIGINT" },
  { OML_GUID_VALUE,     "BIGINT" },
  { OML_BOOL_VALUE,     "BOOLEAN" },
  /* Vector types */
  { OML_VECTOR_DOUBLE_VALUE, "TEXT" },
  { OML_VECTOR_INT32_VALUE,  "TEXT" },
  { OML_VECTOR_UINT32_VALUE, "TEXT" },
  { OML_VECTOR_INT64_VALUE,  "TEXT" },
  { OML_VECTOR_UINT64_VALUE, "TEXT" },
  { OML_VECTOR_BOOL_VALUE,   "TEXT" },
};

static int sql_stmt(PsqlDB* self, const char* stmt);

/* Functions needed by the Database struct */
static OmlValueT psql_type_to_oml (const char *s);
static const char* psql_oml_to_type (OmlValueT type);
static int psql_stmt(Database* db, const char* stmt);
static void psql_release(Database* db);
static int psql_table_create (Database* db, DbTable* table, int shallow);
static int psql_table_free (Database *database, DbTable* table);
static char *psql_prepared_var(Database *db, unsigned int order);
static int psql_insert(Database *db, DbTable *table, int sender_id, int seq_no, double time_stamp, OmlValue *values, int value_count);
static char* psql_get_key_value (Database* database, const char* table, const char* key_column, const char* value_column, const char* key);
static int psql_set_key_value (Database* database, const char* table, const char* key_column, const char* value_column, const char* key, const char* value);
static char* psql_get_metadata (Database* database, const char* key);
static int psql_set_metadata (Database* database, const char* key, const char* value);
static int psql_add_sender_id(Database* database, const char* sender_id);
static char* psql_get_uri(Database *db, char *uri, size_t size);
static TableDescr* psql_get_table_list (Database *database, int *num_tables);

static MString* psql_prepare_conninfo(const char *database, const char *host, const char *port, const char *user, const char *pass, const char *extra_conninfo);
static char* psql_get_sender_id (Database* database, const char* name);
static int psql_set_sender_id (Database* database, const char* name, int id);
static void psql_receive_notice(void *arg, const PGresult *res);

MString* psql_prepare (Database *db, DbTable* table);

/** Prepare the conninfo string to connect to the Postgresql server.
 *
 * \param host server hostname
 * \param port server port or service name
 * \param user username
 * \param pass password
 * \param extra_conninfo additional connection parameters
 * \return a dynamically allocated MString containing the connection information; the caller must take care of freeing it
 *
 * \see mstring_delete, PQconnectdb
 */
static MString*
psql_prepare_conninfo(const char *database, const char *host, const char *port, const char *user, const char *pass, const char *extra_conninfo)
{
  MString *conninfo;
  int portnum;

  portnum = resolve_service(port, 5432);
  conninfo = mstring_create();
  mstring_sprintf (conninfo, "host='%s' port='%d' user='%s' password='%s' dbname='%s' %s",
      host, portnum, user, pass, database, extra_conninfo);

  return conninfo;
}

/** Setup the PostgreSQL backend.
 *
 * \return 0 on success, -1 otherwise
 *
 * \see database_setup_backend
 */
int
psql_backend_setup (void)
{
  MString *str, *conninfo;

  loginfo ("psql: Sending experiment data to PostgreSQL server %s:%s as user '%s'\n",
           pg_host, pg_port, pg_user);

  conninfo = psql_prepare_conninfo("postgres", pg_host, pg_port, pg_user, pg_pass, pg_conninfo);
  PGconn *conn = PQconnectdb (mstring_buf (conninfo));

  if (PQstatus (conn) != CONNECTION_OK) {
    logerror ("psql: Could not connect to PostgreSQL database (conninfo \"%s\"): %s", /* PQerrorMessage strings already have '\n'  */
         mstring_buf(conninfo), PQerrorMessage (conn));
    mstring_delete(conninfo);
    return -1;
  }

  /* oml2-server must be able to create new databases, so check that
     our user has the required role attributes */
  str = mstring_create();
  mstring_sprintf (str, "SELECT rolcreatedb FROM pg_roles WHERE rolname='%s'", pg_user);
  PGresult *res = PQexec (conn, mstring_buf (str));
  mstring_delete(str);
  if (PQresultStatus (res) != PGRES_TUPLES_OK) {
    logerror ("psql: Failed to determine role privileges for role '%s': %s", /* PQerrorMessage strings already have '\n'  */
         pg_user, PQerrorMessage (conn));
    return -1;
  }
  char *has_create = PQgetvalue (res, 0, 0);
  if (strcmp (has_create, "t") == 0)
    logdebug ("psql: User '%s' has CREATE DATABASE privileges\n", pg_user);
  else {
    logerror ("psql: User '%s' does not have required role CREATE DATABASE\n", pg_user);
    return -1;
  }

  mstring_delete(conninfo);
  PQclear (res);
  PQfinish (conn);

  return 0;
}

/** Mapping from PostgreSQL to OML types.
 * \see db_adapter_type_to_oml
 */
static
OmlValueT psql_type_to_oml (const char *type)
{
  int i = 0;
  int n = LENGTH(psql_type_pair);

  for (i = 0; i < n; i++) {
    if (strcmp (type, psql_type_pair[i].name) == 0) {
        return psql_type_pair[i].type;
    }
  }
  logwarn("Unknown PostgreSQL type '%s', using OML_UNKNOWN_VALUE\n", type);
  return OML_UNKNOWN_VALUE;
}

/** Mapping from OML types to PostgreSQL types.
 * \see db_adapter_oml_to_type
 */
static const char*
psql_oml_to_type (OmlValueT type)
{
  int i = 0;
  int n = LENGTH(psql_type_pair);

  for (i = 0; i < n; i++) {
    if (psql_type_pair[i].type == type) {
        return psql_type_pair[i].name;
    }
  }
  logerror("Unknown OML type %d\n", type);
  return NULL;
}

/** Execute an SQL statement (using PQexec()).
 * \see db_adapter_stmt, PQexec
 */
static int
sql_stmt(PsqlDB* self, const char* stmt)
{
  PGresult   *res;
  logdebug2("psql: Will execute '%s'\n", stmt);
  res = PQexec(self->conn, stmt);

  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    logerror("psql: Error executing '%s': %s", /* PQerrorMessage strings already have '\n'  */
        stmt, PQerrorMessage(self->conn));
    PQclear(res);
    return -1;
  }
  /*
   * Should PQclear PGresult whenever it is no longer needed to avoid memory
   * leaks
   */
  PQclear(res);

  return 0;
}

/** Type-agnostic wrapper for sql_stmt */
static int
psql_stmt(Database* db, const char* stmt)
{
 return sql_stmt((PsqlDB*)db->handle, stmt);
}

/** Create or open an PostgreSQL database
 * \see db_adapter_create
 */
int
psql_create_database(Database* db)
{
  MString *conninfo = NULL;
  MString *str = NULL;
  PGconn  *conn = NULL;
  PGresult *res = NULL;
  int ret = -1;

  loginfo ("psql:%s: Accessing database\n", db->name);

  /*
   * Make a connection to the database server -- check if the
   * requested database exists or not by connecting to the 'postgres'
   * database and querying that.
   */
  conninfo = psql_prepare_conninfo("postgres", pg_host, pg_port, pg_user, pg_pass, pg_conninfo);
  conn = PQconnectdb(mstring_buf (conninfo));

  /* Check to see that the backend connection was successfully made */
  if (PQstatus(conn) != CONNECTION_OK) {
    logerror ("psql: Could not connect to PostgreSQL database (conninfo \"%s\"): %s", /* PQerrorMessage strings already have '\n'  */
         mstring_buf(conninfo), PQerrorMessage (conn));
    goto cleanup_exit;
  }
  PQsetNoticeReceiver(conn, psql_receive_notice, "postgres");

  str = mstring_create();
  mstring_sprintf (str, "SELECT datname from pg_database where datname='%s';", db->name);
  res = PQexec (conn, mstring_buf (str));

  if (PQresultStatus (res) != PGRES_TUPLES_OK) {
    logerror ("psql: Could not get list of existing databases\n");
    goto cleanup_exit;
  }

  /* No result rows means database doesn't exist, so create it instead */
  if (PQntuples (res) == 0) {
    loginfo ("psql:%s: Database does not exist, creating it\n", db->name);
    mstring_set (str, "");
    mstring_sprintf (str, "CREATE DATABASE \"%s\";", db->name);

    res = PQexec (conn, mstring_buf (str));
    if (PQresultStatus (res) != PGRES_COMMAND_OK) {
      logerror ("psql:%s: Could not create database: %s", /* PQerrorMessage strings already have '\n'  */
          db->name, PQerrorMessage (conn));
      goto cleanup_exit;
    }
  }
  mstring_delete(str);
  str = NULL;

  PQfinish (conn);
  mstring_delete(conninfo);

  /* Now that the database should exist, make a connection to the it for real */
  conninfo = psql_prepare_conninfo(db->name, pg_host, pg_port, pg_user, pg_pass, pg_conninfo);
  conn = PQconnectdb(mstring_buf (conninfo));

  /* Check to see that the backend connection was successfully made */
  if (PQstatus(conn) != CONNECTION_OK) {
    logerror ("psql:%s: Could not connect to PostgreSQL database (conninfo \"%s\"): %s",
  /* PQerrorMessage strings already have '\n'  */
        db->name, mstring_buf(conninfo), PQerrorMessage (conn));
    goto cleanup_exit;
  }
  PQsetNoticeReceiver(conn, psql_receive_notice, db->name);
  mstring_delete(conninfo);
  conninfo = NULL;

  PsqlDB* self = (PsqlDB*)oml_malloc(sizeof(PsqlDB));
  self->conn = conn;
  self->last_commit = time (NULL);

  db->backend_name = backend_name;
  db->o2t = psql_oml_to_type;
  db->t2o = psql_type_to_oml;
  db->stmt = psql_stmt;
  db->create = psql_create_database;
  db->release = psql_release;
  db->prepared_var = psql_prepared_var;
  db->table_create = psql_table_create;
  db->table_create_meta = dba_table_create_meta;
  db->table_free = psql_table_free;
  db->prepare = psql_prepare;
  db->insert = psql_insert;
  db->add_sender_id = psql_add_sender_id;
  db->get_metadata = psql_get_metadata;
  db->set_metadata = psql_set_metadata;
  db->get_uri = psql_get_uri;
  db->get_table_list = psql_get_table_list;

  db->handle = self;

  dba_begin_transaction (db);

  /* Everything was successufl, prepare for cleanup */
  ret = 0;

cleanup_exit:
  if (res) { PQclear (res) ; }
  if (ret) { PQfinish (conn); } /* If return !=0, cleanup connection */

  if (str) { mstring_delete (str); }
  if (conninfo) { mstring_delete (conninfo); }
  /* All paths leading to here have conn uninitialised */
  return ret;
}

/** Release the psql database.
 * \see db_adapter_release
 */
static void
psql_release(Database* db)
{
  PsqlDB* self = (PsqlDB*)db->handle;
  dba_end_transaction (db);
  PQfinish(self->conn);
  oml_free(self);
  db->handle = NULL;
}

/** Create a PostgreSQL database and adapter structures
 * \see db_adapter_create
 */
/* This function is exposed to the rest of the code for backend initialisation */
int
psql_table_create (Database *db, DbTable *table, int shallow)
{
  MString *insert = NULL, *insert_name = NULL;
  PsqlDB* psqldb = NULL;
  PGresult *res = NULL;
  PsqlTable* psqltable = NULL;

  logdebug("psql:%s: Creating table '%s' (shallow=%d)\n", db->name, table->schema->name, shallow);

  if (db == NULL) {
    logerror("psql: Tried to create a table in a NULL database\n");
    return -1;
  }
  if (table == NULL) {
    logerror("psql:%s: Tried to create a table from a NULL definition\n", db->name);
    return -1;
  }
  if (table->schema == NULL) {
    logerror("psql:%s: No schema defined for table, cannot create\n", db->name);
    return -1;
  }
  psqldb = (PsqlDB*)db->handle;

  if (!shallow) {
    if (dba_table_create_from_schema(db, table->schema)) {
      logerror("psql:%s: Could not create table '%s': %s", /* PQerrorMessage strings already have '\n' */
          db->name, table->schema->name,
          PQerrorMessage (psqldb->conn));
      goto fail_exit;
    }
  }

  /* Related to #1056. */
  if (table->handle != NULL) {
    logwarn("psql:%s: BUG: Recreating PsqlTable handle for table %s\n",
        table->schema->name);
  }
  psqltable = (PsqlTable*)oml_malloc(sizeof(PsqlTable));
  table->handle = psqltable;

  /* Prepare the insert statement  */
  insert_name = mstring_create();
  mstring_set (insert_name, "OMLInsert-");
  mstring_cat (insert_name, table->schema->name);

  /* Prepare the statement in the database if it doesn't exist yet
   * XXX: We should really only create it if shallow==0; however some
   * tables can get created through dba_table_create_from_* which doesn't
   * initialise the prepared statement; there should be a
   * db_adapter_prepare_insert function provided by the backend, and callable
   * from dba_table_create_from_schema to do the following (in the case of
   * PostgreSQL). See #1056.
   */
  /* This next test might kill the transaction */
  dba_reopen_transaction(db);
  res = PQdescribePrepared(psqldb->conn, mstring_buf (insert_name));
  if(PQresultStatus(res) == PGRES_COMMAND_OK) {
    logdebug("psql:%s: Insertion statement %s already exists\n",
        db->name, mstring_buf (insert_name));
  } else {
    PQclear(res);
    /* This test killed the transaction; start a new one */
    dba_reopen_transaction(db);

    insert = database_make_sql_insert (db, table);
    if (!insert) {
      logerror("psql:%s: Failed to build SQL INSERT INTO statement for table '%s'\n",
          db->name, table->schema->name);
      goto fail_exit;
    }

    res = PQprepare(psqldb->conn,
        mstring_buf (insert_name),
        mstring_buf (insert),
        table->schema->nfields + 4, // FIXME:  magic number of metadata cols
        NULL);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
      logerror("psql:%s: Could not prepare statement: %s", /* PQerrorMessage strings already have '\n' */
          db->name, PQerrorMessage(psqldb->conn));
      PQclear(res);
      goto fail_exit;
    }
  }
  PQclear(res);

  psqltable->insert_stmt = insert_name;

  if (insert) { mstring_delete (insert); }
  return 0;

fail_exit:
  if (insert) { mstring_delete (insert); }
  if (insert_name) { mstring_delete (insert_name); }
  if (psqltable) { oml_free (psqltable); }
  return -1;
}

/** Free a PostgreSQL table
 *
 * Parameter database is ignored in this implementation
 *
 * \see db_adapter_table_free
 */
static int
psql_table_free (Database *database, DbTable *table)
{
  (void)database;
  PsqlTable *psqltable = (PsqlTable*)table->handle;
  if (psqltable) {
    mstring_delete (psqltable->insert_stmt);
    oml_free (psqltable);
  }
  return 0;
}

/** Return a string suitable for an unbound variable is PostgreSQL.
 * \see db_adapter_prepared_var
 */
static char*
psql_prepared_var(Database *db, unsigned int order)
{
  int nchar = 1 + (int) floor(log10(order+1))+1; /* Get the number of digits */
  char *s = oml_malloc(nchar + 1);

  (void)db;

  if (NULL != s) {
    snprintf(s, nchar + 1, "$%d", order);
  }

  return s;
}

/** Used as allocation size of strings to hold integers.
 * XXX: We play it safe as INT64_MAX is 9223372036854775807, that is 19 characters
 */
#define MAX_DIGITS 32

/** Prepare an INSERT statement for a given table
 *
 * The returned value is to be destroyed by the caller.
 *
 * \param db Database
 * \param table DbTable adapter for the target SQLite3 table
 * \return an MString containing the prepared statement, or NULL on error
 *
 * \see mstring_create, mstring_delete
 */
MString*
psql_prepare (Database *db, DbTable* table)
{
  MString* mstr = mstring_create ();
  int n = 0, i = 0;
  int max = table->schema->nfields;
  char *pvar;

  if (max <= 0) {
    logerror ("%d: Trying to insert 0 values into table %s\n",
        db->backend_name, table->schema->name);
    goto fail_exit;
  }

  if (mstr == NULL) {
    logerror("%d: Failed to create managed string for preparing SQL INSERT statement\n",
        db->backend_name);
    goto fail_exit;
  }
  
  /* Build SQL "INSERT INTO" statement */
    n += mstring_sprintf (mstr,
        "INSERT INTO \"%s\" (\"oml_sender_id\", \"oml_seq\", \"oml_ts_client\", \"oml_ts_server\"",
        table->schema->name);

    /* Add specific column names */
    for (i=0; i<max; i++) {
      n += mstring_sprintf (mstr, ", \"%s\"", table->schema->fields[i].name);
    }

    /* Close column names, and add variables for the prepared statement */
    pvar = db->prepared_var(db, 1);
    n += mstring_sprintf (mstr, ") VALUES (%s", pvar);
    oml_free(pvar); /* XXX: Not really efficient, but we only do this rarely */

    max += 4; /* Number of metadata columns we want to be able to insert */
    for (i=2; i<=max; i++) {
      pvar = db->prepared_var(db, i);
      n += mstring_sprintf (mstr, ", %s", pvar);
      oml_free(pvar);
    }

    /* We're done */
    n += mstring_cat (mstr, ");");

    logdebug2("%s:%s: Prepared insert statement for table %s: %s\n",
      db->backend_name, db->name, table->schema->name, mstring_buf(mstr));

  if (n != 0) {
    /* mstring_* return -1 on error, with no error, the sum in n should be 0 */
    goto fail_exit;
  }
  return mstr;

 fail_exit:
  if (mstr) mstring_delete (mstr);
  return NULL;
}

/** Insert value in the PostgreSQL database.
 * \see db_adapter_insert
 */
static int
psql_insert(Database* db, DbTable* table, int sender_id, int seq_no, double time_stamp, OmlValue* values, int value_count)
{
  PsqlDB* psqldb = (PsqlDB*)db->handle;
  PsqlTable* psqltable = (PsqlTable*)table->handle;
  PGresult* res;
  int i;
  double time_stamp_server;
  const char* insert_stmt = mstring_buf (psqltable->insert_stmt);
  unsigned char *escaped_blob;
  size_t len=MAX_DIGITS;

  char *paramValues[4+value_count];
  for (i=0;i<4+value_count;i++) {
    /* XXX: If some values are strings or blobs, we'll have to reallocate them */
    paramValues[i] = oml_malloc(MAX_DIGITS);
  }

  int paramLength[4+value_count];
  int paramFormat[4+value_count];

  snprintf(paramValues[0], MAX_DIGITS, "%i",sender_id);
  paramLength[0] = 0;
  paramFormat[0] = 0;

  snprintf(paramValues[1], MAX_DIGITS, "%i",seq_no);
  paramLength[1] = 0;
  paramFormat[1] = 0;

  snprintf(paramValues[2], MAX_DIGITS, "%.14e",time_stamp);
  paramLength[2] = 0;
  paramFormat[2] = 0;

  struct timeval tv;
  gettimeofday(&tv, NULL);
  time_stamp_server = tv.tv_sec - db->start_time + 0.000001 * tv.tv_usec;

  if (tv.tv_sec > psqldb->last_commit) {
    if (dba_reopen_transaction (db) == -1) {
      return -1;
    }
    psqldb->last_commit = tv.tv_sec;
  }

  snprintf(paramValues[3], MAX_DIGITS, "%.14e",time_stamp_server);
  paramLength[3] = 0;
  paramFormat[3] = 0;

  OmlValue* v = values;
  for (i = 0; i < value_count; i++, v++) {
    struct schema_field *field = &table->schema->fields[i];
    if (oml_value_get_type(v) != field->type) {
      logerror("psql:%s: Value %d type mismatch for table '%s'\n", db->name, i, table->schema->name);
      return -1;
    }
    switch (field->type) {
    case OML_LONG_VALUE:   snprintf(paramValues[4+i], MAX_DIGITS, "%i",(int)omlc_get_long(*oml_value_get_value(v))); break;
    case OML_INT32_VALUE:  snprintf(paramValues[4+i], MAX_DIGITS, "%" PRId32,omlc_get_int32(*oml_value_get_value(v))); break;
    case OML_UINT32_VALUE: snprintf(paramValues[4+i], MAX_DIGITS, "%" PRIu32,omlc_get_uint32(*oml_value_get_value(v))); break;
    case OML_INT64_VALUE:  snprintf(paramValues[4+i], MAX_DIGITS, "%" PRId64,omlc_get_int64(*oml_value_get_value(v))); break;
    case OML_UINT64_VALUE: snprintf(paramValues[4+i], MAX_DIGITS, "%" PRIu64,omlc_get_uint64(*oml_value_get_value(v))); break;
    case OML_DOUBLE_VALUE: snprintf(paramValues[4+i], MAX_DIGITS, "%.14e",omlc_get_double(*oml_value_get_value(v))); break;
    case OML_BOOL_VALUE:   snprintf(paramValues[4+i], MAX_DIGITS, "%d", omlc_get_bool(*oml_value_get_value(v)) ? 1 : 0); break;
    case OML_STRING_VALUE:
			   len=omlc_get_string_length(*oml_value_get_value(v)) + 1;
			   if (len > MAX_DIGITS) {
                             logdebug2("psql:%s: Reallocating %d bytes for long string\n", db->name, len);
			     paramValues[4+i] = oml_realloc(paramValues[4+i], len);
			     if (!paramValues[4+i]) {
			       logerror("psql:%s: Could not realloc()at memory for string '%s' in field %d of table '%s'\n",
				   db->name, omlc_get_string_ptr(*oml_value_get_value(v)), i, table->schema->name);
			       return -1;
			     }
			   }
			   snprintf(paramValues[4+i], len, "%s", omlc_get_string_ptr(*oml_value_get_value(v)));
			   break;
    case OML_BLOB_VALUE:
                           escaped_blob = PQescapeByteaConn(psqldb->conn,
					   omlc_get_blob_ptr(*oml_value_get_value(v)),
					   omlc_get_blob_length(*oml_value_get_value(v)),
					   &len);
                           if (!escaped_blob) {
                             logerror("psql:%s: Error escaping blob in field %d of table '%s': %s", /* PQerrorMessage strings already have '\n' */
                                 db->name, i, table->schema->name, PQerrorMessage(psqldb->conn));
                           }
			   if (len > MAX_DIGITS) {
                             logdebug2("psql:%s: Reallocating %d bytes for big blob\n", db->name, len);
                             paramValues[4+i] = oml_realloc(paramValues[4+i], len);
                             if (!paramValues[4+i]) {
                               logerror("psql:%s: Could not realloc()at memory for escaped blob in field %d of table '%s'\n",
                                   db->name, i, table->schema->name);
                               return -1;
                             }
                           }
                           snprintf(paramValues[4+i], len, "%s", escaped_blob);
                           PQfreemem(escaped_blob);
                           break;
    case OML_GUID_VALUE:
                           if(omlc_get_guid(*oml_value_get_value(v)) != OMLC_GUID_NULL) {
                             snprintf(paramValues[4+i], MAX_DIGITS, "%" PRId64, (int64_t)(omlc_get_guid(*oml_value_get_value(v))));
                           } else {
                             paramValues[4+i] = NULL;
                           }
                           break;

    case OML_VECTOR_DOUBLE_VALUE:
      vector_double_to_json(v->value.vectorValue.ptr, v->value.vectorValue.nof_elts, &paramValues[4+i]);
      break;
    case OML_VECTOR_INT32_VALUE:
      vector_int32_to_json(v->value.vectorValue.ptr, v->value.vectorValue.nof_elts, &paramValues[4+i]);
      break;
    case OML_VECTOR_UINT32_VALUE:
      vector_uint32_to_json(v->value.vectorValue.ptr, v->value.vectorValue.nof_elts, &paramValues[4+i]);
      break;
    case OML_VECTOR_INT64_VALUE:
      vector_int64_to_json(v->value.vectorValue.ptr, v->value.vectorValue.nof_elts, &paramValues[4+i]);
      break;
    case OML_VECTOR_UINT64_VALUE:
      vector_uint64_to_json(v->value.vectorValue.ptr, v->value.vectorValue.nof_elts, &paramValues[4+i]);
      break;
    case OML_VECTOR_BOOL_VALUE:
      vector_bool_to_json(v->value.vectorValue.ptr, v->value.vectorValue.nof_elts, &paramValues[4+i]);
      break;

    default:
      logerror("psql:%s: Unknown type %d in col '%s' of table '%s'; this is probably a bug\n",
          db->name, field->type, field->name, table->schema->name);
      return -1;
    }
    paramLength[4+i] = 0;
    paramFormat[4+i] = 0;
  }
  /* Use stuff from http://www.postgresql.org/docs/current/static/plpgsql-control-structures.html#PLPGSQL-ERROR-TRAPPING */

  res = PQexecPrepared(psqldb->conn, insert_stmt,
                       4+value_count, (const char**)paramValues,
                       (int*) &paramLength, (int*) &paramFormat, 0 );

  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    logerror("psql:%s: INSERT INTO '%s' failed: %s", /* PQerrorMessage strings already have '\n' */
        db->name, table->schema->name, PQerrorMessage(psqldb->conn));
    PQclear(res);
    return -1;
  }
  PQclear(res);

  for (i=0;i<4+value_count;i++) {
    oml_free(paramValues[i]);
  }

  return 0;
}

/** Do a key-value style select on a database table.
 *
 * FIXME: Not using prepared statements (#168)
 *
 * The caller is responsible for oml_free()ing the returned value when no longer
 * needed.
 *
 * This function does a key lookup on a database table that is set up
 * in key-value style.  The table can have more than two columns, but
 * this function SELECT's two of them and returns the value of the
 * value column.  It checks to make sure that the key returned is the
 * one requested, then returns its corresponding value.
 *
 * This function makes a lot of assumptions about the database and
 * the table:
 *
 * #- the database exists and is open
 * #- the table exists in the database
 * #- there is a column named key_column in the table
 * #- there is a column named value_column in the table
 *
 * The function does not check for any of these conditions, but just
 * assumes they are true.  Be advised.
 *
 * \param database Database to use
 * \param table name of the table to work in
 * \param key_column name of the column to use as key
 * \param value_column name of the column to set the value in
 * \param key key to look for in key_column
 *
 * \return an oml_malloc'd string value for the given key, or NULL
 *
 * \see psql_set_key_value, oml_malloc, oml_free
 */
static char*
psql_get_key_value (Database *database, const char *table, const char *key_column, const char *value_column, const char *key)
{
  if (database == NULL || table == NULL || key_column == NULL ||
      value_column == NULL || key == NULL)
    return NULL;

  PGresult *res;
  PsqlDB *psqldb = (PsqlDB*) database->handle;
  MString *stmt = mstring_create();
  mstring_sprintf (stmt, "SELECT %s FROM %s WHERE %s='%s';",
                   value_column, table, key_column, key);

  res = PQexec (psqldb->conn, mstring_buf (stmt));

  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
    logerror("psql:%s: Error trying to get %s[%s]; (%s)", /* PQerrorMessage strings already have '\n' */
             database->name, table, key, PQerrorMessage(psqldb->conn));
    goto fail_exit;
  }

  if (PQntuples (res) == 0)
    goto fail_exit;
  if (PQnfields (res) < 1)
    goto fail_exit;

  if (PQntuples (res) > 1)
    logwarn("psql:%s: Key-value lookup for key '%s' in %s(%s, %s) returned more than one possible key.\n",
             database->name, key, table, key_column, value_column);

  char *value = NULL;
  value = PQgetvalue (res, 0, 0);

  if (value != NULL)
    value = oml_strndup (value, strlen (value));

  PQclear (res);
  mstring_delete (stmt);
  return value;

 fail_exit:
  PQclear (res);
  mstring_delete (stmt);
  return NULL;
}

/** Set a value for the given key in the given table
 *
 * FIXME: Not using prepared statements (#168)
 *
 * \param database Database to use
 * \param table name of the table to work in
 * \param key_column name of the column to use as key
 * \param value_column name of the column to set the value in
 * \param key key to look for in key_column
 * \param value value to look for in value_column
 * \return 0 on success, -1 otherwise
 *
 * \see psql_get_key_value
 */
static int
psql_set_key_value (Database *database, const char *table, const char *key_column, const char *value_column, const char *key, const char *value)
{
  PsqlDB *psqldb = (PsqlDB*) database->handle;
  MString *stmt = mstring_create ();
  char *check_value = psql_get_key_value (database, table, key_column, value_column, key);
  if (check_value == NULL) {
    mstring_sprintf (stmt, "INSERT INTO \"%s\" (\"%s\", \"%s\") VALUES ('%s', '%s');",
                     table, key_column, value_column, key, value);
  } else {
    mstring_sprintf (stmt, "UPDATE \"%s\" SET \"%s\"='%s' WHERE \"%s\"='%s';",
                     table, value_column, value, key_column, key);
  }

  if (sql_stmt (psqldb, mstring_buf (stmt))) {
    logwarn("psql:%s: Key-value update failed for %s='%s' in %s(%s, %s) (database error)\n",
            database->name, key, value, table, key_column, value_column);
    return -1;
  }

  return 0;
}

/** Get data from the metadata table
 * \see db_adapter_get_metadata, sq3_get_key_value
 */
static char*
psql_get_metadata (Database *db, const char *key)
{
  return psql_get_key_value (db, "_experiment_metadata", "key", "value", key);
}

/** Set data in the metadata table
 * \see db_adapter_set_metadata, sq3_set_key_value
 */
static int
psql_set_metadata (Database *db, const char *key, const char *value)
{
  return psql_set_key_value (db, "_experiment_metadata", "key", "value", key, value);
}

/** Add a new sender to the database, returning its index.
 * \see db_add_sender_id
 */
static int
psql_add_sender_id(Database *db, const char *sender_id)
{
  PsqlDB *self = (PsqlDB*)db->handle;
  int index = -1;
  char *id_str = psql_get_sender_id (db, sender_id);

  if (id_str) {
    index = atoi (id_str);
    oml_free (id_str);

  } else {
    PGresult *res = PQexec (self->conn, "SELECT MAX(id) FROM _senders;");

    if (PQresultStatus (res) != PGRES_TUPLES_OK) {
      logwarn("psql:%s: Failed to get maximum sender id from database (restarting at 0): %s", /* PQerrorMessage strings already have '\n'  */
          db->name, PQerrorMessage (self->conn));
      PQclear (res);
      index = 0;
    } else {
      int rows = PQntuples (res);
      if (rows == 0) {
        logwarn("psql:%s: Failed to get maximum sender id from database: empty result; starting at 0\n",
            db->name);
        PQclear (res);
        index = 0;
      } else {
        index = atoi (PQgetvalue (res, 0, 0)) + 1;
        PQclear (res);
      }
    }

    psql_set_sender_id (db, sender_id, index);

  }

  return index;
}

/** Build a URI for this database.
 *
 * URI is of the form postgresql://USER@SERVER:PORT/DATABASE
 *
 * \see db_adapter_get_uri
 */
static char*
psql_get_uri(Database *db, char *uri, size_t size)
{
  if(snprintf(uri, size, "postgresql://%s@%s:%d/%s", pg_user, pg_host, resolve_service(pg_port, 5432), db->name) >= size) {
    return NULL;
  }
  return uri;
}

/** Get a list of tables in a PostgreSQL database
 * \see db_adapter_get_table_list
 */
static TableDescr*
psql_get_table_list (Database *database, int *num_tables)
{
  PsqlDB *self = database->handle;
  TableDescr *tables = NULL, *t = NULL;
  const char *table_stmt = "SELECT tablename FROM pg_tables WHERE tablename NOT LIKE 'pg%' AND tablename NOT LIKE 'sql%';";
  const char *tablename, *meta;
  const char *ptable_stmt = "OMLGetTableList";
  const char *schema_stmt = "SELECT value FROM _experiment_metadata WHERE key='table_' || $1;"; /* || is a concatenation */
  const char *pschema_stmt = "OMLGetTableSchema";
  const int paramFormats[] = {0};
  PGresult *res = NULL, *schema_res = NULL;
  struct schema *schema = NULL;
  int have_meta = 0;
  int i, nrows;

  /* Get a list of table names */
  res = PQprepare(self->conn, ptable_stmt, table_stmt, 0, NULL);
  if (PQresultStatus (res) != PGRES_COMMAND_OK) {
    logerror("psql:%s: Could not prepare statement %s from '%s': %s", /* PQerrorMessage strings already have '\n'  */
        database->name, ptable_stmt, table_stmt, PQerrorMessage(self->conn));
    goto fail_exit;
  }
  PQclear (res);
  res = NULL;

  /* Check if the _experiment_metadata table exists */
  res = PQexecPrepared (self->conn, ptable_stmt, 0, NULL, 0, NULL, 0);
  if (PQresultStatus (res) != PGRES_TUPLES_OK) {
    logerror("psql:%s: Could not get list of tables with '%s': %s", /* PQerrorMessage strings already have '\n'  */
        database->name, table_stmt, PQerrorMessage(self->conn));
    goto fail_exit;
  }

  nrows = PQntuples (res);
  i = -1;
  do {
    i++; /* Equivalent to sqlite3_step */
    if(i < nrows) { /* Equivalent to an SQLITE_ROW return */
      if (strcmp (PQgetvalue (res, i, 0), "_experiment_metadata") == 0) {
        logdebug("psql:%s: Found table %s\n",
            database->name, PQgetvalue (res, i, 0));
        have_meta = 1;
      }
    }
  } while (i < nrows && !have_meta);

  *num_tables = 0; /* In case !have_meta, we want it to be 0; also, we need it that way for later */

  if(!have_meta) {
    logdebug("psql:%s: _experiment_metadata table not found\n", database->name);
    /* XXX: This is probably a new database, don't exit in error */
    PQclear (res);
    return NULL;
  }

  /* Get schema for all tables */
  schema_res = PQprepare(self->conn, pschema_stmt, schema_stmt, 1, NULL);
  if (PQresultStatus (schema_res) != PGRES_COMMAND_OK) {
    logerror("psql:%s: Could not prepare statement %s from '%s': %s", /* PQerrorMessage strings already have '\n'  */
        database->name, pschema_stmt, schema_stmt, PQerrorMessage(self->conn));
    goto fail_exit;
  }
  PQclear (schema_res);
  schema_res = NULL;

  i = -1; /* Equivalent to sqlite3_reset; assume nrows is still valid */
  do {
    t = NULL;

    i++; /* Equivalent to sqlite3_step */
    if(i < nrows) { /* Equivalent to an SQLITE_ROW return */
      tablename = PQgetvalue (res, i, 0);

      if(!strcmp (tablename, "_senders")) {
        /* Create a phony entry for the _senders table some
         * server/database.c:database_init() doesn't try to create it */
        t = table_descr_new (tablename, NULL);

      } else {
        /* If it's *not* the _senders table, get its schema from the metadata table */
        logdebug2("psql:%s:%s: Trying to find schema for table %s: %s\n",
            database->name, __FUNCTION__, tablename,
            schema_stmt);

        schema_res = PQexecPrepared (self->conn, pschema_stmt, 1, &tablename, NULL, paramFormats, 0);
        if (PQresultStatus (schema_res) != PGRES_TUPLES_OK) {
          logwarn("psql:%s: Could not get schema for table %s, ignoring it: %s", /* PQerrorMessage strings already have '\n'  */
              database->name, tablename, PQerrorMessage(self->conn));

        } else if (PQntuples(schema_res) <= 0) {
          logwarn("psql:%s: No schema for table %s, ignoring it\n",
              database->name, tablename);

        } else {
          meta = PQgetvalue (schema_res, 0, 0); /* We should only have one result row */
          schema = schema_from_meta(meta);
          PQclear(schema_res);
          schema_res = NULL;

          if (!schema) {
            logwarn("psql:%s: Could not parse schema '%s' (stored in DB) for table %s, ignoring it; "
                "is your database from an oml2-server<2.10?\n",
                database->name, meta, tablename);

          } else {

            t = table_descr_new (tablename, schema);
            if (!t) {
              logerror("psql:%s: Could not create table descrition for table %s\n",
                  database->name, tablename);
              goto fail_exit;
            }
            schema = NULL; /* The pointer has been copied in t (see table_descr_new);
                              we don't want to free it twice in case of error */
          }
        }
      }

      if (t) {
        t->next = tables;
        tables = t;
        (*num_tables)++;
      }
    }
  } while (i < nrows);

  return tables;

fail_exit:
  if (tables) {
    table_descr_list_free(tables);
  }

  if (schema) {
    schema_free(schema);
  }
  if (schema_res) {
   PQclear(schema_res);
  }
  if (res) {
   PQclear(res);
  }

  *num_tables = -1;
  return NULL;
}

/** Get the sender_id for a given name in the _senders table.
 *
 * \param name name of the sender
 * \return the sender ID
 *
 * \see psql_get_key_value
 */
static char*
psql_get_sender_id (Database *database, const char *name)
{
  return psql_get_key_value (database, "_senders", "name", "id", name);
}

/** Set the sender_id for a given name in the _senders table.
 *
 * \param name name of the sender
 * \param id the ID to set
 * \return the sender ID
 *
 * \see psql_set_key_value
 */
static int
psql_set_sender_id (Database *database, const char *name, int id)
{
  MString *mstr = mstring_create();
  mstring_sprintf (mstr, "%d", id);
  int ret = psql_set_key_value (database, "_senders", "name", "id", name, mstring_buf (mstr));
  mstring_delete (mstr);
  return ret;
}

/** Receive notices from PostgreSQL and post them as an OML log message
 * \param arg application-specific state (in our case, the table name)
 * \param res a PGRES_NONFATAL_ERROR PGresult which can be used with PQresultErrorField and PQresultErrorMessage
 */
static void
psql_receive_notice(void *arg, const PGresult *res)
{
  switch(*PQresultErrorField(res, PG_DIAG_SEVERITY)) {
  case 'E': /*RROR*/
  case 'F': /*ATAL*/
  case 'P': /*ANIC*/
    logerror("psql:%s': %s", (char*)arg, PQresultErrorMessage(res));
    break;
  case 'W': /*ARNING*/
    logwarn("psql:%s': %s", (char*)arg, PQresultErrorMessage(res));
    break;
  case 'N': /*OTICE*/
  case 'I': /*NFO*/
    /* Infos and notice from Postgre are not the primary purpose of OML.
     * We only display them as debug messages. */
  case 'L': /*OG*/
  case 'D': /*EBUG*/
    logdebug("psql:%s': %s", (char*)arg, PQresultErrorMessage(res));
    break;
  default:
    logwarn("'psql:%s': Unknown notice: %s", (char*)arg, PQresultErrorMessage(res));
  }
}

/*
 Local Variables:
 mode: C
 tab-width: 2
 indent-tabs-mode: nil
 End:
 vim: sw=2:sts=2:expandtab
*/
