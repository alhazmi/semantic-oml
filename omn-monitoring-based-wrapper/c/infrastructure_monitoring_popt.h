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
 * This is the popt header for the generation of the monitoring application
 * This file is needed if you want the application to have command line support
 */

/*
 * This file was automatically generated by oml2-scaffold V2.12.0pre.79-58cf-dirty
 * for infrastructure_monitoring version 1.0.0.
 * Please do not edit.
 */

#ifndef INFRASTRUCTURE_MONITORING_OPTS_H
#define INFRASTRUCTURE_MONITORING_OPTS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  char* pm_name;
  char* vm_name;
  int availability;
  char* availability_timestamp;
  double used_memory;
  char* used_memory_timestamp;
  double available_memory;
  char* available_memory_timestamp;
  double total_memory;
  char* total_memory_timestamp;
  double used_bandwidth;
  char* used_bandwidth_timestamp;

} opts_t;

#ifndef USE_OPTS

opts_t* g_opts;

#else

static opts_t g_opts_storage = {"", "", 0, "", 0., "", 0., "", 0., "", 0., ""};
opts_t* g_opts = &g_opts_storage;

/* Only the file containing the main() function should come through here */

#include <popt.h>

struct poptOption options[] = {
  POPT_AUTOHELP
  { "pm-name", 0, POPT_ARG_STRING, &g_opts_storage.pm_name, 0, "Name of monitored physical host"},
  { "vm-name", 0, POPT_ARG_STRING, &g_opts_storage.vm_name, 0, "Name of virtual host"},
  { "availability", 0, POPT_ARG_INT, &g_opts_storage.availability, 0, "Check if a VM is available"},
  { "availability-timestamp", 0, POPT_ARG_STRING, &g_opts_storage.availability_timestamp, 0, "Time of checking"},
  { "used-memory", 0, POPT_ARG_DOUBLE, &g_opts_storage.used_memory, 0, "Check used memory"},
  { "used-memory-timestamp", 0, POPT_ARG_STRING, &g_opts_storage.used_memory_timestamp, 0, "Time of checking"},
  { "available-memory", 0, POPT_ARG_DOUBLE, &g_opts_storage.available_memory, 0, "Check available memory"},
  { "available-memory-timestamp", 0, POPT_ARG_STRING, &g_opts_storage.available_memory_timestamp, 0, "Time of checking"},
  { "total-memory", 0, POPT_ARG_DOUBLE, &g_opts_storage.total_memory, 0, "Check total memory"},
  { "total-memory-timestamp", 0, POPT_ARG_STRING, &g_opts_storage.total_memory_timestamp, 0, "Time of checking"},
  { "used-bandwidth", 0, POPT_ARG_DOUBLE, &g_opts_storage.used_bandwidth, 0, "Check used bandwidth"},
  { "used-bandwidth-timestamp", 0, POPT_ARG_STRING, &g_opts_storage.used_bandwidth_timestamp, 0, "Time of checking"},

  { NULL, 0, 0, NULL, 0 }
};

#endif /* USE_OPTS */

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_MONITORING_OPTS_H */
