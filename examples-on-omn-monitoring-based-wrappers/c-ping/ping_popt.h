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
  char* link;
  double packet_loss;
  char* packet_loss_timestamp;
  double delay;
  char* delay_timestamp;

} opts_t;

#ifndef USE_OPTS

opts_t* g_opts;

#else

static opts_t g_opts_storage = {"", 0., "", 0., ""};
opts_t* g_opts = &g_opts_storage;

/* Only the file containing the main() function should come through here */

#include <popt.h>

struct poptOption options[] = {
  POPT_AUTOHELP
  { "link-uri", 0, POPT_ARG_STRING, &g_opts_storage.link, 0, "URI of a Link between two nodes"},
  { "packet-loss", 0, POPT_ARG_DOUBLE, &g_opts_storage.packet_loss, 0, "Check Packet Loss"},
  { "packet-loss-timestamp", 0, POPT_ARG_STRING, &g_opts_storage.packet_loss_timestamp, 0, "Time of checking"},
  { "delay", 0, POPT_ARG_DOUBLE, &g_opts_storage.delay, 0, "Check Delay"},
  { "delay-timestamp", 0, POPT_ARG_STRING, &g_opts_storage.delay_timestamp, 0, "Time of checking"},

  { NULL, 0, 0, NULL, 0 }
};

#endif /* USE_OPTS */

#ifdef __cplusplus
}
#endif

#endif /* PING_OPTS_H */