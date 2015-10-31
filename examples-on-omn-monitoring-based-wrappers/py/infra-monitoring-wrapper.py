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
# This wrapper fetches the necessary data from Zabbix and 
# sends them to the semantic OML Server. 
#
# The following files are needed to be in the same directory:
# Zabbix Library for Python (zabbix_api.py)
# Configuration file (monitoring-config-data.cfg)
# OML Library for Python (oml4py.py)

import oml4py
import sys
import logging
import logging.handlers
import subprocess

import numpy as np
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

monitoring_settings=read_config('../monitoring-config-data.cfg')

logger=init_logger(monitoring_settings,'infrastructure-monitoring-wrapper.py')
logger.debug("infrastructure-monitoring-wrapper.py' has been started")

#The following was automatically generated by oml2-scaffold V2.12.0pre.79-58cf-dirty
#for infrastructure monitoring version 1.0.0.
omlInst = oml4py.OMLBase(monitoring_settings["appname"],monitoring_settings["domain"],monitoring_settings["sender"], monitoring_settings["target"])

#-----------------------------------------------------------------------#
omlInst.addmp("used_memory", "used_memory:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:isMeasurementDataOf|omn-monitoring-metric:UsedMemory}{omn-monitoring-metric:UsedMemory|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:SimpleMeasurement|omn-monitoring:hasUnit|omn-monitoring-unit:Byte}{omn-monitoring-unit:Byte|omn-monitoring-unit:hasPrefix|omn-monitoring-unit:giga} timestamp:datetime:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|omn:hasURI|%value%} virtualresource:string:{omn-domain-pc:VM|omn:hasURI|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")
omlInst.addmp("total_memory", "total_memory:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:isMeasurementDataOf|omn-monitoring-metric:TotalMemory}{omn-monitoring-metric:TotalMemory|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:SimpleMeasurement|omn-monitoring:hasUnit|omn-monitoring-unit:Byte}{omn-monitoring-unit:Byte|omn-monitoring-unit:hasPrefix|omn-monitoring-unit:giga} timestamp:datetime:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|omn:hasURI|%value%} virtualresource:string:{omn-domain-pc:VM|omn:hasURI|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")
omlInst.addmp("available_memory", "available_memory:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:isMeasurementDataOf|omn-monitoring-metric:AvailableMemory}{omn-monitoring-metric:AvailableMemory|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:SimpleMeasurement|omn-monitoring:hasUnit|omn-monitoring-unit:Byte}{omn-monitoring-unit:Byte|omn-monitoring-unit:hasPrefix|omn-monitoring-unit:giga} timestamp:datetime:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|omn:hasURI|%value%} virtualresource:string:{omn-domain-pc:VM|omn:hasURI|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")
omlInst.addmp("free_memory", "free_memory:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:isMeasurementDataOf|omn-monitoring-metric:FreeMemory}{omn-monitoring-metric:FreeMemory|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:SimpleMeasurement|omn-monitoring:hasUnit|omn-monitoring-unit:Byte}{omn-monitoring-unit:Byte|omn-monitoring-unit:hasPrefix|omn-monitoring-unit:giga} timestamp:datetime:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|omn:hasURI|%value%} virtualresource:string:{omn-domain-pc:VM|omn:hasURI|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")
omlInst.addmp("cached_memory", "cached_memory:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:isMeasurementDataOf|omn-monitoring-metric:CachedMemory}{omn-monitoring-metric:CachedMemory|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:SimpleMeasurement|omn-monitoring:hasUnit|omn-monitoring-unit:Byte}{omn-monitoring-unit:Byte|omn-monitoring-unit:hasPrefix|omn-monitoring-unit:giga} timestamp:datetime:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|omn:hasURI|%value%} virtualresource:string:{omn-domain-pc:VM|omn:hasURI|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")

omlInst.addmp("used_swap", "used_swap:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:isMeasurementDataOf|omn-monitoring-metric:UsedSwap}{omn-monitoring-metric:UsedSwap|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:SimpleMeasurement|omn-monitoring:hasUnit|omn-monitoring-unit:percent} timestamp:datetime:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|omn:hasURI|%value%} virtualresource:string:{omn-domain-pc:VM|omn:hasURI|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")
omlInst.addmp("used_bandwidth", "used_bandwidth:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:isMeasurementDataOf|omn-monitoring-metric:UsedBandwidth}{omn-monitoring-metric:UsedBandwidth|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:SimpleMeasurement|omn-monitoring:hasUnit|omn-monitoring-unit:bitpersecond}{omn-monitoring-unit:bitpersecond|omn-monitoring-unit:hasPrefix|omn-monitoring-unit:mega} timestamp:datetime:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|omn:hasURI|%value%} virtualresource:string:{omn-domain-pc:VM|omn:hasURI|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")
omlInst.addmp("cpu_idle", "cpu_idle:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:isMeasurementDataOf|omn-monitoring-metric:CPUidle}{omn-monitoring-metric:CPUidle|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:SimpleMeasurement|omn-monitoring:hasUnit|omn-monitoring-unit:percent} timestamp:datetime:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|omn:hasURI|%value%} virtualresource:string:{omn-domain-pc:VM|omn:hasURI|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")
omlInst.addmp("cpu_nice", "cpu_nice:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:isMeasurementDataOf|omn-monitoring-metric:CPUnice}{omn-monitoring-metric:CPUnice|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:SimpleMeasurement|omn-monitoring:hasUnit|omn-monitoring-unit:percent} timestamp:datetime:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|omn:hasURI|%value%} virtualresource:string:{omn-domain-pc:VM|omn:hasURI|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")
omlInst.addmp("cpu_iowait", "cpu_iowait:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:isMeasurementDataOf|omn-monitoring-metric:CPUIOwait}{omn-monitoring-metric:CPUIOwait|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:SimpleMeasurement|omn-monitoring:hasUnit|omn-monitoring-unit:percent} timestamp:datetime:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|omn:hasURI|%value%} virtualresource:string:{omn-domain-pc:VM|omn:hasURI|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")

omlInst.addmp("availability", "availability:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:isMeasurementDataOf|omn-monitoring-metric:Availability}{omn-monitoring-metric:Availability|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasMeasurementDataValue|%value%} timestamp:datetime:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|omn:hasURI|%value%} virtualresource:string:{omn-domain-pc:VM|omn:hasURI|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")
omlInst.addmp("cpu_load", "cpu_load:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:isMeasurementDataOf|omn-monitoring-metric:CPULoad}{omn-monitoring-metric:CPULoad|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasMeasurementDataValue|%value%} timestamp:datetime:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|omn:hasURI|%value%} virtualresource:string:{omn-domain-pc:VM|omn:hasURI|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")
omlInst.addmp("buffers_memory", "buffers_memory:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:isMeasurementDataOf|omn-monitoring-metric:BuffersMemory}{omn-monitoring-metric:BuffersMemory|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:SimpleMeasurement|omn-monitoring:hasUnit|omn-monitoring-unit:Byte}{omn-monitoring-unit:Byte|omn-monitoring-unit:hasPrefix|omn-monitoring-unit:giga} timestamp:datetime:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|omn:hasURI|%value%} virtualresource:string:{omn-domain-pc:VM|omn:hasURI|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")
omlInst.addmp("free_swap", "free_swap:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:isMeasurementDataOf|omn-monitoring-metric:FreeSwap}{omn-monitoring-metric:FreeSwap|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:SimpleMeasurement|omn-monitoring:hasUnit|omn-monitoring-unit:Byte}{omn-monitoring-unit:Byte|omn-monitoring-unit:hasPrefix|omn-monitoring-unit:giga} timestamp:datetime:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|omn:hasURI|%value%} virtualresource:string:{omn-domain-pc:VM|omn:hasURI|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")
omlInst.addmp("cpu_user", "cpu_user:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:isMeasurementDataOf|omn-monitoring-metric:CPUuser}{omn-monitoring-metric:CPUuser|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:SimpleMeasurement|omn-monitoring:hasUnit|omn-monitoring-unit:percent} timestamp:datetime:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|omn:hasURI|%value%} virtualresource:string:{omn-domain-pc:VM|omn:hasURI|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")

omlInst.addmp("total_swap", "total_swap:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:isMeasurementDataOf|omn-monitoring-metric:TotalSwap}{omn-monitoring-metric:TotalSwap|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:SimpleMeasurement|omn-monitoring:hasUnit|omn-monitoring-unit:Byte}{omn-monitoring-unit:Byte|omn-monitoring-unit:hasPrefix|omn-monitoring-unit:giga} timestamp:datetime:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|omn:hasURI|%value%} virtualresource:string:{omn-domain-pc:VM|omn:hasURI|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")
omlInst.addmp("cpu_system", "cpu_system:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:isMeasurementDataOf|omn-monitoring-metric:CPUsystem}{omn-monitoring-metric:CPUsystem|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:SimpleMeasurement|omn-monitoring:hasUnit|omn-monitoring-unit:percent} timestamp:datetime:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|omn:hasURI|%value%} virtualresource:string:{omn-domain-pc:VM|omn:hasURI|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")
omlInst.addmp("diskIO_read", "diskIO_read:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:isMeasurementDataOf|omn-monitoring-metric:DiskIORead}{omn-monitoring-metric:DiskIORead|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:SimpleMeasurement|omn-monitoring:hasUnit|omn-monitoring-unit:Bytepersecond} timestamp:datetime:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|omn:hasURI|%value%} virtualresource:string:{omn-domain-pc:VM|omn:hasURI|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")
omlInst.addmp("free_disk_space", "free_disk_space:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:isMeasurementDataOf|omn-monitoring-metric:FreeDiskSpace}{omn-monitoring-metric:FreeDiskSpace|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:SimpleMeasurement|omn-monitoring:hasUnit|omn-monitoring-unit:percent} timestamp:datetime:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|omn:hasURI|%value%} virtualresource:string:{omn-domain-pc:VM|omn:hasURI|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")
omlInst.addmp("used_disk_space", "used_disk_space:double:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:isMeasurementDataOf|omn-monitoring-metric:UsedDiskSpace}{omn-monitoring-metric:UsedDiskSpace|omn-monitoring:isMeasurementMetricOf|omn-domain-pc:PC}{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:SimpleMeasurement|omn-monitoring:hasUnit|omn-monitoring-unit:Byte}{omn-monitoring-unit:Byte|omn-monitoring-unit:hasPrefix|omn-monitoring-unit:giga} timestamp:datetime:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasTimestamp|%value%} physicalresource:string:{omn-domain-pc:PC|omn:hasURI|%value%} virtualresource:string:{omn-domain-pc:VM|omn:hasURI|%value%}{omn-domain-pc:VM|omn-lifecycle:childOf|omn-domain-pc:PC} ")

omlInst.addmp("packet_loss", "packet_loss:double:{omn-monitoring:SimpleMeasurement|omn-monitoring-data:isMeasurementDataOf|omn-monitoring-metric:PacketLoss}{omn-monitoring-metric:PacketLoss|omn-monitoring:isMeasurementMetricOf|omn-resource:Link}{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:SimpleMeasurement|omn-monitoring:hasUnit|omn-monitoring-unit:percent} timestamp:datetime:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasTimestamp|%value%} link:string:{omn-resource:Link|omn:hasURI|%value%} ")
omlInst.addmp("delay", "delay:double:{omn-monitoring:SimpleMeasurement|omn-monitoring-data:isMeasurementDataOf|omn-monitoring-metric:Delay}{omn-monitoring-metric:Delay|omn-monitoring:isMeasurementMetricOf|omn-resource:Link}{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasMeasurementDataValue|%value%}{omn-monitoring-data:SimpleMeasurement|omn-monitoring:hasUnit|omn-monitoring-unit:second}{omn-monitoring-unit:second|omn-monitoring-unit:hasPrefix|omn-monitoring-unit:milli} timestamp:datetime:{omn-monitoring-data:SimpleMeasurement|omn-monitoring-data:hasTimestamp|%value%} link:string:{omn-resource:Link|omn:hasURI|%value%} ")

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
            logger.error("can not open zabbix. Try again in 30 secs...")

        try:
            hostname = <hostname>
            vm = "http://localhost/VM-Server1"
            hostid = zapi.host.get({"filter":{"name":hostname},"output":"extend"}).pop()['hostid']

            #metrics
            item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"name":"Used memory"}}).pop()
            usedmemory = float(item['lastvalue']) / (1024)**3
            usedmemory_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))

            item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"name":"Total memory"}}).pop()
            totalmemory = float(item['lastvalue']) / (1024)**3
            totalmemory_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))

            item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"name":"Available memory"}}).pop()
            availablememory = float(item['lastvalue']) / (1024)**3
            availablememory_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))

            item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"name":"Free memory"}}).pop()
            freememory = float(item['lastvalue']) / (1024)**3
            freememory_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))

            item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"name":"Cached memory"}}).pop()
            cachedmemory = float(item['lastvalue']) / (1024)**3
            cachedmemory_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))


            item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"key_":"system.swap.size[,pused]"}}).pop()
            usedswap = float(item['lastvalue'])
            usedswap_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))

            item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"key_":"net.if.in[eth2]"}}).pop()
            usedbandwidth = float(item['lastvalue']) / (1024)**2
            usedbandwidth_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))

            item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"key_":"system.cpu.util[,idle]"}}).pop()
            cpuidle = float(item['lastvalue'])
            cpuidle_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))
            
	    item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"key_":"system.cpu.util[,nice]"}}).pop()
            cpunice = float(item['lastvalue'])
            cpunice_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))

            item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"key_":"system.cpu.util[,iowait]"}}).pop()
            cpuiowait = float(item['lastvalue'])
            cpuiowait_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))


            item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"name":"Openstack is running"}}).pop()
            availability = int(item['lastvalue'])
            availability_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))

            item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"key_":"system.cpu.load[percpu,avg5]"}}).pop()
            cpuload = float(item['lastvalue'])
            cpuload_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))

            item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"name":"Buffers memory"}}).pop()
            buffersmemory = float(item['lastvalue']) / (1024)**3
            buffersmemory_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))

            item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"name":"Free swap space"}}).pop()
            freeswap = float(item['lastvalue']) / (1024)**3
            freeswap_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))
             
            item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"key_":"system.cpu.util[,user]"}}).pop()
            cpuuser = float(item['lastvalue'])
            cpuuser_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))


            item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"name":"Total swap space"}}).pop()
            totalswap = float(item['lastvalue']) / (1024)**3
            totalswap_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))

            item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"key_":"system.cpu.util[,system]"}}).pop()
            cpusystem = float(item['lastvalue'])
            cpusystem_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))

            item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"name":"Disk IO read"}}).pop()
            diskioread = float(item['lastvalue'])
            diskioread_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))

            item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"key_":"vfs.fs.size[/,pfree]"}}).pop()
            freediskspace = float(item['lastvalue'])
            freediskspace_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))

            item = zapi.item.get({"output": "extend","hostids":hostid,"search":{"key_":"vfs.fs.size[/,used]"}}).pop()
            useddiskspace = float(item['lastvalue']) / (1024)**3
            useddiskspace_ts = datetime.fromtimestamp(int(item['lastclock']),pytz.timezone("Europe/Berlin"))

	except Exception as e:
		print e
		logger.error("cannot fetch data from Zabbix. Try again in 30 secs...")

        omlInst.inject("used_memory", [usedmemory, usedmemory_ts, hostname, vm])
	omlInst.inject("total_memory", [totalmemory, totalmemory_ts, hostname, vm])
	omlInst.inject("available_memory", [availablememory, availablememory_ts, hostname, vm])
        omlInst.inject("free_memory", [freememory, freememory_ts, hostname, vm])
        omlInst.inject("cached_memory", [cachedmemory, cachedmemory_ts, hostname, vm])

        omlInst.inject("used_swap", [usedswap, usedswap_ts, hostname, vm])
	omlInst.inject("used_bandwidth", [usedbandwidth, usedbandwidth_ts, hostname, vm])
        omlInst.inject("cpu_idle", [cpuidle, cpuidle_ts, hostname, vm])
        omlInst.inject("cpu_nice", [cpunice, cpunice_ts, hostname, vm])
        omlInst.inject("cpu_iowait", [cpuiowait, cpuiowait_ts, hostname, vm])

	omlInst.inject("availability", [availability, availability_ts, hostname, vm])
	omlInst.inject("cpu_load", [cpuload, cpuload_ts, hostname, vm])
        omlInst.inject("buffers_memory", [buffersmemory, buffersmemory_ts, hostname, vm])
        omlInst.inject("free_swap", [freeswap, freeswap_ts, hostname, vm])
        omlInst.inject("cpu_user", [cpuuser, cpuuser_ts, hostname, vm])

        omlInst.inject("total_swap", [totalswap, totalswap_ts, hostname, vm])
        omlInst.inject("cpu_system", [cpusystem, cpusystem_ts, hostname, vm])
        omlInst.inject("diskIO_read", [diskioread, diskioread_ts, hostname, vm])
        omlInst.inject("free_disk_space", [freediskspace, freediskspace_ts, hostname, vm])
        omlInst.inject("used_disk_space", [useddiskspace, useddiskspace_ts, hostname, vm])

	sleep(30)

omlInst.close()




