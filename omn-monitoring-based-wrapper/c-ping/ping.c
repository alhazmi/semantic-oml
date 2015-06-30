/* 
 * Copyright (c) 2015, Andisa Dewi, Yahya Al-Hazmi, Technische Universitaet Berlin
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License
 *
 * This is the main source code for the generation of the monitoring application
 */

/*
 * This file was automatically generated by oml2-scaffold V2.12.0pre.79-58cf-dirty
 * for ping version 1.0.0.
 * Please edit to suit your needs; the run() function should contain application code.
 */

#include <unistd.h> /* Needed for sleep(3) in run() */
#include <signal.h>
#include <string.h>
#include <popt.h>
#include <oml2/omlc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define USE_OPTS /* Include command line parsing code*/
#include "ping_popt.h"

#define OML_FROM_MAIN /* Define storage for some global variables; #define this in only one file */
#include "ping_oml.h"

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

    FILE *cmd = popen ("ping -c 5 -q 185.63.147.10 | grep -oP \'\\d+(?=% packet loss)\'", "r" ) ;
    char *packetloss = malloc ( sizeof ( char ) * 200 );
    fgets ( packetloss, sizeof ( char )*200, cmd );
    printf ( "%s", packetloss); //show outcome
    pclose ( cmd );
    FILE *cmd2 = popen("ping -c 5 -q 185.63.147.10 | grep rtt | cut -d\"/\" -f5", "r") ;
    char *avgdelay = malloc ( sizeof ( char ) * 200 );
    fgets ( avgdelay, sizeof ( char )*200, cmd2 );
    printf ( "%s", avgdelay);
    pclose ( cmd2 );

    FILE *cmd3 = popen ("ping -c 5 -q 185.63.147.10 | grep \"rtt\" -A 1", "r" ) ;
    char *output = malloc ( sizeof ( char ) * 400 );
    fgets ( output, sizeof ( char )*400, cmd3 );
    printf ( "%s", output);
    pclose ( cmd3 );

    time_t     now;
    struct tm  ts;
    char       buf[80];
    // Get current time
    time(&now);
    // Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
    ts = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ts);
    printf("%s\n", buf);

  long val = 1;
  struct sigaction sa;

  bzero(&sa, sizeof(struct sigaction));
  sa.sa_handler = sighandler;
  sigaction(SIGINT, &sa, NULL);

  /* The oml_inject_MPNAME() helpers are defined in ping_oml.h*/
  sleep(1);
  if(oml_inject_packet_loss(oml_mps->packet_loss, atoi(packetloss), buf, "130.149.22.139" ,"185.63.147.10") != 0) {
      logwarn("Failed to inject data into MP 'packet_loss'\n");
    }
 sleep(1);
  if(oml_inject_delay(oml_mps->delay, atof(avgdelay), buf, "130.149.22.139", "185.63.147.10") != 0) {
      logwarn("Failed to inject data into MP 'delay'\n");
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
  if((ret = omlc_init("ping", &argc, argv, NULL)) < 0) {
    logerror("Could not initialise OML\n");
    return -1;
  }

  /* Parse command line arguments */
  poptContext optCon = poptGetContext(NULL, argc, argv, options, 0); /* options is defined in ping_popt.h */
  while ((c = poptGetNextOpt(optCon)) > 0) {}

  /* Initialise measurement points and start OML */
  oml_register_mps(); /* Defined in ping_oml.h */
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
  run(g_opts, g_oml_mps_ping); /* Do some work and injections, see above */

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
