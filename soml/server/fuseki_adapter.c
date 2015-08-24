/* 
 * File:   fuseki_adapter.c
 * Author: mpoyato
 *
 * Created on 8 de junio de 2014, 19:47
 * Modified by Andisa Dewi, Yahya Al-Hazmi, Technische Universitaet Berlin, 2015
 *  to support OMN Ontology
 */

#include <stdio.h>
#include <stdlib.h>
 #include <stdarg.h>
#include <limits.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <curl/curl.h>
#include <assert.h>

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
#include "fuseki_adapter.h"
#include "cJSON.h"

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

struct string {
  char *ptr;
  size_t len;
};

#define HEADER_INSERT "prefix afn: <http://jena.hpl.hp.com/ARQ/function#> \
 prefix fn: <http://www.w3.org/2005/xpath-functions#> \
 prefix owl: <http://www.w3.org/2002/07/owl#> \
 prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \
 prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> \
 prefix time: <http://www.w3.org/2006/time#> \
 prefix xsd: <http://www.w3.org/2001/XMLSchema#> \
 prefix omn-monitoring-data: <http://open-multinet.info/ontology/omn-monitoring-data#> \
 prefix omn-monitoring-genericconcepts: <http://open-multinet.info/ontology/omn-monitoring-genericconcepts#> \
 prefix omn-monitoring-metric: <http://open-multinet.info/ontology/omn-monitoring-metric#> \
 prefix omn-monitoring-tool: <http://open-multinet.info/ontology/omn-monitoring-tool#> \
 prefix omn-monitoring-unit: <http://open-multinet.info/ontology/omn-monitoring-unit#> \
 prefix omn-monitoring: <http://open-multinet.info/ontology/omn-monitoring#> \
 prefix omn: <http://open-multinet.info/ontology/omn#> \
 prefix omn-resource: <http://open-multinet.info/ontology/omn-resource#> \
 prefix omn-lifecycle: <http://open-multinet.info/ontology/omn-lifecycle#> \
 prefix omn-component: <http://open-multinet.info/ontology/omn-component#> \
 prefix omn-federation: <http://open-multinet.info/ontology/omn-federation#> \
 prefix omn-policy: <http://open-multinet.info/ontology/omn-policy#> \
 prefix omn-service: <http://open-multinet.info/ontology/omn-service#> \
 prefix omn-domain-pc: <http://open-multinet.info/ontology/omn-domain-pc#> "

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
static char* sem_add_sender_id(Database* database, const char* sender_id);    // TODO
static int sem_set_metadata (Database* database, const char* key, const char* value); // TODO
static char* sem_get_metadata (Database* database, const char* key);          // TODO
static char* sem_get_uri(Database *db, char *uri, size_t size);                 // TODO
static TableDescr* sem_get_table_list (Database *database, int *num_tables);    // TODO
static int update_starttime(Database *db, DbTable *table) ;
static int starttime_exist(Database *db, DbTable* table);
static int service_exist(Database *db, DbTable* table);
static int sender_exist(Database *db, DbTable* table);
static int domain_exist(Database *db, DbTable *table) ;
void init_string(struct string *s) ;
size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s) ;
char* parse_object(cJSON *root, char* res_name) ;
//static char** str_split(char* a_str, const char a_delim) ;

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

static char backend_name[] = "fuseki";
/* Cannot be static due to the way the server sets its parameters */
char *fus_host = DEFAULT_FUS_HOST;
char *fus_port = DEFAULT_FUS_PORT;
char *fus_namespace = DEFAULT_FUS_NAMESPACE;

char* sid ;
MString* sender_uri ;
MString* domain_uri ;
MString* service_uri ;
int sender_uri_exist = 0 ;
int domain_uri_exist = 0 ;
int service_uri_exist = 0 ;
int set_starttime = 0 ;
int starttime = 0 ;

#define MAXLINE 4096
#define MAXSUB  2000
/* Definition of the token to replace by sequence number */
#define REPLACE_SEQ "#?#value_seq#?#"
/* Definition of the token to replace by time of the server */
#define REPLACE_TS "#?#value_ts#?#" 
/* Definition of the token to replace by time of the server */
#define REPLACE_TC "#?#value_tc#?#"
#define REPLACE_SID "#?#value_sid#?#"
#define REPLACE_SID_URI "#?#value_sid_uri#?#"
#define REPLACE_STARTTIME "#?#value_st#?#"
#define REPLACE_PHYRESOURCE "#?#value_phyresource#?#"
#define REPLACE_VIRRESOURCE "#?#value_virresource#?#"
// ######################################################################

#define HASHSIZE 101
static struct nlist *hashtab[HASHSIZE]; /* pointer table */

/* hash: form hash value for string s */
unsigned hash(char *s)
{
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
      hashval = *s + 31 * hashval;
    return hashval % HASHSIZE;
}

/* lookup: look for s in hashtab */
char* lookup(char *s)
{
    struct nlist *np;
    for (np = hashtab[hash(s)]; np != NULL; np = np->next)
        if (strcmp(s, np->name) == 0)
          return np; /* found */
    return NULL; /* not found */
}

char* lookup_char(char *s)
{
    struct nlist *np;
    for (np = hashtab[hash(s)]; np != NULL; np = np->next)
        if (strcmp(s, np->name) == 0)
          return np->defn; /* found */
    return NULL; /* not found */
}


char *strdupp(char *s) /* make a duplicate of s */
{
    char *p;
    p = (char *) malloc(strlen(s)+1); /* +1 for ’\0’ */
    if (p != NULL)
       strcpy(p, s);
    return p;
}

/* install: put (name, defn) in hashtab */
struct nlist *install(char *name, char *defn)
{
    struct nlist *np;
    unsigned hashval;
    if ((np = lookup(name)) == NULL) { /* not found */
        np = (struct nlist *) malloc(sizeof(*np));
        if (np == NULL || (np->name = strdupp(name)) == NULL)
          return NULL;
        hashval = hash(name);
        np->next = hashtab[hashval];
        hashtab[hashval] = np;
    } else /* already there */
        free((void *) np->defn); /*free previous defn */
    if ((np->defn = strdupp(defn)) == NULL)
       return NULL;
    return np;
}

void init_list()
{
  struct nlist *list = hashtab ;
  //list = install("omn-monitoring-data:MeasurementData","?data") ;
}

void free_list() 
{
    int i ;
    for (i = 0; i < HASHSIZE; i++) {
        free((void *) hashtab[i]->name);
        free((void *) hashtab[i]->defn);
    }
    //free(hashtab);
}

// ###############################################################
void handleValue(Database *db, int c, MString* insert, char *subject, char *predicate, char *pvar, int n){
  pvar = sem_prepared_var(db,c);
  n += mstring_sprintf(insert,"\t\t%s %s %s .\n", subject, predicate, pvar);
  oml_free(pvar);
}

// ###############################################################

/**
 * This function generates a new socket that connects with fuseki backend and send some data
 * \return int - 0 (OK) -1 (KO)
 */
int fuseki_backend_setup(void)
{
    return 0; // OK = 0 || KO = -1
}

/* Never writes anything, just returns the size presented */
size_t logwrite(char *ptr, size_t size, size_t nmemb, void *userdata)
{
   logwarn("Server response: %s\n",ptr);
   return size * nmemb;
}


/** Create an SQLite3 database and adapter structures
 * \see db_adapter_create
 */
/* This function is exposed to the rest of the code for backend initialisation */
int
fuseki_create_database(Database* db)
{
  // MString* mstr = mstring_create ();
  // CURL *curl;
  // curl_global_init(CURL_GLOBAL_ALL);
  // if (!(curl = curl_easy_init())) return -1;
  // mstring_sprintf(mstr,"http://%s:%s/%s/update",fus_host, fus_port, fus_namespace);
  // loginfo("%s\n",mstring_buf(mstr));
  // curl_easy_setopt(curl, CURLOPT_URL, mstring_buf(mstr));
  // curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &logwrite);
  SemDB* self = oml_malloc(sizeof(SemDB));
  // self->conn = curl;//conn;
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
sparql_stmt(SemDB* self, const char* stmt)
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
  SemDB* semdb = NULL;
  SemTable *semtable = NULL;
  if (db == NULL) {
      logwarn("fuseki: Tried to create a schema in a NULL database\n");
      return -1;
  }
  if (table == NULL) {
    logwarn("fuseki:%s: Tried to create a schema from a NULL definition\n", db->name);
    return -1;
  }
  if (table->schema == NULL) {
    logwarn("fuseki:%s: No schema defined, cannot create\n", db->name);
    return -1;
  }
  semdb = (SemDB*)db->handle;

  if (!shallow) {
    if (dba_table_create_from_schema(db, table->schema)) {
      logerror("fuseki:%s: Could not create schema '%s'\n",
          db->name, table->schema->name);
      goto fail_exit;
    }
  }
  if (table->handle != NULL) {
    logwarn("sqlite:%s: BUG: Recreating Sq3Table handle for table %s\n",
        table->schema->name);
  }
  if (!shallow) {
    semtable = (SemTable*)oml_malloc(sizeof(SemTable));
    table->handle = semtable;
    insert = database_make_sql_insert (db, table);
    if (!insert) {
      logerror ("sqlite:%s: Failed to build SQL INSERT INTO statement string for table '%s'\n",
        db->name, table->schema->name);
      goto fail_exit;
    }
    ((SemTable*)table->handle)->insert_stmt = insert;
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
  SemTable* semtable = (SemTable*)table->handle;
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
  SemDB* self = (SemDB*)db->handle;
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

  (void)db;
  
  if (NULL != s) {
    snprintf(s, nchar + 1, "#?#value_%d#?#", order);
  }
  
  return s;
}

static MString*
sem_prepare(Database *db, DbTable* table)
{
  int n = 0, c, v = 1 ;
  int max = table->schema->nfields;
  char *first_concept = NULL, *first_concept_aux = NULL ;
  char *pvar;
  char *subject = NULL, *object = NULL;
  char temp[1024] ;
  struct nlist *list ;
  MString* mstr = mstring_create ();
  MString* insert = mstring_create ();
  MString* where = mstring_create ();

  if (max <= 0) {
    logerror ("%s: Trying to insert 0 values into table %s\n",
        db->backend_name, table->schema->name);
    goto fail_exit;
  }
  if (mstr == NULL || insert == NULL || where == NULL) {
    logerror("%s: Failed to create managed string for preparing Semantic INSERT statement\n",
        db->backend_name);
    goto fail_exit;
  }
  // Build first part of INSERT
  printf("preparing...") ;
  for (c=0; c<max; c++) {
    struct schema_field *field = table->schema->fields+c;
    OMLSemDef*osd=field->concepts;
    if (osd) {
     
        // =============================== omn-monitoring modification ===============================
      if(!first_concept){
	       memset(&hashtab, 0, sizeof(hashtab));
        init_list() ;
        char *metric = osd->object ;
	list = install(osd->subject,"?measurement") ;
        list = install(metric,"?metric") ;
        first_concept = osd->subject ;
 
        n += mstring_sprintf(insert,"\t\t?measurement a %s .\n", osd->subject);
        n += mstring_sprintf(insert,"\t\t?measurement %s ?metric .\n", osd->predicate);
        n += mstring_sprintf(insert,"\t\t?metric rdfs:label \"%s\"^^xsd:string .\n", table->schema->name);
        n += mstring_sprintf(insert,"\t\t?metric a %s .\n", osd->object);
        n += mstring_sprintf(where,"\tBIND (URI(CONCAT(\"http://%s:%s/%s/%s/\", ?struuid )) as ?measurement) .\n",fus_host,fus_port,"measurement",table->schema->name);
        //n += mstring_sprintf(where,"\tBIND (URI(CONCAT(\"http://%s:%s/%s/%s/\", ?struuid )) as ?data) .\n",fus_host,fus_port,"measurement_data",table->schema->name);

        if(sender_exist(db, table) == 0){
          sender_uri = mstring_create () ;
          mstring_sprintf(sender_uri, "http://%s:%s/%s", fus_host,fus_port, sid);
        }
        n += mstring_sprintf(where,"\tBIND (URI(\"%s/%s/%s\") as ?metric) .\n", mstring_buf(sender_uri),"metric",table->schema->name) ;

	n += mstring_sprintf(insert,"\t\t?measurement omn:sequenceNumber %s .\n", REPLACE_SEQ);
	n += mstring_sprintf(insert,"\t\t?measurement omn-monitoring:elapsedTimeAtClientSinceExperimentStartedInSeconds %s .\n", REPLACE_TC);
	n += mstring_sprintf(insert,"\t\t?measurement omn-monitoring:elapsedTimeAtServerSinceExperimentStartedInSeconds %s .\n", REPLACE_TS);
   	n += mstring_sprintf(insert,"\t\t?measurement omn-monitoring:sentFrom ?sender .\n");
   	n += mstring_sprintf(insert,"\t\t?sender a omn-federation:Infrastructure .\n");
   	n += mstring_sprintf(insert,"\t\t?sender rdfs:label \"%s\"^^xsd:string .\n", sid);
   	n += mstring_sprintf(where,"\tBIND (URI(\"%s\") as ?sender) .\n",mstring_buf(sender_uri));
       	n += mstring_sprintf(insert,"\t\t?measurement omn-monitoring:sentFrom ?domain .\n");
	n += mstring_sprintf(insert,"\t\t?domain a omn-monitoring-genericconcepts:MonitoringDomain .\n");
	n += mstring_sprintf(insert,"\t\t?domain rdfs:label \"%s\"^^xsd:string .\n", db->name);

        if(domain_exist(db, table) == 0){
          domain_uri = mstring_create () ;
          mstring_sprintf(domain_uri, "http://%s:%s/%s", fus_host,fus_port, db->name);
        }
        n += mstring_sprintf(where,"\tBIND (URI(\"%s\") as ?domain) .\n",mstring_buf(domain_uri));

  	if(service_exist(db, table) == 0){
          	service_uri = mstring_create () ;
          	mstring_sprintf(service_uri, "http://%s:%s/%s_%s/%s", fus_host,fus_port, sid, db->name, "monitoring_service");
        	n += mstring_sprintf(insert,"\t\t?domain omn:hasService ?monservice .\n") ;
        	n += mstring_sprintf(insert,"\t\t?monservice a omn-monitoring:MonitoringService .\n");
          	n += mstring_sprintf(insert,"\t\t?monservice omn:isServiceOf ?sender .\n");
         	n += mstring_sprintf(insert,"\t\t?monservice omn-lifecycle:StartTime \"%d\"^^xsd:integer .\n", starttime);
          	n += mstring_sprintf(where,"\tBIND (URI(\"%s\") as ?monservice) .\n",mstring_buf(service_uri));
	}else{
          update_starttime(db, table) ;
  }
        osd = osd->next ;
      }
      
      while(osd){
        subject = lookup_char(osd->subject) ;
        object = lookup_char(osd->object) ;  
       
	     if(subject && !object){
          if(!strcmp(osd->predicate,"omn-monitoring:isMeasurementMetricOf")){
              snprintf(temp, sizeof(temp), "?v%d", v);
              n += mstring_sprintf(insert,"\t\t%s %s ?v%d .\n", subject, osd->predicate, v);
              n += mstring_sprintf(insert,"\t\t?v%d a %s .\n", v, osd->object);
              list = install(osd->object,temp) ;
              v++;
          }
          else if(subject && !strcmp(osd->predicate,"rdfs:label") && !strcmp(osd->object,"%value%")){
            handleValue(db, c, insert, subject, osd->predicate, pvar, n) ;
            n += mstring_sprintf(where,"\tBIND (URI(%s) as %s) .\n", REPLACE_PHYRESOURCE, subject);
          } else if(!strcmp(osd->object,"%value%")){
            handleValue(db, c, insert, subject, osd->predicate, pvar, n) ;
          }
          else{
            /*if(!strcmp(osd->predicate,"rdf:type")){
              n += mstring_sprintf(insert,"\t\t%s a %s .\n", subject, osd->object);
            }
	          else{*/
              snprintf(temp, sizeof(temp), "?v%d", v);
              n += mstring_sprintf(insert,"\t\t%s %s ?v%d .\n", subject, osd->predicate, v);
              n += mstring_sprintf(insert,"\t\t?v%d a %s .\n", v, osd->object);
              list = install(osd->object,temp) ;
  	          n += mstring_sprintf(where,"\tBIND (URI(CONCAT(\"http://%s:%s/%s/%s/v%d/\", ?struuid )) as ?v%d) .\n",fus_host,fus_port,table->schema->name,field->name,v,v);
  	          v++ ;
            //}
          }
        }

        else if(!subject && object){
          if(!strcmp(osd->object,"%value%")){
            handleValue(db, c, insert, subject, osd->predicate, pvar, n) ;
          }
          else{
            snprintf(temp, sizeof(temp), "?v%d", v) ;
            n += mstring_sprintf(insert,"\t\t?v%d %s %s .\n", v, osd->predicate, object);
            n += mstring_sprintf(insert,"\t\t?v%d a %s .\n", v, osd->subject);
            list = install(osd->subject,temp) ;
	    n += mstring_sprintf(where,"\tBIND (URI(CONCAT(\"http://%s:%s/%s/%s/v%d/\", ?struuid )) as ?v%d) .\n",fus_host,fus_port,table->schema->name,field->name,v,v);
	    v++ ;
          }
        }

        else if(!subject && !object){
          snprintf(temp, sizeof(temp), "?v%d", v) ;
          n += mstring_sprintf(insert,"\t\t?v%d a %s .\n", v, osd->subject);
          list = install(osd->subject,temp) ;

          if(!strcmp(osd->predicate,"rdfs:label") && !strcmp(osd->object,"%value%")){
              n += mstring_sprintf(where,"\tBIND (URI(%s) as %s) .\n", REPLACE_VIRRESOURCE, temp);
          }else{
	         n += mstring_sprintf(where,"\tBIND (URI(CONCAT(\"http://%s:%s/%s/%s/v%d/\", ?struuid )) as ?v%d) .\n",fus_host,fus_port,table->schema->name,field->name,v,v);
          }
          
          if(!strcmp(osd->object,"%value%")){
	           subject = lookup_char(osd->subject) ;
             handleValue(db, c, insert, subject, osd->predicate, pvar, n) ;
          }
          else{
            /*if(!strcmp(osd->predicate,"rdf:type")){
              n += mstring_sprintf(insert,"\t\t?v%d a %s .\n", v, osd->object);
            }
            else{*/
            n += mstring_sprintf(insert,"\t\t?v%d %s ?v%d .\n", v, osd->predicate, v++);
            n += mstring_sprintf(insert,"\t\t?v%d a %s .\n", v, osd->object);
            snprintf(temp, sizeof(temp), "?v%d", v) ;
            list = install(osd->object,temp) ;
	           n += mstring_sprintf(where,"\tBIND (URI(CONCAT(\"http://%s:%s/%s/%s/v%d/\", ?struuid )) as ?v%d) .\n",fus_host,fus_port,table->schema->name,field->name,v,v);
	           v++ ;
           //}
          }
        }

        else if(subject && object){
          if(!strcmp(osd->object,"%value%")){
            handleValue(db, c, insert, subject, osd->predicate, pvar, n) ;
          }
          else{
            n += mstring_sprintf(insert,"\t\t%s %s %s .\n", subject, osd->predicate, object);
            n += mstring_sprintf(insert,"\t\t%s a %s .\n", object, osd->object);
          }
        }

        osd = osd->next ;
      }

      // =====================================================================================
 
      //n += mstring_sprintf(where,"\tBIND (URI(CONCAT(\"http://%s:%s/%s/%s/\", ?struuid )) as ?v%d) .\n",fus_host,fus_port,table->schema->name,field->name,v);
	//free_list() ;
      //printf(mstring_buf(where));
    }
  }
  if (mstring_buf(insert)&&mstring_buf(insert)[0]&&mstring_buf(where)&&mstring_buf(where)[0])
  {
      //loginfo("GRAPH: <http://%s:%s/%s>\n", fus_host,fus_port,db->name);
      n += mstring_sprintf(mstr,"%s\nINSERT\n\n\t{\n%s\t}\n\nWHERE\n{\n\tBIND (STRUUID() as ?struuid) .\n%s} ",
          HEADER_INSERT,mstring_buf(insert),mstring_buf(where));
	//printf(mstring_buf(mstr));
  //    n += mstring_sprintf(mstr,"%s\nINSERT\n{\n\tGRAPH <http://%s:%s/%s>\n\t{\n%s\t}\n}\nWHERE\n{\n\tBIND (STRUUID() as ?struuid) .\n%s} ",
  //          HEADER_INSERT,fus_host,fus_port,db->name,mstring_buf(insert),mstring_buf(where));
      loginfo("====================================================\n%s\n====================================================\n", mstring_buf(mstr));
      fflush(stdout);
      fflush(stderr);
  }
  if (n != 0) {
    /* mstring_* return -1 on error, with no error, the sum in n should be 0 */
    goto fail_exit;
  }
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
  //TODO table->schema->fields->concepts
  SemDB* semdb = (SemDB*)db->handle;
  SemTable* semtable = (SemTable*)table->handle;
  printf("trying to insert....") ;
  if (semtable && semtable->insert_stmt) {
    printf("inserting....") ;
    int i, res = 0;
    long http_code;
    char* tok = NULL, *old_tok = NULL,*pvar, tok2 = NULL;
    char *json = NULL;
    ssize_t json_sz;
    char* stmt = xstrdup(mstring_buf(semtable->insert_stmt));
    MString *stmtend = mstring_create ();
    double time_stamp_server;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_stamp_server = tv.tv_sec - db->start_time + 0.000001 * tv.tv_usec;

    if (tv.tv_sec > semdb->last_commit) {
      //loginfo("%s\n",table->schema->name);
      if (dba_reopen_transaction (db) == -1) {
        return -1;
      }
      semdb->last_commit = tv.tv_sec;
    }

    if (stmtend == NULL) {
      logerror("%d: Failed to create managed string for preparing SQL INSERT statement\n",
          db->backend_name);
      return -1;
    }
    struct schema *schema = table->schema;
    if (schema->nfields != value_count) {
      logerror ("fuseki:%s: Failed to insert %d values into table '%s' with %d columns\n",
          db->name, value_count, table->schema->name, schema->nfields);
      oml_free(stmt);
      mstring_delete(stmtend);
      return -1;
    }
    res += mstring_sprintf(stmtend, "update=");
    old_tok = stmt;
    // Asign sequence index to the Measurement
    /*if ((tok = strstr(old_tok, REPLACE_URI))) {
        *tok = '\0';
        res += mstring_sprintf(stmtend, "%s\"http://resourceURI/\"", old_tok);
	old_tok = tok+strlen(REPLACE_URI);
    }*/
    if ((tok = strstr(old_tok, REPLACE_SEQ))) {
        *tok = '\0';
        res += mstring_sprintf(stmtend, "%s\"%d\"^^xsd:integer", old_tok, seq_no);
        old_tok = tok+strlen(REPLACE_SEQ);
    }
    if ((tok = strstr(old_tok, REPLACE_TC))) {
        *tok = '\0';
        res += mstring_sprintf(stmtend, "%s\"%f\"^^xsd:double", old_tok, time_stamp);
        old_tok = tok+strlen(REPLACE_TC);
    }
    if ((tok = strstr(old_tok, REPLACE_TS))) {
        *tok = '\0';
        res += mstring_sprintf(stmtend, "%s\"%f\"^^xsd:double", old_tok, time_stamp_server);
        old_tok = tok+strlen(REPLACE_TS);
    }
    if ((tok = strstr(old_tok, REPLACE_SID))) {
        *tok = '\0';
        res += mstring_sprintf(stmtend, "%s\"%s\"^^xsd:string", old_tok, sid);
        old_tok = tok+strlen(REPLACE_SID);
    }
    if ((tok = strstr(old_tok, REPLACE_STARTTIME))) {
        starttime_exist(db, table) ;
        *tok = '\0';
        res += mstring_sprintf(stmtend, "%s\"%d\"^^xsd:integer", old_tok, starttime);
        old_tok = tok+strlen(REPLACE_STARTTIME);
    }
    pvar = sem_prepared_var(db,0);
    for (i = 0; i < schema->nfields;i++) {
      if ((tok = strstr(old_tok, pvar))) {
        if (oml_value_get_type(v+i) != schema->fields[i].type) {
          const char *expected = oml_type_to_s (schema->fields[i].type);
          const char *received = oml_type_to_s (oml_value_get_type(v));
          logerror("fuseki:%s: Value %d type mismatch for table '%s'\n", db->name, i, table->schema->name);
          logdebug("fuseki:%s: -> Column name='%s', type=%s, but trying to insert a %s\n",
              db->name, schema->fields[i].name, expected, received);
          oml_free(pvar);
          oml_free(stmt);
          mstring_delete(stmtend);
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
            logwarn("fuseki:%s: Trying to store value %" PRIu64 " (>2^63) in column '%s' of table '%s', this might lead to a loss of resolution\n",
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
	
	case OML_DATETIME_VALUE:
          res += mstring_sprintf(stmtend, "%s\"%s\"^^%s", old_tok, omlc_get_string_ptr(*oml_value_get_value(v+i)), sem_oml_to_type (schema->fields[i].type));
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
          logerror("fuseki:%s: Unknown type %d in col '%s' of table '%s; this is probably a bug'\n",
              db->name, schema->fields[i].type, schema->fields[i].name, table->schema->name);
          oml_free(stmt);
          mstring_delete(stmtend);
          return -1;
        }
        if (res != 0) {
          logerror("fuseki:%s: Could not bind column '%s'\n",
              db->name, schema->fields[i].name);
          oml_free(stmt);
          mstring_delete(stmtend);
          return -1;
        }
        old_tok = tok+strlen(pvar); // go to next segment
      }
      /*if ((tok = strstr(old_tok, REPLACE_URI_VM))) {
        if(!strcmp(schema->fields[i].name,"VMname")){
                *tok = '\0';
                res += mstring_sprintf(stmtend, "%s\"%s%s\"", old_tok, "http://", omlc_get_string_ptr(*oml_value_get_value(v+i)));
                old_tok = tok+strlen(REPLACE_URI_VM);
        }
      }*/
      oml_free(pvar);
      pvar = sem_prepared_var(db,i+1);
    } //end for
    
      if ((tok = strstr(old_tok, REPLACE_PHYRESOURCE))) {
        for (i = 0; i < schema->nfields;i++) {
          if(!strcmp(schema->fields[i].name,"physicalresource")){
            *tok = '\0';
            if (strstr(omlc_get_string_ptr(*oml_value_get_value(v+i)), "://") != NULL) {
              res += mstring_sprintf(stmtend, "%s\"%s\"", old_tok, omlc_get_string_ptr(*oml_value_get_value(v+i)));   
            }else {
              res += mstring_sprintf(stmtend, "%s\"%s%s\"", old_tok, "http://", omlc_get_string_ptr(*oml_value_get_value(v+i)));
            }
            old_tok = tok+strlen(REPLACE_PHYRESOURCE);
          } 
        }
      }
      if ((tok = strstr(old_tok, REPLACE_VIRRESOURCE))) {
        for (i = 0; i < schema->nfields;i++) {
          if(!strcmp(schema->fields[i].name,"virtualresource")){
            *tok = '\0';
            if (strstr(omlc_get_string_ptr(*oml_value_get_value(v+i)), "://") != NULL) {
              res += mstring_sprintf(stmtend, "%s\"%s\"", old_tok, omlc_get_string_ptr(*oml_value_get_value(v+i)));          
             }else {
              res += mstring_sprintf(stmtend, "%s\"%s%s\"", old_tok, "http://", omlc_get_string_ptr(*oml_value_get_value(v+i)));
            }
            old_tok = tok+strlen(REPLACE_VIRRESOURCE);
          } 
        }
      }

    oml_free(pvar);
    res += mstring_sprintf(stmtend, "%s", old_tok);
    printf("STMTEND\n%s\n",mstring_buf(stmtend)) ;
    if (old_tok != stmt)
    {
        MString* url = mstring_create ();
        CURL *curl;
        curl_global_init(CURL_GLOBAL_ALL);
        if (!(curl = curl_easy_init())) return -1;
        mstring_sprintf(url,"http://%s:%s/%s/update",fus_host, fus_port, fus_namespace);
        loginfo("%s\n",mstring_buf(url));
        curl_easy_setopt(curl, CURLOPT_URL, mstring_buf(url));
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &logwrite);
        semdb->conn = curl ;

        curl_easy_setopt(semdb->conn, CURLOPT_POSTFIELDS, mstring_buf(stmtend));
        res = curl_easy_perform(semdb->conn);
        /* Check for errors */ 
        if (res != CURLE_OK)
          logerror("fuseki:%s: Semantic insertion failed: %s\n", db->name, curl_easy_strerror(res));
        curl_easy_getinfo (semdb->conn, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code != 200L)
          logerror("fuseki:%s: Semantic insertion failed. HTTP code status: %ld\n", db->name, http_code);
    }
    oml_free(stmt);

    mstring_delete(stmtend);
  }
  return 0;
}

/** Add a new sender to the database, returning its index.
 * \see db_add_sender_id
 */
static char*
sem_add_sender_id(Database* database, const char* sender_id)
{
  sid = strdup(sender_id) ;
  return 0;
}

/** Set data in the metadata table
 * \see db_adapter_set_metadata, sq3_set_key_value
 */
static int
sem_set_metadata (Database* database, const char* key, const char* value)
{
  if(strcmp(key, "start-time") == 0 || strcmp(key, "start_time") == 0){
	starttime = atoi(value) ;	
  printf("starttime parsed: %d", starttime) ;
  }
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

static int
update_starttime(Database *db, DbTable *table)
{
  CURL *curl; 
  SemDB* semdb = (SemDB*)db->handle;
  SemTable* semtable = (SemTable*)table->handle;
  MString *query = mstring_create ();
  MString *url = mstring_create ();
  int res = 0;
  long http_code;

  struct string s;
  init_string(&s);

  mstring_sprintf(url,"http://%s:%s/%s/update",fus_host, fus_port, fus_namespace);
  mstring_sprintf(query,"update=%s DELETE {?monservice omn-lifecycle:StartTime ?st} INSERT {?monservice omn-lifecycle:StartTime \"%d\"} WHERE { ?monservice omn-lifecycle:StartTime ?st .\n?monservice omn:isServiceOf ?sender .\n?sender rdfs:label ?label_send .\n?domain rdfs:label ?label_dom .\n?domain omn:hasService ?monservice .\nfilter(str(?label_dom) = \"%s\") .\nfilter(str(?label_send) = \"%s\")}", HEADER_INSERT, starttime, db->name, sid) ;

  if ((curl = curl_easy_init()) == NULL) return -1 ;
  curl_easy_setopt(curl, CURLOPT_URL, mstring_buf(url));
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &logwrite);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, mstring_buf(query));
  res = curl_easy_perform(curl);
  /* cleanup curl handle */
  curl_easy_cleanup(curl);

  if(res == CURLE_OK || http_code == 200L){
      free(s.ptr);
    return 1 ;
  } 
  /* Check for errors */ 
  else{
    logerror("fuseki:%s: starttime Semantic update failed: %s\n", db->name, curl_easy_strerror(res));
    return 0 ;
  }
  curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
  if (http_code != 200L){
    logerror("fuseki:%s: starttime Semantic update failed. HTTP code status: %ld\n", db->name, http_code);
    return 0 ;
  }
  return 0 ;
}

static int
starttime_exist(Database *db, DbTable *table)
{
  CURL *curl; 
  SemDB* semdb = (SemDB*)db->handle;
  SemTable* semtable = (SemTable*)table->handle;
  MString *query = mstring_create ();
  MString *url = mstring_create ();
  int res = 0;
  long http_code;

  struct string s;
  init_string(&s);

  mstring_sprintf(url,"http://%s:%s/%s/query",fus_host, fus_port, fus_namespace);
  mstring_sprintf(query,"query=%s SELECT ?st {{?domain rdfs:label ?label_dom .\n?domain omn:hasService ?monservice .\n?monservice omn:isServiceOf ?sender . \n?sender rdfs:label ?label_send .\n?monservice omn-lifecycle:StartTime ?st .\nfilter(str(?label_dom) = \"%s\") .\nfilter(str(?label_send) = \"%s\")}} limit 1",HEADER_INSERT, db->name, sid);

  if ((curl = curl_easy_init()) == NULL) return -1 ;
  curl_easy_setopt(curl, CURLOPT_URL, mstring_buf(url));
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, mstring_buf(query));
  res = curl_easy_perform(curl);

  /* cleanup curl handle */
  curl_easy_cleanup(curl);

  if(res == CURLE_OK || http_code == 200L){
    //parse
    cJSON *root = cJSON_Parse(s.ptr);
    char* value = parse_object(root, "st");
    if (value != NULL){
      starttime = atoi(value) ;
      cJSON_Delete(root);
      free(s.ptr);
    return 1 ;
    }
    else return 0 ;
  } 
  /* Check for errors */ 
  else{
    logerror("fuseki:%s: starttime Semantic query failed: %s\n", db->name, curl_easy_strerror(res));
    return 0 ;
  }
  curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
  if (http_code != 200L){
    logerror("fuseki:%s: starttime Semantic query failed. HTTP code status: %ld\n", db->name, http_code);
    return 0 ;
  }

  return 0 ;
}

static int
sender_exist(Database *db, DbTable *table)
{
  CURL *curl; 
  SemDB* semdb = (SemDB*)db->handle;
  SemTable* semtable = (SemTable*)table->handle;
  MString *query = mstring_create ();
  MString *url = mstring_create ();
  int res = 0;
  long http_code;

  struct string s;
  init_string(&s);

  mstring_sprintf(url,"http://%s:%s/%s/query",fus_host, fus_port, fus_namespace);
  mstring_sprintf(query,"query=%s SELECT ?sender {{?sender rdfs:label ?label .\n?sender rdf:type omn:Infrastructure .\nfilter(str(?label) = \"%s\")}} ",HEADER_INSERT, sid);

  if ((curl = curl_easy_init()) == NULL) return -1 ;
  curl_easy_setopt(curl, CURLOPT_URL, mstring_buf(url));
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, mstring_buf(query));
  res = curl_easy_perform(curl);

  /* cleanup curl handle */
  curl_easy_cleanup(curl);

  if(res == CURLE_OK || http_code == 200L){
    //parse
    cJSON *root = cJSON_Parse(s.ptr);
    char* value = parse_object(root, "sender");
    if (value != NULL){
      sender_uri = mstring_create () ;
      mstring_sprintf(sender_uri, "%s", value) ;
      cJSON_Delete(root);
      free(s.ptr);
    return 1 ;
    }
    else return 0 ;
  } 
  /* Check for errors */ 
  else{
    logerror("fuseki:%s: sender-id Semantic query failed: %s\n", db->name, curl_easy_strerror(res));
    return 0 ;
  }
  curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
  if (http_code != 200L){
    logerror("fuseki:%s: sender-id Semantic query failed. HTTP code status: %ld\n", db->name, http_code);
    return 0 ;
  }

  return 0 ;
}

static int
domain_exist(Database *db, DbTable *table)
{
  CURL *curl; 
  SemDB* semdb = (SemDB*)db->handle;
  SemTable* semtable = (SemTable*)table->handle;
  MString *query = mstring_create ();
  MString *url = mstring_create ();
  int res = 0;
  long http_code;

  struct string s;
  init_string(&s);
  mstring_sprintf(url,"http://%s:%s/%s/query",fus_host, fus_port, fus_namespace);
  mstring_sprintf(query,"query=%s SELECT ?domain {{?domain rdfs:label ?label .\n?domain rdf:type omn-monitoring-genericconcepts:MonitoringDomain .\nfilter(str(?label) = \"%s\")}} limit 1 ",HEADER_INSERT, db->name);
  if ((curl = curl_easy_init()) == NULL) return -1 ;
  curl_easy_setopt(curl, CURLOPT_URL, mstring_buf(url));
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, mstring_buf(query));
  res = curl_easy_perform(curl);
  /* cleanup curl handle */
  curl_easy_cleanup(curl);
  if(res == CURLE_OK || http_code == 200L){
    //parse
    cJSON *root = cJSON_Parse(s.ptr);
    char* value = parse_object(root, "domain");
    if (value != NULL){
      domain_uri = mstring_create () ;
      mstring_sprintf(domain_uri, "%s", value) ;
      domain_uri_exist = 1 ;
      cJSON_Delete(root);
      free(s.ptr);
    return 1 ;
    }
    else return 0 ;
  } 
  /* Check for errors */ 
  else{
    logerror("fuseki:%s: domain Semantic query failed: %s\n", db->name, curl_easy_strerror(res));
    return 0 ;
  }
  curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
  if (http_code != 200L){
    logerror("fuseki:%s: domain Semantic query failed. HTTP code status: %ld\n", db->name, http_code);
    return 0 ;
  }

  return 0 ;
}

static int
service_exist(Database *db, DbTable *table)
{
  CURL *curl; 
  SemDB* semdb = (SemDB*)db->handle;
  SemTable* semtable = (SemTable*)table->handle;
  MString *query = mstring_create ();
  MString *url = mstring_create ();
  int res = 0;
  long http_code;

  struct string s;
  init_string(&s);

  mstring_sprintf(url,"http://%s:%s/%s/query",fus_host, fus_port, fus_namespace);
  mstring_sprintf(query,"query=%s SELECT ?monservice {{?domain rdfs:label ?label_dom .\n?domain omn:hasService ?monservice .\n?monservice omn:isServiceOf ?sender . \n?sender rdfs:label ?label_send .\nfilter(str(?label_dom) = \"%s\") .\nfilter(str(?label_send) = \"%s\")}} ",HEADER_INSERT, db->name, sid);

  if ((curl = curl_easy_init()) == NULL) return -1 ;
  curl_easy_setopt(curl, CURLOPT_URL, mstring_buf(url));
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
  semdb->conn = curl ;
  curl_easy_setopt(semdb->conn, CURLOPT_POSTFIELDS, mstring_buf(query));
  res = curl_easy_perform(semdb->conn);

  /* cleanup curl handle */
  curl_easy_cleanup(curl);

  if(res == CURLE_OK || http_code == 200L){
    //parse
    cJSON *root = cJSON_Parse(s.ptr);
    char* value = parse_object(root,"monservice");
    if (value != NULL){
      service_uri = mstring_create () ;
      mstring_sprintf(service_uri, "%s", value) ;
      service_uri_exist = 1 ;
      cJSON_Delete(root);
      free(s.ptr);
    return 1 ;
    }
    else return 0 ;
  } 
  /* Check for errors */ 
  else{
    logerror("fuseki:%s: service Semantic query failed: %s\n", db->name, curl_easy_strerror(res));
    return 0 ;
  }
  curl_easy_getinfo (semdb->conn, CURLINFO_RESPONSE_CODE, &http_code);
  if (http_code != 200L){
    logerror("fuseki:%s: service Semantic query failed. HTTP code status: %ld\n", db->name, http_code);
    return 0 ;
  }

  return 0 ;
}

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

char* parse_object(cJSON *root, char* res_name)
{
  //cJSON* results = NULL;
  //cJSON* bindings = NULL;
  cJSON* st = NULL ;
  char* value = NULL;
  int i;

  cJSON *results = cJSON_CreateObject();
  results = cJSON_GetObjectItem(root,"results");
  cJSON *bindings = cJSON_CreateArray();
  bindings = cJSON_GetObjectItem(results,"bindings");

  if(cJSON_GetArraySize(bindings) != 0){
    for (i = 0 ; i < cJSON_GetArraySize(bindings) ; i++)
    {
       cJSON * subitem = cJSON_GetArrayItem(bindings, i);
       st = cJSON_GetObjectItem(subitem, res_name) ;
       value = cJSON_GetObjectItem(st, "value")->valuestring;
    }
  }

  return value ;
}

/*
char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    count += last_comma < (a_str + strlen(a_str) - 1);

    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}
*/





