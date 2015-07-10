/*
 * This file was automatically generated by oml2-scaffold V2.12.0pre.79-58cf-dirty
 * for oml2-server version 2.12.0.
 * Please do not edit.
 */

#ifndef OML2_SERVER_OML_H
#define OML2_SERVER_OML_H

#ifdef __cplusplus
extern "C" {
#endif

/* Define HUGE, etc.. */
#define _GNU_SOURCE
#include <math.h>
/* Get size_t and NULL from <stddef.h> */
#include <stddef.h>

#include <oml2/omlc.h>

typedef struct {
  OmlMP *clients;

} oml_mps_t;


#ifdef OML_FROM_MAIN

extern OMLSemDef *oml_sem_register_concepts(char***concepts, int n);

/*
 * Only declare storage once, usually from the main
 * source, where OML_FROM_MAIN is defined
 */

static OmlMPDef oml_clients_def[] = {
  {"address", OML_STRING_VALUE, NULL},
  {"port", OML_INT32_VALUE, NULL},
  {"node_id", OML_STRING_VALUE, NULL},
  {"domain", OML_STRING_VALUE, NULL},
  {"appname", OML_STRING_VALUE, NULL},
  {"event", OML_STRING_VALUE, NULL},
  {"message", OML_STRING_VALUE, NULL},
  {NULL, (OmlValueT)0, NULL}
};

static oml_mps_t g_oml_mps_storage;
oml_mps_t* g_oml_mps_oml2_server = &g_oml_mps_storage;

static inline void
oml_register_mps(void)
{
  int i,j;
  char*** concept;// = (char***)oml_malloc(sizeof(char**)*);
  g_oml_mps_oml2_server->clients = omlc_add_mp("clients", oml_clients_def);

}

#else
/*
 * Not included from the main source, only declare the global pointer
 * to the MPs and injection helpers.
 */

extern oml_mps_t* g_oml_mps_oml2_server;

#endif /* OML_FROM_MAIN */

static inline int
oml_inject_clients(OmlMP *mp, const char *address, int32_t port, const char *node_id, const char *domain, const char *appname, const char *event, const char *message)
{
  int ret = -1;

  OmlValueU v[7];
  omlc_zero_array(v, 7);

  omlc_set_string(v[0], address);
  omlc_set_int32(v[1], port);
  omlc_set_string(v[2], node_id);
  omlc_set_string(v[3], domain);
  omlc_set_string(v[4], appname);
  omlc_set_string(v[5], event);
  omlc_set_string(v[6], message);

  ret = omlc_inject(mp, v);

  omlc_reset_string(v[0]);
  omlc_reset_string(v[2]);
  omlc_reset_string(v[3]);
  omlc_reset_string(v[4]);
  omlc_reset_string(v[5]);
  omlc_reset_string(v[6]);
  return ret;
}


/* Compatibility with old applications */
#ifndef g_oml_mps
# define g_oml_mps	g_oml_mps_oml2_server
#endif

#ifdef __cplusplus
}
#endif

#endif /* OML2_SERVER_OML_H */