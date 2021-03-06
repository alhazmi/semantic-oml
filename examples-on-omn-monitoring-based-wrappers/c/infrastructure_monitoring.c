/*
 * This file was automatically generated by oml2-scaffold V2.12.0pre.79-58cf-dirty
 * for infrastructure_monitoring version 1.0.0.
 * Please edit to suit your needs; the run() function should contain application code.
 */

#include <unistd.h> /* Needed for sleep(3) in run() */
#include <signal.h>
#include <string.h>
#include <popt.h>
#include <oml2/omlc.h>

#define USE_OPTS /* Include command line parsing code*/
#include "infrastructure_monitoring_popt.h"

#define OML_FROM_MAIN /* Define storage for some global variables; #define this in only one file */
#include "infrastructure_monitoring_oml.h"

#include "config.h"

int loop = 1;

static void
sighandler (int signum) {
  switch (signum) {
    case SIGINT:
      /* Terminate on SIGINT */
      loop = 0;
      break;

  }
}

/* Do application-specific work here.
 */
void
run(opts_t *opts, oml_mps_t *oml_mps)
{
  long val = 1;
  struct sigaction sa;

  bzero(&sa, sizeof(struct sigaction));
  sa.sa_handler = sighandler;
  sigaction(SIGINT, &sa, NULL);


  /* The oml_inject_MPNAME() helpers are defined in infrastructure_monitoring_oml.h*/
  sleep(1);
  if(opts->vm && opts->pm && opts->used_memory && opts->used_memory_timestamp){
	if(oml_inject_used_memory(oml_mps->used_memory, opts->used_memory, opts->used_memory_timestamp, opts->pm, opts->vm) != 0) {
      		logwarn("Failed to inject data into MP 'used_memory'\n");
    	}
  }

 sleep(1);
 if(opts->vm && opts->pm && opts->total_memory && opts->total_memory_timestamp){
  if(oml_inject_total_memory(oml_mps->total_memory, opts->total_memory, opts->total_memory_timestamp, opts->pm, opts->vm) != 0) {
      logwarn("Failed to inject data into MP 'total_memory'\n");
    }
 } 

sleep(1);
if(opts->vm && opts->pm && opts->available_memory && opts->available_memory_timestamp){
  if(oml_inject_available_memory(oml_mps->available_memory, opts->available_memory, opts->available_memory_timestamp, opts->pm, opts->vm) != 0) {
      logwarn("Failed to inject data into MP 'available_memory'\n");
    }
 }

sleep(1);
if(opts->vm && opts->pm && opts->used_bandwidth && opts->used_bandwidth_timestamp){
  if(oml_inject_used_bandwidth(oml_mps->used_bandwidth, opts->used_bandwidth, opts->used_bandwidth_timestamp, opts->pm, opts->vm) != 0) {
      logwarn("Failed to inject data into MP 'used_bandwidth'\n");
    }
 } 

sleep(1);
if(opts->vm && opts->pm && opts->availability && opts->availability_timestamp){
  if(oml_inject_availability(oml_mps->availability, opts->availability, opts->availability_timestamp, opts->pm, opts->vm) != 0) {
      logwarn("Failed to inject data into MP 'availability'\n");
    }
 } 

sleep(1);

}

int
main(int argc, const char *argv[])
{
  int c, i, ret;

  /* Reconstruct command line */
  size_t cmdline_len = 0;
  for(i = 0; i < argc; i++) {
    cmdline_len += strlen(argv[i]) + 1;
  }
  char cmdline[cmdline_len + 1];
  cmdline[0] = '\0';
  for(i = 0; i < argc; i++) {
    strncat(cmdline, argv[i], cmdline_len);
    cmdline_len -= strlen(argv[i]);
    strncat(cmdline, " ", cmdline_len);
    cmdline_len--;
  }

  /* Initialize OML */
  if((ret = omlc_init("infrastructure_monitoring", &argc, argv, NULL)) < 0) {
    logerror("Could not initialise OML\n");
    return -1;
  }

  /* Parse command line arguments */
  poptContext optCon = poptGetContext(NULL, argc, argv, options, 0); /* options is defined in infrastructure_monitoring_popt.h */
  while ((c = poptGetNextOpt(optCon)) > 0) {}

  /* Initialise measurement points and start OML */
  oml_register_mps(); /* Defined in infrastructure_monitoring_oml.h */
  if(omlc_start()) {
    logerror("Could not start OML\n");
    return -1;
  }

  /* Inject some metadata about this application */
  OmlValueU v;
  omlc_zero(v);
  omlc_set_string(v, PACKAGE_NAME);
  omlc_inject_metadata(NULL, "appname", &v, OML_STRING_VALUE, NULL);

  omlc_set_string(v, PACKAGE_VERSION);
  omlc_inject_metadata(NULL, "version", &v, OML_STRING_VALUE, NULL);

  omlc_set_string(v, cmdline);
  omlc_inject_metadata(NULL, "cmdline", &v, OML_STRING_VALUE, NULL);
  omlc_reset_string(v);

  /* Inject measurements */
  run(g_opts, g_oml_mps_infrastructure_monitoring); /* Do some work and injections, see above */

  omlc_close();

  return 0;
}

/*
 Local Variables:
 mode: C
 tab-width: 2
 indent-tabs-mode: nil
 End:
 vim: sw=2:sts=2:expandtab
*/
