/*
 * Copyright 2013-2014 National ICT Australia (NICTA), Australia
 *
 *
 * This software may be used and distributed solely under the terms of
 * the MIT license (License).  You should find a copy of the License in
 * COPYING or at http://opensource.org/licenses/MIT. By downloading or
 * using this software you accept the terms and the liability disclaimer
 * in the License.
 */

#ifndef MONITORING_SERVER_H_
#define MONITORING_SERVER_H_

void oml_setup(int *argc, const char **argv);

void oml_cleanup(void);

void client_event_inject(const char* address, uint32_t port, const char* oml_id, const char* domain, const char* appname, const char* event, const char* message);

#endif /*MONITORING_SERVER_H_*/

/*
 Local Variables:
 mode: C
 tab-width: 2
 indent-tabs-mode: nil
 End:
 vim: sw=2:sts=2:expandtab
*/
