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
# This template contains the schemas based on OMN ontology.
# It is used by oml2-scaffold to generate an application skeleton based on the semantic schema


defApplication('oml:infrastructure-monitoring', 'infrastructure_monitoring') do |app|
        app.version(1,0)
        app.shortDescription = "Monitoring"
        app.description = %{an application to provide infrastructure resource monitoring}
        app.path = "/home/ubuntu/"

        app.defProperty('pm_name', 'Name of monitored physical host', '--pm-name',
                :type => :string)
        app.defProperty('vm_name', 'Name of virtual host', '--vm-name',
                :type => :string)

        app.defProperty('availability', 'Check if a VM is available', '--availability',
                :type => :int)
        app.defProperty('availability_timestamp', 'Time of checking', '--availability-timestamp',
                :type => :string)
        app.defProperty('used_memory', 'Check used memory', '--used-memory',
                :type => :double)
        app.defProperty('used_memory_timestamp', 'Time of checking', '--used-memory-timestamp',
                :type => :string)
        app.defProperty('available_memory', 'Check available memory', '--available-memory',
                :type => :double)
        app.defProperty('available_memory_timestamp', 'Time of checking', '--available-memory-timestamp',
                :type => :string)
        app.defProperty('total_memory', 'Check total memory', '--total-memory',
                :type => :double)
        app.defProperty('total_memory_timestamp', 'Time of checking', '--total-memory-timestamp',
                :type => :string)
        app.defProperty('used_bandwidth', 'Check used bandwidth', '--used-bandwidth',
                :type => :double)
        app.defProperty('used_bandwidth_timestamp', 'Time of checking', '--used-bandwidth-timestamp',
                :type => :string)
        app.defProperty('cpu_load', 'Check CPU Load', '--cpu-load',
                :type => :double)
        app.defProperty('cpu_load_timestamp', 'Time of checking', '--cpu-load-timestamp',
                :type => :string)

app.defMeasurement("used_memory"){ |m|
    m.defMetric('used_memory', :double, 'Used memory value of host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring:isMeasurementOf','omn-monitoring-metric:UsedMemory'],
    ['omn-monitoring-metric:UsedMemory','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-metric:UsedMemory','omn-monitoring-data:hasMeasurementData','omn-monitoring-data:MeasurementData'],
    ['omn-monitoring-data:MeasurementData','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:MeasurementData','omn-monitoring:hasUnit','omn-monitoring-unit:Byte'],
    ['omn-monitoring-unit:Byte','omn-monitoring-unit:hasPrefix','omn-monitoring-unit:giga']])


    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:MeasurementData','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'Name of monitored physical host',
    [['omn-domain-pc:PC','rdfs:label','%value%']])


    m.defMetric('virtualresource', :string, 'Name of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','rdfs:label','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}


app.defMeasurement("total_memory"){ |m|
    m.defMetric('total_memory', :double, 'Total memory value of host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring:isMeasurementOf','omn-monitoring-metric:TotalMemory'],
    ['omn-monitoring-metric:TotalMemory','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-metric:TotalMemory','omn-monitoring-data:hasMeasurementData','omn-monitoring-data:MeasurementData'],
    ['omn-monitoring-data:MeasurementData','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:MeasurementData','omn-monitoring:hasUnit','omn-monitoring-unit:Byte'],
    ['omn-monitoring-unit:Byte','omn-monitoring-unit:hasPrefix','omn-monitoring-unit:giga']])


    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:MeasurementData','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'Name of monitored physical host',
    [['omn-domain-pc:PC','rdfs:label','%value%']])


    m.defMetric('virtualresource', :string, 'Name of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','rdfs:label','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}


app.defMeasurement("available_memory"){ |m|
    m.defMetric('available_memory', :double, 'Available memory value of host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring:isMeasurementOf','omn-monitoring-metric:AvailableMemory'],
    ['omn-monitoring-metric:AvailableMemory','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-metric:AvailableMemory','omn-monitoring-data:hasMeasurementData','omn-monitoring-data:MeasurementData'],
    ['omn-monitoring-data:MeasurementData','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:MeasurementData','omn-monitoring:hasUnit','omn-monitoring-unit:Byte'],
    ['omn-monitoring-unit:Byte','omn-monitoring-unit:hasPrefix','omn-monitoring-unit:giga']])


    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:MeasurementData','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'Name of monitored physical host',
    [['omn-domain-pc:PC','rdfs:label','%value%']])


    m.defMetric('virtualresource', :string, 'Name of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','rdfs:label','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}


app.defMeasurement("used_bandwidth"){ |m|
    m.defMetric('used_bandwidth', :double, 'used bandwidth of monitored host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring:isMeasurementOf','omn-monitoring-metric:UsedBandwidth'],
    ['omn-monitoring-metric:UsedBandwidth','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-metric:UsedBandwidth','omn-monitoring-data:hasMeasurementData','omn-monitoring-data:MeasurementData'],
    ['omn-monitoring-data:MeasurementData','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:MeasurementData','omn-monitoring:hasUnit','omn-monitoring-unit:bitpersecond'],
    ['omn-monitoring-unit:bitpersecond','omn-monitoring-unit:hasPrefix','omn-monitoring-unit:mega']])


    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:MeasurementData','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'Name of monitored physical host',
    [['omn-domain-pc:PC','rdfs:label','%value%']])


    m.defMetric('virtualresource', :string, 'Name of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','rdfs:label','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}


app.defMeasurement("availability"){ |m|
    m.defMetric('availability', :double, '1 if host is available, else 0',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring:isMeasurementOf','omn-monitoring-metric:Availability'],
    ['omn-monitoring-metric:Availability','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-metric:Availability','omn-monitoring-data:hasMeasurementData','omn-monitoring-data:MeasurementData'],
    ['omn-monitoring-data:MeasurementData','omn-monitoring-data:hasMeasurementDataValue','%value%']])


    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:MeasurementData','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'Name of monitored physical host',
    [['omn-domain-pc:PC','rdfs:label','%value%']])


    m.defMetric('virtualresource', :string, 'Name of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','rdfs:label','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}

app.defMeasurement("cpu_load"){ |m|
    m.defMetric('cpu_load', :double, 'CPU Load of monitored host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring:isMeasurementOf','omn-monitoring-metric:CPULoad'],
    ['omn-monitoring-metric:CPULoad','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-metric:CPULoad','omn-monitoring-data:hasMeasurementData','omn-monitoring-data:MeasurementData'],
    ['omn-monitoring-data:MeasurementData','omn-monitoring-data:hasMeasurementDataValue','%value%']])


    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:MeasurementData','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'Name of monitored physical host',
    [['omn-domain-pc:PC','rdfs:label','%value%']])


    m.defMetric('virtualresource', :string, 'Name of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','rdfs:label','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}

end

