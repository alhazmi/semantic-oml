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
 * This is the header file for the generation of the monitoring application
 */

/*
 * This file was automatically generated by oml2-scaffold V2.12.0pre.79-58cf-dirty
 * for infrastructure_monitoring version 1.0.0.
 * Please do not edit.
 */

#ifndef INFRASTRUCTURE_MONITORING_OML_H
#define INFRASTRUCTURE_MONITORING_OML_H

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
  OmlMP *used_memory;
  OmlMP *total_memory;
  OmlMP *available_memory;
  OmlMP *used_bandwidth;
  OmlMP *availability;

} oml_mps_t;


#ifdef OML_FROM_MAIN

extern OMLSemDef *oml_sem_register_concepts(char***concepts, int n);

/*
 * Only declare storage once, usually from the main
 * source, where OML_FROM_MAIN is defined
 */

static OmlMPDef oml_used_memory_def[] = {
  {"used_memory", OML_DOUBLE_VALUE, NULL},
  {"timestamp", OML_DATETIME_VALUE, NULL},
  {"physicalresource", OML_STRING_VALUE, NULL},
  {"virtualresource", OML_STRING_VALUE, NULL},
  {NULL, (OmlValueT)0, NULL}
};

static OmlMPDef oml_total_memory_def[] = {
  {"total_memory", OML_DOUBLE_VALUE, NULL},
  {"timestamp", OML_DATETIME_VALUE, NULL},
  {"physicalresource", OML_STRING_VALUE, NULL},
  {"virtualresource", OML_STRING_VALUE, NULL},
  {NULL, (OmlValueT)0, NULL}
};

static OmlMPDef oml_available_memory_def[] = {
  {"available_memory", OML_DOUBLE_VALUE, NULL},
  {"timestamp", OML_DATETIME_VALUE, NULL},
  {"physicalresource", OML_STRING_VALUE, NULL},
  {"virtualresource", OML_STRING_VALUE, NULL},
  {NULL, (OmlValueT)0, NULL}
};

static OmlMPDef oml_used_bandwidth_def[] = {
  {"used_bandwidth", OML_DOUBLE_VALUE, NULL},
  {"timestamp", OML_DATETIME_VALUE, NULL},
  {"physicalresource", OML_STRING_VALUE, NULL},
  {"virtualresource", OML_STRING_VALUE, NULL},
  {NULL, (OmlValueT)0, NULL}
};

static OmlMPDef oml_availability_def[] = {
  {"availability", OML_DOUBLE_VALUE, NULL},
  {"timestamp", OML_DATETIME_VALUE, NULL},
  {"physicalresource", OML_STRING_VALUE, NULL},
  {"virtualresource", OML_STRING_VALUE, NULL},
  {NULL, (OmlValueT)0, NULL}
};

static oml_mps_t g_oml_mps_storage;
oml_mps_t* g_oml_mps_infrastructure_monitoring = &g_oml_mps_storage;

static inline void
oml_register_mps(void)
{
  int i,j;
  char*** concept;// = (char***)oml_malloc(sizeof(char**)*);
  concept = (char***)oml_malloc(sizeof(char**)*6);
  concept[0] = (char**)oml_malloc(sizeof(char*)*3);
  concept[0][0]=strdup("omn-monitoring:Measurement");
  concept[0][1]=strdup("omn-monitoring:isMeasurementOf");
  concept[0][2]=strdup("omn-monitoring-metric:UsedMemory");
  concept[1] = (char**)oml_malloc(sizeof(char*)*3);
  concept[1][0]=strdup("omn-monitoring-metric:UsedMemory");
  concept[1][1]=strdup("omn-monitoring:isMeasurementMetricOf");
  concept[1][2]=strdup("omn-domain-pc:PC");
  concept[2] = (char**)oml_malloc(sizeof(char*)*3);
  concept[2][0]=strdup("omn-monitoring-metric:UsedMemory");
  concept[2][1]=strdup("omn-monitoring-data:hasMeasurementData");
  concept[2][2]=strdup("omn-monitoring-data:MeasurementData");
  concept[3] = (char**)oml_malloc(sizeof(char*)*3);
  concept[3][0]=strdup("omn-monitoring-data:MeasurementData");
  concept[3][1]=strdup("omn-monitoring-data:hasMeasurementDataValue");
  concept[3][2]=strdup("%value%");
  concept[4] = (char**)oml_malloc(sizeof(char*)*3);
  concept[4][0]=strdup("omn-monitoring-data:MeasurementData");
  concept[4][1]=strdup("omn-monitoring:hasUnit");
  concept[4][2]=strdup("omn-monitoring-unit:Byte");
  concept[5] = (char**)oml_malloc(sizeof(char*)*3);
  concept[5][0]=strdup("omn-monitoring-unit:Byte");
  concept[5][1]=strdup("omn-monitoring-unit:hasPrefix");
  concept[5][2]=strdup("omn-monitoring-unit:giga");
  oml_used_memory_def[0].relations = oml_sem_register_concepts(concept, 6);
  for (i=0;i<6;i++)
  {
    for (j=0;j<3;j++)
      free(concept[i][j]);
    oml_free(concept[i]);
  }
  oml_free(concept);
  concept = (char***)oml_malloc(sizeof(char**)*1);
  concept[0] = (char**)oml_malloc(sizeof(char*)*3);
  concept[0][0]=strdup("omn-monitoring-data:MeasurementData");
  concept[0][1]=strdup("omn-monitoring-data:hasTimestamp");
  concept[0][2]=strdup("%value%");
  oml_used_memory_def[1].relations = oml_sem_register_concepts(concept, 1);
  for (i=0;i<1;i++)
  {
    for (j=0;j<3;j++)
      free(concept[i][j]);
    oml_free(concept[i]);
  }
  oml_free(concept);
  concept = (char***)oml_malloc(sizeof(char**)*1);
  concept[0] = (char**)oml_malloc(sizeof(char*)*3);
  concept[0][0]=strdup("omn-domain-pc:PC");
  concept[0][1]=strdup("rdfs:label");
  concept[0][2]=strdup("%value%");
  oml_used_memory_def[2].relations = oml_sem_register_concepts(concept, 1);
  for (i=0;i<1;i++)
  {
    for (j=0;j<3;j++)
      free(concept[i][j]);
    oml_free(concept[i]);
  }
  oml_free(concept);
  concept = (char***)oml_malloc(sizeof(char**)*2);
  concept[0] = (char**)oml_malloc(sizeof(char*)*3);
  concept[0][0]=strdup("omn-domain-pc:VM");
  concept[0][1]=strdup("rdfs:label");
  concept[0][2]=strdup("%value%");
  concept[1] = (char**)oml_malloc(sizeof(char*)*3);
  concept[1][0]=strdup("omn-domain-pc:VM");
  concept[1][1]=strdup("omn-lifecycle:childOf");
  concept[1][2]=strdup("omn-domain-pc:PC");
  oml_used_memory_def[3].relations = oml_sem_register_concepts(concept, 2);
  for (i=0;i<2;i++)
  {
    for (j=0;j<3;j++)
      free(concept[i][j]);
    oml_free(concept[i]);
  }
  oml_free(concept);
  g_oml_mps_infrastructure_monitoring->used_memory = omlc_add_mp("used_memory", oml_used_memory_def);
  concept = (char***)oml_malloc(sizeof(char**)*6);
  concept[0] = (char**)oml_malloc(sizeof(char*)*3);
  concept[0][0]=strdup("omn-monitoring:Measurement");
  concept[0][1]=strdup("omn-monitoring:isMeasurementOf");
  concept[0][2]=strdup("omn-monitoring-metric:TotalMemory");
  concept[1] = (char**)oml_malloc(sizeof(char*)*3);
  concept[1][0]=strdup("omn-monitoring-metric:TotalMemory");
  concept[1][1]=strdup("omn-monitoring:isMeasurementMetricOf");
  concept[1][2]=strdup("omn-domain-pc:PC");
  concept[2] = (char**)oml_malloc(sizeof(char*)*3);
  concept[2][0]=strdup("omn-monitoring-metric:TotalMemory");
  concept[2][1]=strdup("omn-monitoring-data:hasMeasurementData");
  concept[2][2]=strdup("omn-monitoring-data:MeasurementData");
  concept[3] = (char**)oml_malloc(sizeof(char*)*3);
  concept[3][0]=strdup("omn-monitoring-data:MeasurementData");
  concept[3][1]=strdup("omn-monitoring-data:hasMeasurementDataValue");
  concept[3][2]=strdup("%value%");
  concept[4] = (char**)oml_malloc(sizeof(char*)*3);
  concept[4][0]=strdup("omn-monitoring-data:MeasurementData");
  concept[4][1]=strdup("omn-monitoring:hasUnit");
  concept[4][2]=strdup("omn-monitoring-unit:Byte");
  concept[5] = (char**)oml_malloc(sizeof(char*)*3);
  concept[5][0]=strdup("omn-monitoring-unit:Byte");
  concept[5][1]=strdup("omn-monitoring-unit:hasPrefix");
  concept[5][2]=strdup("omn-monitoring-unit:giga");
  oml_total_memory_def[0].relations = oml_sem_register_concepts(concept, 6);
  for (i=0;i<6;i++)
  {
    for (j=0;j<3;j++)
      free(concept[i][j]);
    oml_free(concept[i]);
  }
  oml_free(concept);
  concept = (char***)oml_malloc(sizeof(char**)*1);
  concept[0] = (char**)oml_malloc(sizeof(char*)*3);
  concept[0][0]=strdup("omn-monitoring-data:MeasurementData");
  concept[0][1]=strdup("omn-monitoring-data:hasTimestamp");
  concept[0][2]=strdup("%value%");
  oml_total_memory_def[1].relations = oml_sem_register_concepts(concept, 1);
  for (i=0;i<1;i++)
  {
    for (j=0;j<3;j++)
      free(concept[i][j]);
    oml_free(concept[i]);
  }
  oml_free(concept);
  concept = (char***)oml_malloc(sizeof(char**)*1);
  concept[0] = (char**)oml_malloc(sizeof(char*)*3);
  concept[0][0]=strdup("omn-domain-pc:PC");
  concept[0][1]=strdup("rdfs:label");
  concept[0][2]=strdup("%value%");
  oml_total_memory_def[2].relations = oml_sem_register_concepts(concept, 1);
  for (i=0;i<1;i++)
  {
    for (j=0;j<3;j++)
      free(concept[i][j]);
    oml_free(concept[i]);
  }
  oml_free(concept);
  concept = (char***)oml_malloc(sizeof(char**)*2);
  concept[0] = (char**)oml_malloc(sizeof(char*)*3);
  concept[0][0]=strdup("omn-domain-pc:VM");
  concept[0][1]=strdup("rdfs:label");
  concept[0][2]=strdup("%value%");
  concept[1] = (char**)oml_malloc(sizeof(char*)*3);
  concept[1][0]=strdup("omn-domain-pc:VM");
  concept[1][1]=strdup("omn-lifecycle:childOf");
  concept[1][2]=strdup("omn-domain-pc:PC");
  oml_total_memory_def[3].relations = oml_sem_register_concepts(concept, 2);
  for (i=0;i<2;i++)
  {
    for (j=0;j<3;j++)
      free(concept[i][j]);
    oml_free(concept[i]);
  }
  oml_free(concept);
  g_oml_mps_infrastructure_monitoring->total_memory = omlc_add_mp("total_memory", oml_total_memory_def);
  concept = (char***)oml_malloc(sizeof(char**)*6);
  concept[0] = (char**)oml_malloc(sizeof(char*)*3);
  concept[0][0]=strdup("omn-monitoring:Measurement");
  concept[0][1]=strdup("omn-monitoring:isMeasurementOf");
  concept[0][2]=strdup("omn-monitoring-metric:AvailableMemory");
  concept[1] = (char**)oml_malloc(sizeof(char*)*3);
  concept[1][0]=strdup("omn-monitoring-metric:AvailableMemory");
  concept[1][1]=strdup("omn-monitoring:isMeasurementMetricOf");
  concept[1][2]=strdup("omn-domain-pc:PC");
  concept[2] = (char**)oml_malloc(sizeof(char*)*3);
  concept[2][0]=strdup("omn-monitoring-metric:AvailableMemory");
  concept[2][1]=strdup("omn-monitoring-data:hasMeasurementData");
  concept[2][2]=strdup("omn-monitoring-data:MeasurementData");
  concept[3] = (char**)oml_malloc(sizeof(char*)*3);
  concept[3][0]=strdup("omn-monitoring-data:MeasurementData");
  concept[3][1]=strdup("omn-monitoring-data:hasMeasurementDataValue");
  concept[3][2]=strdup("%value%");
  concept[4] = (char**)oml_malloc(sizeof(char*)*3);
  concept[4][0]=strdup("omn-monitoring-data:MeasurementData");
  concept[4][1]=strdup("omn-monitoring:hasUnit");
  concept[4][2]=strdup("omn-monitoring-unit:Byte");
  concept[5] = (char**)oml_malloc(sizeof(char*)*3);
  concept[5][0]=strdup("omn-monitoring-unit:Byte");
  concept[5][1]=strdup("omn-monitoring-unit:hasPrefix");
  concept[5][2]=strdup("omn-monitoring-unit:giga");
  oml_available_memory_def[0].relations = oml_sem_register_concepts(concept, 6);
  for (i=0;i<6;i++)
  {
    for (j=0;j<3;j++)
      free(concept[i][j]);
    oml_free(concept[i]);
  }
  oml_free(concept);
  concept = (char***)oml_malloc(sizeof(char**)*1);
  concept[0] = (char**)oml_malloc(sizeof(char*)*3);
  concept[0][0]=strdup("omn-monitoring-data:MeasurementData");
  concept[0][1]=strdup("omn-monitoring-data:hasTimestamp");
  concept[0][2]=strdup("%value%");
  oml_available_memory_def[1].relations = oml_sem_register_concepts(concept, 1);
  for (i=0;i<1;i++)
  {
    for (j=0;j<3;j++)
      free(concept[i][j]);
    oml_free(concept[i]);
  }
  oml_free(concept);
  concept = (char***)oml_malloc(sizeof(char**)*1);
  concept[0] = (char**)oml_malloc(sizeof(char*)*3);
  concept[0][0]=strdup("omn-domain-pc:PC");
  concept[0][1]=strdup("rdfs:label");
  concept[0][2]=strdup("%value%");
  oml_available_memory_def[2].relations = oml_sem_register_concepts(concept, 1);
  for (i=0;i<1;i++)
  {
    for (j=0;j<3;j++)
      free(concept[i][j]);
    oml_free(concept[i]);
  }
  oml_free(concept);
  concept = (char***)oml_malloc(sizeof(char**)*2);
  concept[0] = (char**)oml_malloc(sizeof(char*)*3);
  concept[0][0]=strdup("omn-domain-pc:VM");
  concept[0][1]=strdup("rdfs:label");
  concept[0][2]=strdup("%value%");
  concept[1] = (char**)oml_malloc(sizeof(char*)*3);
  concept[1][0]=strdup("omn-domain-pc:VM");
  concept[1][1]=strdup("omn-lifecycle:childOf");
  concept[1][2]=strdup("omn-domain-pc:PC");
  oml_available_memory_def[3].relations = oml_sem_register_concepts(concept, 2);
  for (i=0;i<2;i++)
  {
    for (j=0;j<3;j++)
      free(concept[i][j]);
    oml_free(concept[i]);
  }
  oml_free(concept);
  g_oml_mps_infrastructure_monitoring->available_memory = omlc_add_mp("available_memory", oml_available_memory_def);
  concept = (char***)oml_malloc(sizeof(char**)*6);
  concept[0] = (char**)oml_malloc(sizeof(char*)*3);
  concept[0][0]=strdup("omn-monitoring:Measurement");
  concept[0][1]=strdup("omn-monitoring:isMeasurementOf");
  concept[0][2]=strdup("omn-monitoring-metric:UsedBandwidth");
  concept[1] = (char**)oml_malloc(sizeof(char*)*3);
  concept[1][0]=strdup("omn-monitoring-metric:UsedBandwidth");
  concept[1][1]=strdup("omn-monitoring:isMeasurementMetricOf");
  concept[1][2]=strdup("omn-domain-pc:PC");
  concept[2] = (char**)oml_malloc(sizeof(char*)*3);
  concept[2][0]=strdup("omn-monitoring-metric:UsedBandwidth");
  concept[2][1]=strdup("omn-monitoring-data:hasMeasurementData");
  concept[2][2]=strdup("omn-monitoring-data:MeasurementData");
  concept[3] = (char**)oml_malloc(sizeof(char*)*3);
  concept[3][0]=strdup("omn-monitoring-data:MeasurementData");
  concept[3][1]=strdup("omn-monitoring-data:hasMeasurementDataValue");
  concept[3][2]=strdup("%value%");
  concept[4] = (char**)oml_malloc(sizeof(char*)*3);
  concept[4][0]=strdup("omn-monitoring-data:MeasurementData");
  concept[4][1]=strdup("omn-monitoring:hasUnit");
  concept[4][2]=strdup("omn-monitoring-unit:bitpersecond");
  concept[5] = (char**)oml_malloc(sizeof(char*)*3);
  concept[5][0]=strdup("omn-monitoring-unit:bitpersecond");
  concept[5][1]=strdup("omn-monitoring-unit:hasPrefix");
  concept[5][2]=strdup("omn-monitoring-unit:mega");
  oml_used_bandwidth_def[0].relations = oml_sem_register_concepts(concept, 6);
  for (i=0;i<6;i++)
  {
    for (j=0;j<3;j++)
      free(concept[i][j]);
    oml_free(concept[i]);
  }
  oml_free(concept);
  concept = (char***)oml_malloc(sizeof(char**)*1);
  concept[0] = (char**)oml_malloc(sizeof(char*)*3);
  concept[0][0]=strdup("omn-monitoring-data:MeasurementData");
  concept[0][1]=strdup("omn-monitoring-data:hasTimestamp");
  concept[0][2]=strdup("%value%");
  oml_used_bandwidth_def[1].relations = oml_sem_register_concepts(concept, 1);
  for (i=0;i<1;i++)
  {
    for (j=0;j<3;j++)
      free(concept[i][j]);
    oml_free(concept[i]);
  }
  oml_free(concept);
  concept = (char***)oml_malloc(sizeof(char**)*1);
  concept[0] = (char**)oml_malloc(sizeof(char*)*3);
  concept[0][0]=strdup("omn-domain-pc:PC");
  concept[0][1]=strdup("rdfs:label");
  concept[0][2]=strdup("%value%");
  oml_used_bandwidth_def[2].relations = oml_sem_register_concepts(concept, 1);
  for (i=0;i<1;i++)
  {
    for (j=0;j<3;j++)
      free(concept[i][j]);
    oml_free(concept[i]);
  }
  oml_free(concept);
  concept = (char***)oml_malloc(sizeof(char**)*2);
  concept[0] = (char**)oml_malloc(sizeof(char*)*3);
  concept[0][0]=strdup("omn-domain-pc:VM");
  concept[0][1]=strdup("rdfs:label");
  concept[0][2]=strdup("%value%");
  concept[1] = (char**)oml_malloc(sizeof(char*)*3);
  concept[1][0]=strdup("omn-domain-pc:VM");
  concept[1][1]=strdup("omn-lifecycle:childOf");
  concept[1][2]=strdup("omn-domain-pc:PC");
  oml_used_bandwidth_def[3].relations = oml_sem_register_concepts(concept, 2);
  for (i=0;i<2;i++)
  {
    for (j=0;j<3;j++)
      free(concept[i][j]);
    oml_free(concept[i]);
  }
  oml_free(concept);
  g_oml_mps_infrastructure_monitoring->used_bandwidth = omlc_add_mp("used_bandwidth", oml_used_bandwidth_def);
  concept = (char***)oml_malloc(sizeof(char**)*4);
  concept[0] = (char**)oml_malloc(sizeof(char*)*3);
  concept[0][0]=strdup("omn-monitoring:Measurement");
  concept[0][1]=strdup("omn-monitoring:isMeasurementOf");
  concept[0][2]=strdup("omn-monitoring-metric:Availability");
  concept[1] = (char**)oml_malloc(sizeof(char*)*3);
  concept[1][0]=strdup("omn-monitoring-metric:Availability");
  concept[1][1]=strdup("omn-monitoring:isMeasurementMetricOf");
  concept[1][2]=strdup("omn-domain-pc:PC");
  concept[2] = (char**)oml_malloc(sizeof(char*)*3);
  concept[2][0]=strdup("omn-monitoring-metric:Availability");
  concept[2][1]=strdup("omn-monitoring-data:hasMeasurementData");
  concept[2][2]=strdup("omn-monitoring-data:MeasurementData");
  concept[3] = (char**)oml_malloc(sizeof(char*)*3);
  concept[3][0]=strdup("omn-monitoring-data:MeasurementData");
  concept[3][1]=strdup("omn-monitoring-data:hasMeasurementDataValue");
  concept[3][2]=strdup("%value%");
  oml_availability_def[0].relations = oml_sem_register_concepts(concept, 4);
  for (i=0;i<4;i++)
  {
    for (j=0;j<3;j++)
      free(concept[i][j]);
    oml_free(concept[i]);
  }
  oml_free(concept);
  concept = (char***)oml_malloc(sizeof(char**)*1);
  concept[0] = (char**)oml_malloc(sizeof(char*)*3);
  concept[0][0]=strdup("omn-monitoring-data:MeasurementData");
  concept[0][1]=strdup("omn-monitoring-data:hasTimestamp");
  concept[0][2]=strdup("%value%");
  oml_availability_def[1].relations = oml_sem_register_concepts(concept, 1);
  for (i=0;i<1;i++)
  {
    for (j=0;j<3;j++)
      free(concept[i][j]);
    oml_free(concept[i]);
  }
  oml_free(concept);
  concept = (char***)oml_malloc(sizeof(char**)*1);
  concept[0] = (char**)oml_malloc(sizeof(char*)*3);
  concept[0][0]=strdup("omn-domain-pc:PC");
  concept[0][1]=strdup("rdfs:label");
  concept[0][2]=strdup("%value%");
  oml_availability_def[2].relations = oml_sem_register_concepts(concept, 1);
  for (i=0;i<1;i++)
  {
    for (j=0;j<3;j++)
      free(concept[i][j]);
    oml_free(concept[i]);
  }
  oml_free(concept);
  concept = (char***)oml_malloc(sizeof(char**)*2);
  concept[0] = (char**)oml_malloc(sizeof(char*)*3);
  concept[0][0]=strdup("omn-domain-pc:VM");
  concept[0][1]=strdup("rdfs:label");
  concept[0][2]=strdup("%value%");
  concept[1] = (char**)oml_malloc(sizeof(char*)*3);
  concept[1][0]=strdup("omn-domain-pc:VM");
  concept[1][1]=strdup("omn-lifecycle:childOf");
  concept[1][2]=strdup("omn-domain-pc:PC");
  oml_availability_def[3].relations = oml_sem_register_concepts(concept, 2);
  for (i=0;i<2;i++)
  {
    for (j=0;j<3;j++)
      free(concept[i][j]);
    oml_free(concept[i]);
  }
  oml_free(concept);
  g_oml_mps_infrastructure_monitoring->availability = omlc_add_mp("availability", oml_availability_def);

}

#else
/*
 * Not included from the main source, only declare the global pointer
 * to the MPs and injection helpers.
 */

extern oml_mps_t* g_oml_mps_infrastructure_monitoring;

#endif /* OML_FROM_MAIN */

static inline int
oml_inject_used_memory(OmlMP *mp, double used_memory, const char *timestamp, const char *physicalresource, const char *virtualresource)
{
  int ret = -1;

  OmlValueU v[4];
  omlc_zero_array(v, 4);

  omlc_set_double(v[0], used_memory);
  omlc_set_string(v[1], timestamp);
  omlc_set_string(v[2], physicalresource);
  omlc_set_string(v[3], virtualresource);

  ret = omlc_inject(mp, v);

  omlc_reset_string(v[2]);
  omlc_reset_string(v[3]);
  return ret;
}

static inline int
oml_inject_total_memory(OmlMP *mp, double total_memory, const char *timestamp, const char *physicalresource, const char *virtualresource)
{
  int ret = -1;

  OmlValueU v[4];
  omlc_zero_array(v, 4);

  omlc_set_double(v[0], total_memory);
  omlc_set_string(v[1], timestamp);
  omlc_set_string(v[2], physicalresource);
  omlc_set_string(v[3], virtualresource);

  ret = omlc_inject(mp, v);

  omlc_reset_string(v[2]);
  omlc_reset_string(v[3]);
  return ret;
}

static inline int
oml_inject_available_memory(OmlMP *mp, double available_memory, const char *timestamp, const char *physicalresource, const char *virtualresource)
{
  int ret = -1;

  OmlValueU v[4];
  omlc_zero_array(v, 4);

  omlc_set_double(v[0], available_memory);
  omlc_set_string(v[1], timestamp);
  omlc_set_string(v[2], physicalresource);
  omlc_set_string(v[3], virtualresource);

  ret = omlc_inject(mp, v);

  omlc_reset_string(v[2]);
  omlc_reset_string(v[3]);
  return ret;
}

static inline int
oml_inject_used_bandwidth(OmlMP *mp, double used_bandwidth, const char *timestamp, const char *physicalresource, const char *virtualresource)
{
  int ret = -1;

  OmlValueU v[4];
  omlc_zero_array(v, 4);

  omlc_set_double(v[0], used_bandwidth);
  omlc_set_string(v[1], timestamp);
  omlc_set_string(v[2], physicalresource);
  omlc_set_string(v[3], virtualresource);

  ret = omlc_inject(mp, v);

  omlc_reset_string(v[2]);
  omlc_reset_string(v[3]);
  return ret;
}

static inline int
oml_inject_availability(OmlMP *mp, double availability, const char *timestamp, const char *physicalresource, const char *virtualresource)
{
  int ret = -1;

  OmlValueU v[4];
  omlc_zero_array(v, 4);

  omlc_set_double(v[0], availability);
  omlc_set_string(v[1], timestamp);
  omlc_set_string(v[2], physicalresource);
  omlc_set_string(v[3], virtualresource);

  ret = omlc_inject(mp, v);

  omlc_reset_string(v[2]);
  omlc_reset_string(v[3]);
  return ret;
}


/* Compatibility with old applications */
#ifndef g_oml_mps
# define g_oml_mps	g_oml_mps_infrastructure_monitoring
#endif

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_MONITORING_OML_H */
