# Copyright (c) 2015, Andisa Dewi, Yahya Al-Hazmi, Technische Universitaet Berlin

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License
#
# This wrapper reads data from SQLite database, fetches the necessary data from Zabbix and 
# sends them to the semantic OML Server. 
#
# The following files are needed to be in the same directory:
# Zabbix Library for Python (zabbix_api.py)
# Configuration file (monitoring-config-data.cfg)
# SQLite database (sqliteDB.db)
# OML Library for Python (oml4py.py)

import oml4py

import sqlite3
import sys
import logging
import logging.handlers
import subprocess

import time
import math
from datetime import datetime
import ast
import exceptions
import cStringIO
import os
from zabbix_api import ZabbixAPI
import tempfile
from time import sleep
import pytz

def init_logger(settings,name):
    logger=logging.getLogger(name)
    logfilename=settings['logger_filename']
    if(settings['logger_loglevel']=="DEBUG"):
        loglevel=logging.DEBUG
    elif settings['logger_loglevel']=="INFO":
        loglevel=logging.INFO
    elif settings['logger_loglevel']=="WARNING":
        loglevel=logging.WARNING
    else:
        loglevel=logging.ERROR

    logformatter=logging.Formatter(settings['logger_formatter'])
    logger.setLevel(loglevel)
    if(settings['logger_toconsole']=="1"):
        ch1 = logging.StreamHandler()
        ch1.setLevel(loglevel)
        ch1.setFormatter(logformatter)
        logger.addHandler(ch1)
    ch2 = logging.handlers.RotatingFileHandler(logfilename, maxBytes=int(settings['logger_maxBytes']), backupCount=int(settings['logger_backupCount']))
    ch2.setLevel(loglevel)
    ch2.setFormatter(logformatter)
    logger.addHandler(ch2)
    return logger

def read_config(filename):
    try:
        f = open(filename, "r")
    except:
        logger.error("can not read file %s, script terminated" % (filename))
        sys.exit()
    try:
        dictionsry = {}
        for line in f:
            splitchar = '='
            kv = line.split(splitchar)
            if (len(kv)==2):
                dictionsry[kv[0]] = str(kv[1])[1:-2]
        return dictionsry
    except:
        logger.error("can not read file %s to a dictionary, format must be KEY=VALUE" % (filename))
        sys.exit()

def connect_sqlite() :
        try :
                ## create a SQLite database if not exist and connect to it
                con = sqlite3.connect(monitoring_settings['sqliteDB'])
        except Exception :
                logger.error("Cannot connect to SQLite3.")
                sys.exit()
        return con

monitoring_settings=read_config('./monitoring-config-data.cfg')

logger=init_logger(monitoring_settings,'infrastructure-monitoring-wrapper.py')
logger.debug("infrastructure-monitoring-wrapper.py' has been started")

con = connect_sqlite()
logger.debug("Connecting to SQLite...")

with con :
                try :
                        cur = con.cursor()
                        cur.execute("select distinct(host_name), collector_uri, vm_uri from virtual_physical_map")
                        rows = cur.fetchall()
                        logger.debug("Fetching all host names from database...")
                        for row in rows :
                                print row[0], row[1], row[2]
                except:
                        logger.error("Error fetching data from SQLite.")
                        sys.exit()

if not rows :
        logger.error("No host name found. Exiting...")
        sys.exit()

#The following was automatically generated by oml2-scaffold V2.12.0pre.79-58cf-dirty
#for infrastructure monitoring version 1.0.0.

omlInst = oml4py.OMLBase(monitoring_settings["appname"],monitoring_settings["domain"],monitoring_settings["sender"], monitoring_settings["target"])

#-----------------------------------------------------------------------#

omlInst.addmp("used_memory", "used_memory:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring:isMeasurementOf|omn-monitoring-metric:UsedMemory}{omn-monitoring-metric:UsedMemory|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-metric:UsedMemory|omn-monitoring-data:hasMeasurementData|omn-monitoring-data:MeasurementData}{omn-monitoring-data:MeasurementData|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:MeasurementData|omn-monitoring:hasUnit|omn-monitoring-unit:Byte}{omn-monitoring-unit:Byte|omn-monitoring-unit:hasPrefix|omn-monitoring-unit:giga} timestamp:datetime:{omn-monitoring-data:MeasurementData|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|rdfs:label|%value%} virtualresource:string:{omn-domain-pc:VM|rdfs:label|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")
omlInst.addmp("total_memory", "total_memory:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring:isMeasurementOf|omn-monitoring-metric:TotalMemory}{omn-monitoring-metric:TotalMemory|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-metric:TotalMemory|omn-monitoring-data:hasMeasurementData|omn-monitoring-data:MeasurementData}{omn-monitoring-data:MeasurementData|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:MeasurementData|omn-monitoring:hasUnit|omn-monitoring-unit:Byte}{omn-monitoring-unit:Byte|omn-monitoring-unit:hasPrefix|omn-monitoring-unit:giga} timestamp:datetime:{omn-monitoring-data:MeasurementData|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|rdfs:label|%value%} virtualresource:string:{omn-domain-pc:VM|rdfs:label|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")
omlInst.addmp("available_memory", "available_memory:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring:isMeasurementOf|omn-monitoring-metric:AvailableMemory}{omn-monitoring-metric:AvailableMemory|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-metric:AvailableMemory|omn-monitoring-data:hasMeasurementData|omn-monitoring-data:MeasurementData}{omn-monitoring-data:MeasurementData|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:MeasurementData|omn-monitoring:hasUnit|omn-monitoring-unit:Byte}{omn-monitoring-unit:Byte|omn-monitoring-unit:hasPrefix|omn-monitoring-unit:giga} timestamp:datetime:{omn-monitoring-data:MeasurementData|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|rdfs:label|%value%} virtualresource:string:{omn-domain-pc:VM|rdfs:label|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")
omlInst.addmp("used_bandwidth", "used_bandwidth:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring:isMeasurementOf|omn-monitoring-metric:UsedBandwidth}{omn-monitoring-metric:UsedBandwidth|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-metric:UsedBandwidth|omn-monitoring-data:hasMeasurementData|omn-monitoring-data:MeasurementData}{omn-monitoring-data:MeasurementData|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:MeasurementData|omn-monitoring:hasUnit|omn-monitoring-unit:bitpersecond}{omn-monitoring-unit:bitpersecond|omn-monitoring-unit:hasPrefix|omn-monitoring-unit:mega} timestamp:datetime:{omn-monitoring-data:MeasurementData|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|rdfs:label|%value%} virtualresource:string:{omn-domain-pc:VM|rdfs:label|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")
omlInst.addmp("availability", "availability:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring:isMeasurementOf|omn-monitoring-metric:Availability}{omn-monitoring-metric:Availability|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-metric:Availability|omn-monitoring-data:hasMeasurementData|omn-monitoring-data:MeasurementData}{omn-monitoring-data:MeasurementData|omn-monitoring-data:hasMeasurementDataValue|%value%} timestamp:datetime:{omn-monitoring-data:MeasurementData|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|rdfs:label|%value%} virtualresource:string:{omn-domain-pc:VM|rdfs:label|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")
omlInst.addmp("cpu_load", "cpu_load:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring:isMeasurementOf|omn-monitoring-metric:CPULoad}{omn-monitoring-metric:CPULoad|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-metric:CPULoad|omn-monitoring-data:hasMeasurementData|omn-monitoring-data:MeasurementData}{omn-monitoring-data:MeasurementData|omn-monitoring-data:hasMeasurementDataValue|%value%} timestamp:datetime:{omn-monitoring-data:MeasurementData|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|rdfs:label|%value%} virtualresource:string:{omn-domain-pc:VM|rdfs:label|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")

#-----------------------------------------------------------------------#
omlInst.start()

while True :
	try:
		zabbix_server_uri = monitoring_settings['zabbixuri']
		zapi = ZabbixAPI(server=zabbix_server_uri, log_level=int(monitoring_settings['log_level']))
		zabbix_username = monitoring_settings['username']
		zabbix_password = monitoring_settings['password']
		zapi.login(zabbix_username,zabbix_password)

	except Exception as e:
		print e
		logger.error("can not open zabbix.")
		sys.exit()

	for row in rows :
		try:
			hostid = zapi.host.get({"filter":{"name":row[0]},"output":"extend"}).pop()['hostid']

			#metrics
			item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"name":"Total memory"}}).pop()
			totalmemory = float(item['lastvalue']) / (1024)**3
			totalmemory_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))

			item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"name":"Used memory"}}).pop()
			usedmemory = float(item['lastvalue']) / (1024)**3
			usedmemory_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))

			item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"name":"Available memory"}}).pop()
			availablememory = float(item['lastvalue']) / (1024)**3
			availablememory_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))
			
			item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"key_":"net.if.in[eth2]"}}).pop()
			usedbandwidth = float(item['lastvalue']) / (1024)**2
			usedbandwidth_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))

			item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"key_":"system.cpu.load[percpu,avg5]"}}).pop()
                        cpuload = float(item['lastvalue'])
                        cpuload_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))

		except Exception as e:
			print e
			logger.error("cannot fetch data from Zabbix.")
			sys.exit()

		omlInst.inject("used_memory", [totalmemory, totalmemory_ts, row[0], row[2]])
		omlInst.inject("total_memory", [usedmemory, usedmemory_ts, row[0], row[2]])
		omlInst.inject("available_memory", [availablememory, availablememory_ts, row[0], row[2]])
		omlInst.inject("used_bandwidth", [usedbandwidth, usedbandwidth_ts, row[0], row[2]])
		omlInst.inject("cpu_load", [cpuload, cpuload_ts, row[0], row[2]])

	sleep(30)

omlInst.close()



