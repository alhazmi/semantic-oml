/* 
 * Copyright 2014 UAM - Mario Poyato Pino
 * File:   fuseki_adapter.h
 * Author: Mario Poyato Pino <mario.poyato@estudiante.uam.es>
 *
 * Modified by Andisa Dewi, Yahya Al-Hazmi, Technische Universitaet Berlin, 2015
 *  to support OMN Ontology
 *
 * This software may be used and distributed solely under the terms of
 * the MIT license (License).  You should find a copy of the License in
 * COPYING or at http://opensource.org/licenses/MIT. By downloading or
 * using this software you accept the terms and the liability disclaimer
 * in the License.
 */
#ifndef SEMANTIC_ADAPTER_H_
#define SEMANTIC_ADAPTER_H_

#include "../lib/ocomm/ocomm/o_socket.h"
#include "database.h"
#include <oml2/oml_out_stream.h>
#include <curl/curl.h>

#define DEFAULT_FUS_HOST "localhost"
#define DEFAULT_FUS_PORT "3030"
#define DEFAULT_FUS_NAMESPACE "ds"

typedef struct SemDB {
  CURL*     conn;
  int       sender_cnt;
  time_t    last_commit;
} SemDB;

typedef struct SemTable {
  MString* insert_stmt;  // prepared insert statement
} SemTable;

struct nlist { /* table entry: */
    struct nlist *next; /* next entry in chain */
    char *name; /* defined name */
    char *defn; /* replacement text */
};

int fuseki_backend_setup (void);
int fuseki_create_database (Database* db);

#endif /*SQLITE_ADAPTER_H_*/

/*
 Local Variables:
 mode: C
 tab-width: 2
 indent-tabs-mode: nil
 End:
 vim: sw=2:sts=2:expandtab
*/
