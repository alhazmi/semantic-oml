/* 
 * Copyright 2014 UAM - Mario Poyato Pino
 * File:   virtuoso_adapter.h
 * Author: Mario Poyato Pino
 *
 *
 * This software may be used and distributed solely under the terms of
 * the MIT license (License).  You should find a copy of the License in
 * COPYING or at http://opensource.org/licenses/MIT. By downloading or
 * using this software you accept the terms and the liability disclaimer
 * in the License.
 */
#ifndef VIRTUOSO_ADAPTER_H_
#define VIRTUOSO_ADAPTER_H_

#include "../lib/ocomm/ocomm/o_socket.h"
#include "database.h"
#include <oml2/oml_out_stream.h>
#include <curl/curl.h>

#define DEFAULT_VIR_HOST "localhost"
#define DEFAULT_VIR_PORT "8890"
#define DEFAULT_VIR_USER "dba"
#define DEFAULT_VIR_PROTOCOL "tcp"
#define DEFAULT_VIR_PASS "dba"
#define DEFAULT_VIR_CONNINFO ""

typedef struct VirDB {
  CURL*     conn;
  int       sender_cnt;
  time_t    last_commit;
} VirDB;

typedef struct VirTable {
  MString* insert_stmt;  // prepared insert statement
} VirTable;

int virtuoso_backend_setup (void);
int virtuoso_create_database (Database* db);

#endif /*SQLITE_ADAPTER_H_*/

/*
 Local Variables:
 mode: C
 tab-width: 2
 indent-tabs-mode: nil
 End:
 vim: sw=2:sts=2:expandtab
*/
