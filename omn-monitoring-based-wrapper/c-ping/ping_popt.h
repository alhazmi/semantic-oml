/*
 * This file was automatically generated by oml2-scaffold V2.12.0pre.79-58cf-dirty
 * for ping version 1.0.0.
 * Please do not edit.
 */

#ifndef PING_OPTS_H
#define PING_OPTS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

} opts_t;

#ifndef USE_OPTS

opts_t* g_opts;

#else

static opts_t g_opts_storage = {};
opts_t* g_opts = &g_opts_storage;

/* Only the file containing the main() function should come through here */

#include <popt.h>

struct poptOption options[] = {
  POPT_AUTOHELP

  { NULL, 0, 0, NULL, 0 }
};

#endif /* USE_OPTS */

#ifdef __cplusplus
}
#endif

#endif /* PING_OPTS_H */