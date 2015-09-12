/* 
 * File:   virtuoso_adapter.c
 * Author: mpoyato
 *
 * Created on 8 de junio de 2014, 19:47
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <curl/curl.h>

// Podria ser para añadir el 
#include <oml2/omlc.h>
#include <oml2/oml_filter.h>
#include <oml2/oml_writer.h>

#include "ocomm/o_log.h"
#include "mem.h"
#include "mstring.h"
#include "htonll.h"
#include "json.h"
#include "guid.h"
#include "oml_value.h"
#include "oml_util.h"
#include "schema.h"
#include "database.h"
#include "table_descr.h"
#include "database_adapter.h"
#include "virtuoso_adapter.h"

/** Mapping between OML and SQLite3 data types
 * \see sq3_type_to_oml, sq3_oml_to_type
 */
static db_typemap sem_type_pair [] = {
  { OML_DB_PRIMARY_KEY,      "xsd:long"},
  { OML_DB_PRIMARY_KEY,      "xsd:long"},
  { OML_INT32_VALUE,         "xsd:integer"  },
  { OML_UINT32_VALUE,        "xsd:unsignedInt" },
  { OML_INT64_VALUE,         "xsd:long"  },
  { OML_UINT64_VALUE,        "xsd:unsignedLong" },
  { OML_DOUBLE_VALUE,        "xsd:double" },
  { OML_STRING_VALUE,        "xsd:string" },
  { OML_BLOB_VALUE,          "xsd:string" },
  { OML_GUID_VALUE,          "xsd:unsignedLong" },
  { OML_BOOL_VALUE,          "xsd:boolean" },
  /* Vector types are rendered as JSON-format text */
  { OML_VECTOR_DOUBLE_VALUE, "xsd:string" },
  { OML_VECTOR_INT32_VALUE,  "xsd:string" },
  { OML_VECTOR_UINT32_VALUE, "xsd:string" },
  { OML_VECTOR_INT64_VALUE,  "xsd:string" },
  { OML_VECTOR_UINT64_VALUE, "xsd:string" },
  { OML_VECTOR_BOOL_VALUE,   "xsd:string" },
  { OML_DATETIME_VALUE,   "xsd:dateTime" },
};

#define HEADER_INSERT "prefix afn: <http://jena.hpl.hp.com/ARQ/function#> \
 prefix fn: <http://www.w3.org/2005/xpath-functions#> \
 prefix owl: <http://www.w3.org/2002/07/owl#> \
 prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \
 prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> \
 prefix time: <http://www.w3.org/2006/time#> \
 prefix xsd: <http://www.w3.org/2001/XMLSchema#> \
 prefix MD: <http://www.etsi.org/isg/moi/data.owl#> \
 prefix MM: <http://www.etsi.org/isg/moi/Metadata.owl#> \
 prefix MGC: <http://www.etsi.org/isg/moi/GeneralConcepts.owl#> \
 prefix MU: <http://www.etsi.org/isg/moi/Units.owl#>"

/* Functions needed by the Database struct */
static OmlValueT sem_type_to_oml (const char *s);
static const char *sem_oml_to_type (OmlValueT T);
static int sem_stmt(Database* db, const char* stmt);
static int sem_table_create (Database* db, DbTable* table, int shallow);      //TODO
static int sem_table_free (Database *database, DbTable* table);               //TODO
static void sem_release(Database* db);
static char* sem_prepared_var(Database *db, unsigned int order);
static MString* sem_prepare(Database *db, DbTable* table);                     // TODO
static int sem_insert(Database *db, DbTable *table, int sender_id, int seq_no, double time_stamp, OmlValue *values, int value_count); // TODO
static /*char**/int sem_add_sender_id(Database* database, const char* sender_id);    // TODO
static int sem_set_metadata (Database* database, const char* key, const char* value); // TODO
static char* sem_get_metadata (Database* database, const char* key);          // TODO
static char* sem_get_uri(Database *db, char *uri, size_t size);                 // TODO
static TableDescr* sem_get_table_list (Database *database, int *num_tables);    // TODO
/*
static int sq3_insert(Database *db, DbTable *table, int sender_id, int seq_no, double time_stamp, OmlValue *values, int value_count);
static char* sq3_get_key_value (Database* database, const char* table, const char* key_column, const char* value_column, const char* key);
static int sq3_set_key_value (Database* database, const char* table, const char* key_column, const char* value_column, const char* key, const char* value);

static char* sq3_get_sender_id (Database* database, const char* name);
static int sq3_set_sender_id (Database* database, const char* name, int id);
static int sq3_get_max_value (Database* database, const char* table, const char* column_name, const char* where_column, const char* where_value);
static int sq3_get_max_sender_id (Database* database);
*/
char * str_replace ( const char *string, const char *substr, const char *replacement );

static char backend_name[] = "virtuoso";
/* Cannot be static due to the way the server sets its parameters */
char *vir_host = DEFAULT_VIR_HOST;
char *vir_port = DEFAULT_VIR_PORT;
char *vir_user = DEFAULT_VIR_USER;
char *vir_pass = DEFAULT_VIR_PASS;


#define MAXLINE 4096
#define MAXSUB  2000
/* Definition of the token to replace by sequence number */
#define REPLACE_SEQ "#?#value_seq#?#"
/* Definition of the token to replace by time of the server */
#define REPLACE_TS "#?#value_ts#?#" 
/* Definition of the token to replace by time of the server */
#define REPLACE_TC "#?#value_tc#?#"

/**
 * This function generates a new socket that connects with virtuoso backend and send some data
 * \return int - 0 (OK) -1 (KO)
 */
int virtuoso_backend_setup(void)
{
    return 0;
}

/* Never writes anything, just returns the size presented */
size_t logwrite_vir(char *ptr, size_t size, size_t nmemb, void *userdata)
{
   return size * nmemb;
}


/** Create an SQLite3 database and adapter structures
 * \see db_adapter_create
 */
/* This function is exposed to the rest of the code for backend initialisation */
int
virtuoso_create_database(Database* db)
{
  MString* mstr = mstring_create ();
  MString* userpw = mstring_create ();
  CURL *curl;
  curl_global_init(CURL_GLOBAL_ALL);
  if (!(curl = curl_easy_init())) return -1;
  mstring_sprintf(mstr,"http://%s:%s/sparql/",vir_host, vir_port);
  mstring_sprintf(userpw,"%s:%s",vir_user,vir_pass);
  curl_easy_setopt(curl, CURLOPT_URL, mstring_buf(mstr));
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &logwrite_vir);
  curl_easy_setopt(curl, CURLOPT_USERPWD, mstring_buf(userpw));
  mstring_delete (mstr);
  mstring_delete (userpw);
  VirDB* self = oml_malloc(sizeof(VirDB));
  self->conn = curl;//conn;
  self->last_commit = time (NULL);
  db->semantic = 1;
  db->backend_name = backend_name;
  db->o2t = sem_oml_to_type;
  db->t2o = sem_type_to_oml;
  db->stmt = sem_stmt;
  db->table_create = sem_table_create;
  db->table_create_meta = dba_table_create_meta;
  db->table_free = sem_table_free;
  db->release = sem_release;
  db->prepared_var = sem_prepared_var;
  db->prepare = sem_prepare;
  db->insert = sem_insert;
  db->add_sender_id = sem_add_sender_id;
  db->set_metadata = sem_set_metadata;
  db->get_metadata = sem_get_metadata;
  db->get_uri = sem_get_uri;
  db->get_table_list = sem_get_table_list;
  db->handle = self;
  dba_begin_transaction (db);
  return 0;
}

/** Mapping from SQLite3 to OML types.
 * \see db_adapter_type_to_oml
 */
static
OmlValueT sem_type_to_oml (const char *type)
{
  int i = 0;
  int n = LENGTH(sem_type_pair);

  for (i = 0; i < n; i++) {
    if (strcmp (type, sem_type_pair[i].name) == 0) {
        return sem_type_pair[i].type;
    }
  }
  logwarn("Unknown Semantic type '%s', using OML_UNKNOWN_VALUE\n", type);
  return OML_UNKNOWN_VALUE;
}

/** Mapping from OML types to SQLite3 types.
 * \see db_adapter_oml_to_type
 */
static const char*
sem_oml_to_type (OmlValueT type)
{
  int i = 0;
  int n = LENGTH(sem_type_pair);

  for (i = 0; i < n; i++) {
    if (sem_type_pair[i].type == type) {
        return sem_type_pair[i].name;
    }
  }
  logerror("Unknown OML type %d\n", type);
  return NULL;
}

/** Execute an SQL statement (using sqlite3_exec()).
 * FIXME: Clearly not prepared statements (#168)
 * FIXME: Wrap into sq3_stmt and adapt code (s/db->conn/db/)
 * \see db_adapter_stmt, sqlite3_exec
 */
static int
sparql_stmt(VirDB* self, const char* stmt)
{
  return 0;
}

/** Type-agnostic wrapper for sql_stmt */
static int
sem_stmt(Database* db, const char* stmt)
{
    return 0;
}


/** Create the adapter structures required for the SQLite3 adapter
 * \see db_adapter_table_create
 */
static int
sem_table_create (Database* db, DbTable* table, int shallow)
{
  MString *insert = NULL;
  VirDB* semdb = NULL;
  VirTable *semtable = NULL;
  if (db == NULL) {
      logwarn("virtuoso: Tried to create a schema in a NULL database\n");
      return -1;
  }
  if (table == NULL) {
    logwarn("virtuoso:%s: Tried to create a schema from a NULL definition\n", db->name);
    return -1;
  }
  if (table->schema == NULL) {
    logwarn("virtuoso:%s: No schema defined, cannot create\n", db->name);
    return -1;
  }
  semdb = (VirDB*)db->handle;

  if (!shallow) {
    if (dba_table_create_from_schema(db, table->schema)) {
      logerror("virtuoso:%s: Could not create schema '%s'\n",
          db->name, table->schema->name);
      goto fail_exit;
    }
  }
  if (table->handle != NULL) {
    logwarn("sqlite:%s: BUG: Recreating Sq3Table handle for table %s\n",
        table->schema->name);
  }
  if (!shallow) {
    semtable = (VirTable*)oml_malloc(sizeof(VirTable));
    table->handle = semtable;
    insert = database_make_sql_insert (db, table);
    if (!insert) {
      logerror ("sqlite:%s: Failed to build SQL INSERT INTO statement string for table '%s'\n",
        db->name, table->schema->name);
      goto fail_exit;
    }
    ((VirTable*)table->handle)->insert_stmt = insert;
  }
  //if (insert) { mstring_delete (insert); }
  return 0;
    
 fail_exit:
  if (insert) { mstring_delete (insert); }
  if (semtable) { oml_free (semtable); }
  return -1;
}

/** Free an Semantic table
 *
 * Parameter database is ignored in this implementation
 *
 * \see db_adapter_table_free, sqlite3_finalize
 */
static int
sem_table_free (Database *database, DbTable* table)
{
  (void)database;
  VirTable* semtable = (VirTable*)table->handle;
  int ret = 0;
  if (semtable)
    oml_free (semtable);
  return ret;
}

/** Release the Semantic database.
 * \see db_adapter_release
 */
static void
sem_release(Database* db)
{
  VirDB* self = (VirDB*)db->handle;
  dba_end_transaction (db);
  curl_easy_cleanup(self->conn);
  curl_global_cleanup();
  oml_free(self);
  db->handle = NULL;
}


/** Return a string suitable for an unbound variable is SQLite3.
 *
 * This is always "?"
 *
 * \see db_adapter_prepared_var
 */
static char*
sem_prepared_var(Database *db, unsigned int order)
{
  int nchar = 1 + (int) floor(log10(order+1))+12; /* Get the number of digits */
  char *s = oml_malloc(nchar + 1);
  if (NULL != s)
    snprintf(s, nchar + 1, "#?#value_%d#?#", order);
  return s;
}

static MString*
sem_prepare(Database *db, DbTable* table)
{
  int n = 0, c, v = 1;
  int max = table->schema->nfields;
  char *first_concept = NULL, *first_concept_aux = NULL;;
  char *pvar;
  MString* mstr = mstring_create ();
  MString* insert = mstring_create ();
  if (max <= 0) {
    logerror ("%s: Trying to insert 0 values into table %s\n",
        db->backend_name, table->schema->name);
    goto fail_exit;
  }
  if (mstr == NULL || insert == NULL) {
    logerror("%s: Failed to create managed string for preparing Semantic INSERT statement\n",
        db->backend_name);
    goto fail_exit;
  }
  // Build first part of INSERT
  for (c=0; c<max; c++) {
    struct schema_field *field = table->schema->fields+c;
    OMLSemDef*osd=field->concepts;
    if (osd) {
      if (!first_concept) {
        first_concept = osd->subject;
        n += mstring_sprintf(insert,"\t\t`iri(bif:concat(\"http://%s:%s/%s/\",bif:sequence_next('<http://%s:%s/%s>')))` a %s .\n",
            vir_host,vir_port,table->schema->name,vir_host,vir_port,db->name,osd->subject);
        n += mstring_sprintf(insert,"\t\t`iri(bif:concat(\"http://%s:%s/%s/\",bif:sequence_next('<http://%s:%s/%s>')))` rdfs:label \"%s\" .\n",
            vir_host,vir_port,table->schema->name,vir_host,vir_port,db->name,table->schema->name);
        n += mstring_sprintf(insert,"\t\t`iri(bif:concat(\"http://%s:%s/%s/\",bif:sequence_next('<http://%s:%s/%s>')))` MD:hasMeasurementData `iri(bif:concat(\"http://%s:%s/seq_no/\",bif:sequence_next('<http://%s:%s/%s>')))` .\n",
            vir_host,vir_port,table->schema->name,vir_host,vir_port,db->name,vir_host,vir_port,vir_host,vir_port,db->name);
        n += mstring_sprintf(insert,"\t\t`iri(bif:concat(\"http://%s:%s/%s/\",bif:sequence_next('<http://%s:%s/%s>')))` MD:hasMeasurementData `iri(bif:concat(\"http://%s:%s/oml_ts_client/\",bif:sequence_next('<http://%s:%s/%s>')))` .\n",
            vir_host,vir_port,table->schema->name,vir_host,vir_port,db->name,vir_host,vir_port,vir_host,vir_port,db->name);
        n += mstring_sprintf(insert,"\t\t`iri(bif:concat(\"http://%s:%s/%s/\",bif:sequence_next('<http://%s:%s/%s>')))` MD:hasMeasurementData `iri(bif:concat(\"http://%s:%s/oml_ts_server/\",bif:sequence_next('<http://%s:%s/%s>')))` .\n",
            vir_host,vir_port,table->schema->name,vir_host,vir_port,db->name,vir_host,vir_port,vir_host,vir_port,db->name);
        // Build seq_no virtuoso insertion
        n += mstring_sprintf(insert,"\t\t`iri(bif:concat(\"http://%s:%s/seq_no/\",bif:sequence_next('<http://%s:%s/%s>')))` a MD:SequenceIndex .\n",
            vir_host,vir_port,vir_host,vir_port,db->name);
        n += mstring_sprintf(insert,"\t\t`iri(bif:concat(\"http://%s:%s/seq_no/\",bif:sequence_next('<http://%s:%s/%s>')))` MD:SequenceIndexValue %s .\n",
            vir_host,vir_port,vir_host,vir_port,db->name,REPLACE_SEQ);
        // Build current time (oml client) virtuoso insertion
        n += mstring_sprintf(insert,"\t\t`iri(bif:concat(\"http://%s:%s/oml_ts_client/\",bif:sequence_next('<http://%s:%s/%s>')))` a MD:Time .\n",
            vir_host,vir_port,vir_host,vir_port,db->name);
        n += mstring_sprintf(insert,"\t\t`iri(bif:concat(\"http://%s:%s/oml_ts_client/\",bif:sequence_next('<http://%s:%s/%s>')))` MD:TimeValue %s .\n",
            vir_host,vir_port,vir_host,vir_port,db->name, REPLACE_TC);
        // Build current time (oml server) virtuoso insertion
        n += mstring_sprintf(insert,"\t\t`iri(bif:concat(\"http://%s:%s/oml_ts_server/\",bif:sequence_next('<http://%s:%s/%s>')))` a MD:Time .\n",
            vir_host,vir_port,vir_host,vir_port,db->name);
        n += mstring_sprintf(insert,"\t\t`iri(bif:concat(\"http://%s:%s/oml_ts_server/\",bif:sequence_next('<http://%s:%s/%s>')))` MD:TimeValue `now()` .\n",
            vir_host,vir_port,vir_host,vir_port,db->name);
      }
      if (strcmp(first_concept,osd->subject)) {
        logerror("%s: Failed in specification of the concepts, subject: %s is different to subject: %s.\n",
            db->backend_name, first_concept, osd->subject);
        goto fail_exit;
      }
      n += mstring_sprintf(insert,"\t\t`iri(bif:concat(\"http://%s:%s/%s/\",bif:sequence_next('<http://%s:%s/%s>')))` %s `iri(bif:concat(\"http://%s:%s/%s/\",bif:sequence_next('<http://%s:%s/%s>')))` .\n",
        vir_host,vir_port,table->schema->name,vir_host,vir_port,db->name,osd->predicate, vir_host,vir_port,field->name,vir_host,vir_port,db->name);
      first_concept_aux = osd->object;
      n += mstring_sprintf(insert,"\t\t`iri(bif:concat(\"http://%s:%s/%s/\",bif:sequence_next('<http://%s:%s/%s>')))` a %s .\n",
        vir_host,vir_port,field->name,vir_host,vir_port,db->name, first_concept_aux);
      osd = osd->next;
      if (!osd) {
          logerror("%s: Failed in specification of the concepts, subject: %s doesnt have more info.\n",
            db->backend_name, first_concept);
        goto fail_exit;
      }
      n += mstring_sprintf(insert,"\t\t`iri(bif:concat(\"http://%s:%s/%s/\",bif:sequence_next('<http://%s:%s/%s>')))` rdfs:label \"%s\" .\n",
        vir_host,vir_port,field->name,vir_host,vir_port,db->name, field->name);
      while (osd) {
        if (strcmp(first_concept_aux,osd->subject)) {
          logerror("%s: Failed in specification of the concepts, subject: %s is different to subject: %s.\n",
              db->backend_name, first_concept, osd->subject);
          goto fail_exit;
        }
        if (!strcmp(osd->object,"%value%"))
        {
            pvar = sem_prepared_var(db,c);
            n += mstring_sprintf(insert,"\t\t`iri(bif:concat(\"http://%s:%s/%s/\",bif:sequence_next('<http://%s:%s/%s>')))` %s %s .\n", 
                vir_host,vir_port,field->name,vir_host,vir_port,db->name, osd->predicate, pvar);
            oml_free(pvar);
        }
        else
            n += mstring_sprintf(insert,"\t\t`iri(bif:concat(\"http://%s:%s/%s/\",bif:sequence_next('<http://%s:%s/%s>')))` %s %s .\n", 
                vir_host,vir_port,field->name,vir_host,vir_port,db->name, osd->predicate, osd->object);
        osd = osd->next;
      }
      v++;
    }
  }
  if (mstring_buf(insert)&&mstring_buf(insert)[0])
      n += mstring_sprintf(mstr,"%s\nINSERT\n{\n\tGRAPH <http://%s:%s/%s>\n\t{\n%s\t}\n}",
        HEADER_INSERT,vir_host,vir_port,db->name,mstring_buf(insert));
  if (n != 0)
    goto fail_exit;
  return mstr;
 fail_exit:
  if (mstr) mstring_delete (mstr);
  return NULL;
}

/** Insert value in the SQLite3 database.
 * \see db_adapter_insert
 * XXX: This function actively does text protocol interpretation, see #1088
 */
static int
sem_insert(Database *db, DbTable *table, int sender_id, int seq_no, double time_stamp, OmlValue *v, int value_count)
{
  VirDB* semdb = (VirDB*)db->handle;
  VirTable* semtable = (VirTable*)table->handle;
  
  if (semtable && semtable->insert_stmt) {
    int i, res = 0;
    long http_code;
    char* tok = NULL, *old_tok = NULL,*pvar;
    char *json = NULL;
    ssize_t json_sz;
    char* stmt = xstrdup(mstring_buf(semtable->insert_stmt));
    MString *stmtend = mstring_create (), *stmtdata = mstring_create ();
    double time_stamp_server;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_stamp_server = tv.tv_sec - db->start_time + 0.000001 * tv.tv_usec;

    if (tv.tv_sec > semdb->last_commit) {
      if (dba_reopen_transaction (db) == -1) {
        mstring_delete(stmtdata);
        return -1;
      }
      semdb->last_commit = tv.tv_sec;
    }

    if (stmtend == NULL) {
      logerror("%d: Failed to create managed string for preparing SQL INSERT statement\n",
          db->backend_name);
      mstring_delete(stmtdata);
      return -1;
    }
    struct schema *schema = table->schema;
    if (schema->nfields != value_count) {
      logerror ("virtuoso:%s: Failed to insert %d values into table '%s' with %d columns\n",
          db->name, value_count, table->schema->name, schema->nfields);
      oml_free(stmt);
      mstring_delete(stmtend);
      mstring_delete(stmtdata);
      return -1;
    }
    old_tok = stmt;
    // Asign sequence index to the Measurement
    if ((tok = strstr(old_tok, REPLACE_SEQ))) {
        *tok = '\0';
        res += mstring_sprintf(stmtend, "%s\"%d\"^^xsd:integer", old_tok, seq_no);
        old_tok = tok+strlen(REPLACE_SEQ);
    }
    if ((tok = strstr(old_tok, REPLACE_TC))) {
        *tok = '\0';
        res += mstring_sprintf(stmtend, "%s\"%d\"^^xsd:double", old_tok, time_stamp);
        old_tok = tok+strlen(REPLACE_TC);
    }
    pvar = sem_prepared_var(db,0);
    for (i = 0; i < schema->nfields;i++) {
      if ((tok = strstr(old_tok, pvar))) {
        if (oml_value_get_type(v+i) != schema->fields[i].type) {
          const char *expected = oml_type_to_s (schema->fields[i].type);
          const char *received = oml_type_to_s (oml_value_get_type(v));
          logerror("virtuoso:%s: Value %d type mismatch for table '%s'\n", db->name, i, table->schema->name);
          logdebug("virtuoso:%s: -> Column name='%s', type=%s, but trying to insert a %s\n",
              db->name, schema->fields[i].name, expected, received);
          oml_free(pvar);
          oml_free(stmt);
          mstring_delete(stmtend);
          mstring_delete(stmtdata);
          return -1;
        }
        *tok = '\0';
        switch (schema->fields[i].type) {
        case OML_DOUBLE_VALUE:
          res += mstring_sprintf(stmtend, "%s\"%lf\"^^%s", old_tok, omlc_get_double(*oml_value_get_value(v+i)), sem_oml_to_type (schema->fields[i].type));
          break;
        case OML_LONG_VALUE:
          res += mstring_sprintf(stmtend, "%s\"%ld\"^^%s", old_tok, (long)omlc_get_long(*oml_value_get_value(v+i)), sem_oml_to_type (schema->fields[i].type));
          break;
        case OML_INT32_VALUE:
          res += mstring_sprintf(stmtend, "%s\"%d\"^^%s", old_tok, (int32_t)omlc_get_int32(*oml_value_get_value(v+i)), sem_oml_to_type (schema->fields[i].type));
          break;
        case OML_UINT32_VALUE:
          res += mstring_sprintf(stmtend, "%s\"%d\"^^%s", old_tok, (uint32_t)omlc_get_uint32(*oml_value_get_value(v+i)), sem_oml_to_type (schema->fields[i].type));
          break;
        case OML_INT64_VALUE:
          res += mstring_sprintf(stmtend, "%s\"%ld\"^^%s", old_tok, (int64_t)omlc_get_int64(*oml_value_get_value(v+i)), sem_oml_to_type (schema->fields[i].type));
          break;
        case OML_UINT64_VALUE:
          if (omlc_get_uint64(*oml_value_get_value(v+i)) > (uint64_t)9223372036854775808ull) {
            logwarn("virtuoso:%s: Trying to store value %" PRIu64 " (>2^63) in column '%s' of table '%s', this might lead to a loss of resolution\n",
                db->name, (uint64_t)omlc_get_uint64(*oml_value_get_value(v+i)), schema->fields[i].name, table->schema->name);
          }
          res += mstring_sprintf(stmtend, "%s\"%lu\"^^%s", old_tok, (uint64_t)omlc_get_uint64(*oml_value_get_value(v+i)), sem_oml_to_type (schema->fields[i].type));
          break;
        case OML_STRING_VALUE:
          res += mstring_sprintf(stmtend, "%s\"%s\"^^%s", old_tok, omlc_get_string_ptr(*oml_value_get_value(v+i)), sem_oml_to_type (schema->fields[i].type));
          break;
    // Not supported
    //    case OML_BLOB_VALUE:
    //      break;
        case OML_GUID_VALUE:
          if(omlc_get_guid(*oml_value_get_value(v+i)) != UINT64_C(0)) {
            res += mstring_sprintf(stmtend, "%s\"%lu\"^^%s", old_tok, (uint64_t)omlc_get_uint64(*oml_value_get_value(v+i)), sem_oml_to_type (schema->fields[i].type));
          } else {
            res += mstring_sprintf(stmtend, "%s\"\"^^%s", old_tok, sem_oml_to_type (schema->fields[i].type));
          }
          break;

        case OML_BOOL_VALUE:
          res += mstring_sprintf(stmtend, "%s\"%s\"^^%s", old_tok, (int)omlc_get_bool(*oml_value_get_value(v+i))?"true":"false", sem_oml_to_type (schema->fields[i].type));
          break;

        case OML_VECTOR_DOUBLE_VALUE:
          json = NULL;
          json_sz = vector_double_to_json((v+i)->value.vectorValue.ptr, (v+i)->value.vectorValue.nof_elts, &json);
          if(-1 != json_sz)
          {
            res += mstring_sprintf(stmtend, "%s\"%s\"^^%s", old_tok, json, sem_oml_to_type (schema->fields[i].type));
            oml_free(json);
          }
          else
            res += mstring_sprintf(stmtend, "%s\"\"^^%s", old_tok, sem_oml_to_type (schema->fields[i].type));
          break;
        case OML_VECTOR_INT32_VALUE:
          json = NULL;
          json_sz = vector_int32_to_json((v+i)->value.vectorValue.ptr, (v+i)->value.vectorValue.nof_elts, &json);
          if(-1 != json_sz)
          {
            res += mstring_sprintf(stmtend, "%s\"%s\"^^%s", old_tok, json, sem_oml_to_type (schema->fields[i].type));
            oml_free(json);
          }
          else
            res += mstring_sprintf(stmtend, "%s\"\"^^%s", old_tok, sem_oml_to_type (schema->fields[i].type));
          break;
        case OML_VECTOR_UINT32_VALUE:
          json = NULL;
          json_sz = vector_uint32_to_json((v+i)->value.vectorValue.ptr, (v+i)->value.vectorValue.nof_elts, &json);
          if(-1 != json_sz)
          {
            res += mstring_sprintf(stmtend, "%s\"%s\"^^%s", old_tok, json, sem_oml_to_type (schema->fields[i].type));
            oml_free(json);
          }
          else
            res += mstring_sprintf(stmtend, "%s\"\"^^%s", old_tok, sem_oml_to_type (schema->fields[i].type));
          break;
        case OML_VECTOR_INT64_VALUE:
          json = NULL;
          json_sz = vector_int64_to_json((v+i)->value.vectorValue.ptr, (v+i)->value.vectorValue.nof_elts, &json);
          if(-1 != json_sz)
          {
            res += mstring_sprintf(stmtend, "%s\"%s\"^^%s", old_tok, json, sem_oml_to_type (schema->fields[i].type));
            oml_free(json);
          }
          else
            res += mstring_sprintf(stmtend, "%s\"\"^^%s", old_tok, sem_oml_to_type (schema->fields[i].type));
          break;
        case OML_VECTOR_UINT64_VALUE:
          json = NULL;
          json_sz = vector_uint64_to_json((v+i)->value.vectorValue.ptr, (v+i)->value.vectorValue.nof_elts, &json);
          if(-1 != json_sz)
          {
            res += mstring_sprintf(stmtend, "%s\"%s\"^^%s", old_tok, json, sem_oml_to_type (schema->fields[i].type));
            oml_free(json);
          }
          else
            res += mstring_sprintf(stmtend, "%s\"\"^^%s", old_tok, sem_oml_to_type (schema->fields[i].type));
          break;
        case OML_VECTOR_BOOL_VALUE:
          json = NULL;
          json_sz = vector_bool_to_json((v+i)->value.vectorValue.ptr, (v+i)->value.vectorValue.nof_elts, &json);
          if(-1 != json_sz)
          {
            res += mstring_sprintf(stmtend, "%s\"%s\"^^%s", old_tok, json, sem_oml_to_type (schema->fields[i].type));
            oml_free(json);
          }
          else
            res += mstring_sprintf(stmtend, "%s\"\"^^%s", old_tok, sem_oml_to_type (schema->fields[i].type));
          break;
        default:
          logerror("virtuoso:%s: Unknown type %d in col '%s' of table '%s; this is probably a bug'\n",
              db->name, schema->fields[i].type, schema->fields[i].name, table->schema->name);
          oml_free(stmt);
          mstring_delete(stmtend);
          mstring_delete(stmtdata);
          return -1;
        }
        if (res != 0) {
          logerror("virtuoso:%s: Could not bind column '%s'\n",
              db->name, schema->fields[i].name);
          oml_free(stmt);
          mstring_delete(stmtend);
          mstring_delete(stmtdata);
          return -1;
        }
        old_tok = tok+strlen(pvar); // go to next segment
      }
      oml_free(pvar);
      pvar = sem_prepared_var(db,i+1);
    }
    oml_free(pvar);
    res += mstring_sprintf(stmtend, "%s", old_tok);
    if (old_tok != stmt)
    {
        char*data=curl_easy_escape(semdb->conn,mstring_buf(stmtend),strlen(mstring_buf(stmtend)));
        res += mstring_sprintf(stmtdata,"query=%s",data);
        curl_easy_setopt(semdb->conn, CURLOPT_POSTFIELDS, mstring_buf(stmtdata));
        curl_free(data);
        res = curl_easy_perform(semdb->conn);
        /* Check for errors */ 
        if (res != CURLE_OK)
          logerror("virtuoso:%s: Semantic insertion failed: %s\n",db->name,curl_easy_strerror(res));
        curl_easy_getinfo (semdb->conn, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code != 200L)
        {
          logerror("virtuoso:%s: Semantic insertion failed. HTTP code status: %ld\n",db->name,http_code);
          logdebug3("virtuoso:%s: %s\n",db->name,mstring_buf(stmtdata));
        }
    }
    oml_free(stmt);
    mstring_delete(stmtdata);
    mstring_delete(stmtend);
  }
  return 0;
}

/** Add a new sender to the database, returning its index.
 * \see db_add_sender_id
 */
static int /*char*/
sem_add_sender_id(Database* database, const char* sender_id)
{
  return -1;
}

/** Set data in the metadata table
 * \see db_adapter_set_metadata, sq3_set_key_value
 */
static int
sem_set_metadata (Database* database, const char* key, const char* value)
{
  return 0;
}

/** Get data from the metadata table
 * \see db_adapter_get_metadata, sq3_get_key_value
 */
static char*
sem_get_metadata (Database* database, const char* key)
{
  return 0;
}

/** Build a URI for this database.
 *
 * URI is of the form file:PATH/DATABASE.sq3
 *
 * \see db_adapter_get_uri
 */
static char*
sem_get_uri(Database *db, char *uri, size_t size)
{
    return "";
}

/** Get a list of tables in an SQLite3 database
 * \see db_adapter_get_table_list
 */
static TableDescr*
sem_get_table_list (Database *database, int *num_tables)
{
    return NULL;
}
