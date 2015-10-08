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
# This wrapper is implemented because there are currently no Zabbix library for C.
# It fetches the necessary data from Zabbix and 
# sends them to the semantic OML Server by calling the scaffold-generated C-Application. 
#
# The following files are needed to be in the same directory:
# Zabbix Library for Python (zabbix_api.py)
# Configuration file (monitoring-config-data.cfg)
# The generated C-Application (infrastructure-monitoring)

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
import time
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

logger=init_logger(monitoring_settings,'infrastructure-monitoring-c-wrapper.py')
logger.debug("infrastructure-monitoring-c-wrapper.py' has been started")

try:
        zabbix_server_uri = monitoring_settings['zabbixuri']
        zapi = ZabbixAPI(server=zabbix_server_uri, log_level=int(monitoring_settings['log_level']))
        zabbix_username = monitoring_settings['username']
        zabbix_password = monitoring_settings['password']
        zapi.login(zabbix_username,zabbix_password)

except Exception as e:
        print e
        logger.error("can not open zabbix.")

try:
        hostname = "<hostname>"
        vm = "vm"
        hostid = zapi.host.get({"filter":{"name":hostname},"output":"extend"}).pop()['hostid']

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

except Exception as e:
        logger.error("cannot fetch data from Zabbix.")

subprocess.call(["./infrastructure-monitoring","--vm-uri",str(vm),"--pm-uri",str(hostname),"--total-memory",str(totalmemory),"--total-memory-timestamp",str(totalmemory_ts),"--used-memory",str(usedmemory),"--used-memory-timestamp",str(usedmemory_ts),"--available-memory",str(availablememory),"--available-memory-timestamp",str(availablememory_ts),"--used-bandwidth",str(usedbandwidth),"--used-bandwidth-timestamp",str(usedbandwidth_ts),"--oml-id",str(monitoring_settings['appname']),"--oml-domain",str(monitoring_settings['domain']),"--oml-collect",str(monitoring_settings['target'])])


