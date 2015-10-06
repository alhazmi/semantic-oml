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

        app.defProperty('pm', 'URI of monitored physical host', '--pm-uri',
                :type => :string)
        app.defProperty('vm', 'URI of virtual host', '--vm-uri',
                :type => :string)
        app.defProperty('link', 'URI of a Link between two nodes', '--link-uri',
                :type => :string)

        app.defProperty('used_memory', 'Check used memory', '--used-memory',
                :type => :double)
        app.defProperty('used_memory_timestamp', 'Time of checking', '--used-memory-timestamp',
                :type => :string)
        app.defProperty('total_memory', 'Check total memory', '--total-memory',
                :type => :double)
        app.defProperty('total_memory_timestamp', 'Time of checking', '--total-memory-timestamp',
                :type => :string)
        app.defProperty('available_memory', 'Check available memory', '--available-memory',
                :type => :double)
        app.defProperty('available_memory_timestamp', 'Time of checking', '--available-memory-timestamp',
                :type => :string)
        app.defProperty('free_memory', 'Check free memory', '--free-memory',
                :type => :double)
        app.defProperty('free_memory_timestamp', 'Time of checking', '--free-memory-timestamp',
                :type => :string)
        app.defProperty('catched_memory', 'Check catched memory', '--catched-memory',
                :type => :double)
        app.defProperty('catched_memory_timestamp', 'Time of checking', '--catched-memory-timestamp',
                :type => :string)
        app.defProperty('buffers_memory', 'Check buffers memory', '--buffers-memory',
                :type => :double)
        app.defProperty('buffers_memory_timestamp', 'Time of checking', '--buffers-memory-timestamp',
                :type => :string)
        app.defProperty('used_swap', 'Check used swap space', '--used-swap',
                :type => :double)
        app.defProperty('used_swap_timestamp', 'Time of checking', '--used-swap-timestamp',
                :type => :string)
        app.defProperty('used_bandwidth', 'Check used bandwidth', '--used-bandwidth',
                :type => :double)
        app.defProperty('used_bandwidth_timestamp', 'Time of checking', '--used-bandwidth-timestamp',
                :type => :string)
        app.defProperty('cpu_idle', 'Check CPU Idle', '--cpu-idle',
                :type => :double)
        app.defProperty('cpu_idle_timestamp', 'Time of checking', '--cpu-idle-timestamp',
                :type => :string)
        app.defProperty('cpu_nice', 'Check CPU Nice', '--cpu-nice',
                :type => :double)
        app.defProperty('cpu_nice_timestamp', 'Time of checking', '--cpu-nice-timestamp',
                :type => :string)
        app.defProperty('cpu_iowait', 'Check CPU IOwait', '--cpu-iowait',
                :type => :double)
        app.defProperty('cpu_iowait_timestamp', 'Time of checking', '--cpu-iowait-timestamp',
                :type => :string)
        app.defProperty('availability', 'Check if a VM is available', '--availability',
                :type => :int)
        app.defProperty('availability_timestamp', 'Time of checking', '--availability-timestamp',
                :type => :string)
        app.defProperty('cpu_load', 'Check CPU Load', '--cpu-load',
                :type => :double)
        app.defProperty('cpu_load_timestamp', 'Time of checking', '--cpu-load-timestamp',
                :type => :string)
        app.defProperty('free_swap', 'Check Free Swap', '--free-swap',
                :type => :double)
        app.defProperty('free_swap_timestamp', 'Time of checking', '--free-swap-timestamp',
                :type => :string)
        app.defProperty('cpu_user', 'Check CPU User', '--cpu-user',
                :type => :double)
        app.defProperty('cpu_user_timestamp', 'Time of checking', '--cpu-user-timestamp',
                :type => :string)
        app.defProperty('total_swap', 'Check Total Swap', '--total-swap',
                :type => :double)
        app.defProperty('total_swap_timestamp', 'Time of checking', '--total-swap-timestamp',
                :type => :string)
        app.defProperty('cpu_system', 'Check CPU System', '--cpu-system',
                :type => :double)
        app.defProperty('cpu_system_timestamp', 'Time of checking', '--cpu-system-timestamp',
                :type => :string)
        app.defProperty('disk_ioread', 'Check Disk IO read', '--disk-ioread',
                :type => :double)
        app.defProperty('disk_ioread_timestamp', 'Time of checking', '--disk-ioread-timestamp',
                :type => :string)
        app.defProperty('free_disk_space', 'Check Free Disk Space', '--free-disk-space',
                :type => :double)
        app.defProperty('free_disk_space_timestamp', 'Time of checking', '--free-disk-space-timestamp',
                :type => :string)
        app.defProperty('used_disk_space', 'Check Used Disk Space', '--used-disk-space',
                :type => :double)
        app.defProperty('used_disk_space_timestamp', 'Time of checking', '--used-disk-space-timestamp',
                :type => :string)

        app.defProperty('packet_loss', 'Check Packet Loss', '--packet-loss',
                :type => :double)
        app.defProperty('packet_loss_timestamp', 'Time of checking', '--packet-loss-timestamp',
                :type => :string)
        app.defProperty('delay', 'Check Delay', '--delay',
                :type => :double)
        app.defProperty('delay_timestamp', 'Time of checking', '--delay-timestamp',
                :type => :string)

app.defMeasurement("used_memory"){ |m|
    m.defMetric('used_memory', :double, 'Used memory value of host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:isMeasurementDataOf','omn-monitoring-metric:UsedMemory'],
    ['omn-monitoring-metric:UsedMemory','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring:hasUnit','omn-monitoring-unit:Byte'],
    ['omn-monitoring-unit:Byte','omn-monitoring-unit:hasPrefix','omn-monitoring-unit:giga']])


    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'URI of monitored resource',
    [['omn-domain-pc:PC','omn:hasURI','%value%']])


    m.defMetric('virtualresource', :string, 'URI of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','omn:hasURI','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}


app.defMeasurement("total_memory"){ |m|
    m.defMetric('total_memory', :double, 'Total memory value of host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:isMeasurementDataOf','omn-monitoring-metric:TotalMemory'],
    ['omn-monitoring-metric:TotalMemory','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring:hasUnit','omn-monitoring-unit:Byte'],
    ['omn-monitoring-unit:Byte','omn-monitoring-unit:hasPrefix','omn-monitoring-unit:giga']])


    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'URI of monitored resource',
    [['omn-domain-pc:PC','omn:hasURI','%value%']])


    m.defMetric('virtualresource', :string, 'URI of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','omn:hasURI','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}


app.defMeasurement("available_memory"){ |m|
    m.defMetric('available_memory', :double, 'Available memory value of host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:isMeasurementDataOf','omn-monitoring-metric:AvailableMemory'],
    ['omn-monitoring-metric:AvailableMemory','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring:hasUnit','omn-monitoring-unit:Byte'],
    ['omn-monitoring-unit:Byte','omn-monitoring-unit:hasPrefix','omn-monitoring-unit:giga']])


    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'URI of monitored resource',
    [['omn-domain-pc:PC','omn:hasURI','%value%']])


    m.defMetric('virtualresource', :string, 'URI of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','omn:hasURI','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}

app.defMeasurement("free_memory"){ |m|
    m.defMetric('free_memory', :double, 'Free memory value of host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:isMeasurementDataOf','omn-monitoring-metric:FreeMemory'],
    ['omn-monitoring-metric:FreeMemory','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring:hasUnit','omn-monitoring-unit:Byte'],
    ['omn-monitoring-unit:Byte','omn-monitoring-unit:hasPrefix','omn-monitoring-unit:giga']])


    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'URI of monitored resource',
    [['omn-domain-pc:PC','omn:hasURI','%value%']])


    m.defMetric('virtualresource', :string, 'URI of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','omn:hasURI','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}

app.defMeasurement("cached_memory"){ |m|
    m.defMetric('cached_memory', :double, 'Cached memory value of host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:isMeasurementDataOf','omn-monitoring-metric:CachedMemory'],
    ['omn-monitoring-metric:CachedMemory','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring:hasUnit','omn-monitoring-unit:Byte'],
    ['omn-monitoring-unit:Byte','omn-monitoring-unit:hasPrefix','omn-monitoring-unit:giga']])


    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'URI of monitored resource',
    [['omn-domain-pc:PC','omn:hasURI','%value%']])


    m.defMetric('virtualresource', :string, 'URI of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','omn:hasURI','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}

app.defMeasurement("buffers_memory"){ |m|
    m.defMetric('buffers_memory', :double, 'Buffers memory value of host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:isMeasurementDataOf','omn-monitoring-metric:BuffersMemory'],
    ['omn-monitoring-metric:BuffersMemory','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring:hasUnit','omn-monitoring-unit:Byte'],
    ['omn-monitoring-unit:Byte','omn-monitoring-unit:hasPrefix','omn-monitoring-unit:giga']])


    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'URI of monitored resource',
    [['omn-domain-pc:PC','omn:hasURI','%value%']])


    m.defMetric('virtualresource', :string, 'URI of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','omn:hasURI','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}

app.defMeasurement("used_swap"){ |m|
    m.defMetric('used_swap', :double, 'Used swap space value of host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:isMeasurementDataOf','omn-monitoring-metric:UsedSwap'],
    ['omn-monitoring-metric:UsedSwap','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring:hasUnit','omn-monitoring-unit:percent']])

    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'URI of monitored resource',
    [['omn-domain-pc:PC','omn:hasURI','%value%']])


    m.defMetric('virtualresource', :string, 'URI of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','omn:hasURI','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}

app.defMeasurement("used_bandwidth"){ |m|
    m.defMetric('used_bandwidth', :double, 'used bandwidth of monitored host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:isMeasurementDataOf','omn-monitoring-metric:UsedBandwidth'],
    ['omn-monitoring-metric:UsedBandwidth','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring:hasUnit','omn-monitoring-unit:bitpersecond'],
    ['omn-monitoring-unit:bitpersecond','omn-monitoring-unit:hasPrefix','omn-monitoring-unit:mega']])


    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'URI of monitored resource',
    [['omn-domain-pc:PC','omn:hasURI','%value%']])


    m.defMetric('virtualresource', :string, 'URI of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','omn:hasURI','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}

app.defMeasurement("cpu_idle"){ |m|
    m.defMetric('cpu_idle', :double, 'CPU idle time of monitored host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:isMeasurementDataOf','omn-monitoring-metric:CPUidle'],
    ['omn-monitoring-metric:CPUidle','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring:hasUnit','omn-monitoring-unit:percent']])

    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'URI of monitored resource',
    [['omn-domain-pc:PC','omn:hasURI','%value%']])


    m.defMetric('virtualresource', :string, 'URI of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','omn:hasURI','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}

app.defMeasurement("cpu_nice"){ |m|
    m.defMetric('cpu_nice', :double, 'CPU nice time of monitored host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:isMeasurementDataOf','omn-monitoring-metric:CPUnice'],
    ['omn-monitoring-metric:CPUnice','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring:hasUnit','omn-monitoring-unit:percent']])

    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'URI of monitored resource',
    [['omn-domain-pc:PC','omn:hasURI','%value%']])


    m.defMetric('virtualresource', :string, 'URI of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','omn:hasURI','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}

app.defMeasurement("cpu_iowait"){ |m|
    m.defMetric('cpu_iowait', :double, 'CPU IOwait time of monitored host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:isMeasurementDataOf','omn-monitoring-metric:CPUIOwait'],
    ['omn-monitoring-metric:CPUIOwait','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring:hasUnit','omn-monitoring-unit:percent']])

    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'URI of monitored resource',
    [['omn-domain-pc:PC','omn:hasURI','%value%']])


    m.defMetric('virtualresource', :string, 'URI of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','omn:hasURI','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}

app.defMeasurement("availability"){ |m|
    m.defMetric('availability', :double, '1 if host is available, else 0',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:isMeasurementDataOf','omn-monitoring-metric:Availability'],
    ['omn-monitoring-metric:Availability','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasMeasurementDataValue','%value%']])


    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'URI of monitored resource',
    [['omn-domain-pc:PC','omn:hasURI','%value%']])


    m.defMetric('virtualresource', :string, 'URI of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','omn:hasURI','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}

app.defMeasurement("cpu_load"){ |m|
    m.defMetric('cpu_load', :double, 'CPU Load of monitored host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:isMeasurementDataOf','omn-monitoring-metric:CPULoad'],
    ['omn-monitoring-metric:CPULoad','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasMeasurementDataValue','%value%']])


    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'URI of monitored resource',
    [['omn-domain-pc:PC','omn:hasURI','%value%']])


    m.defMetric('virtualresource', :string, 'URI of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','omn:hasURI','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}

app.defMeasurement("free_swap"){ |m|
    m.defMetric('free_swap', :double, 'Free swap space value of host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:isMeasurementDataOf','omn-monitoring-metric:FreeSwap'],
    ['omn-monitoring-metric:FreeSwap','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring:hasUnit','omn-monitoring-unit:Byte'],
    ['omn-monitoring-unit:Byte','omn-monitoring-unit:hasPrefix','omn-monitoring-unit:giga']])


    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'URI of monitored resource',
    [['omn-domain-pc:PC','omn:hasURI','%value%']])


    m.defMetric('virtualresource', :string, 'URI of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','omn:hasURI','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}

app.defMeasurement("cpu_user"){ |m|
    m.defMetric('cpu_user', :double, 'CPU User time of monitored host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:isMeasurementDataOf','omn-monitoring-metric:CPUuser'],
    ['omn-monitoring-metric:CPUuser','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring:hasUnit','omn-monitoring-unit:percent']])

    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'URI of monitored resource',
    [['omn-domain-pc:PC','omn:hasURI','%value%']])


    m.defMetric('virtualresource', :string, 'URI of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','omn:hasURI','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}

app.defMeasurement("total_swap"){ |m|
    m.defMetric('total_swap', :double, 'Total swap space value of host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:isMeasurementDataOf','omn-monitoring-metric:TotalSwap'],
    ['omn-monitoring-metric:TotalSwap','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring:hasUnit','omn-monitoring-unit:Byte'],
    ['omn-monitoring-unit:Byte','omn-monitoring-unit:hasPrefix','omn-monitoring-unit:giga']])


    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'URI of monitored resource',
    [['omn-domain-pc:PC','omn:hasURI','%value%']])


    m.defMetric('virtualresource', :string, 'URI of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','omn:hasURI','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}

app.defMeasurement("cpu_system"){ |m|
    m.defMetric('cpu_system', :double, 'CPU System time of monitored host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:isMeasurementDataOf','omn-monitoring-metric:CPUsystem'],
    ['omn-monitoring-metric:CPUsystem','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring:hasUnit','omn-monitoring-unit:percent']])

    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'URI of monitored resource',
    [['omn-domain-pc:PC','omn:hasURI','%value%']])


    m.defMetric('virtualresource', :string, 'URI of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','omn:hasURI','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}

app.defMeasurement("diskIO_read"){ |m|
    m.defMetric('diskIO_read', :double, 'Disk IO read of monitored host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:isMeasurementDataOf','omn-monitoring-metric:DiskIORead'],
    ['omn-monitoring-metric:DiskIORead','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring:hasUnit','omn-monitoring-unit:Bytepersecond']])

    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'URI of monitored resource',
    [['omn-domain-pc:PC','omn:hasURI','%value%']])


    m.defMetric('virtualresource', :string, 'URI of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','omn:hasURI','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}

app.defMeasurement("free_disk_space"){ |m|
    m.defMetric('free_disk_space', :double, 'Free Disk Space of monitored host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:isMeasurementDataOf','omn-monitoring-metric:FreeDiskSpace'],
    ['omn-monitoring-metric:FreeDiskSpace','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring:hasUnit','omn-monitoring-unit:percent']])

    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'URI of monitored resource',
    [['omn-domain-pc:PC','omn:hasURI','%value%']])


    m.defMetric('virtualresource', :string, 'URI of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','omn:hasURI','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}

app.defMeasurement("used_disk_space"){ |m|
    m.defMetric('used_disk_space', :double, 'Used Disk Space of monitored host',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:isMeasurementDataOf','omn-monitoring-metric:UsedDiskSpace'],
    ['omn-monitoring-metric:UsedDiskSpace','omn-monitoring:isMeasurementMetricOf','omn-domain-pc:PC'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring:hasUnit','omn-monitoring-unit:Byte'],
    ['omn-monitoring-unit:Byte','omn-monitoring-unit:hasPrefix','omn-monitoring-unit:giga']])

    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('physicalresource', :string, 'URI of monitored resource',
    [['omn-domain-pc:PC','omn:hasURI','%value%']])


    m.defMetric('virtualresource', :string, 'URI of virtual host which is running on the monitored physical host',
    [['omn-domain-pc:VM','omn:hasURI','%value%'],
    ['omn-domain-pc:VM','omn-lifecycle:childOf','omn-domain-pc:PC']])}

app.defMeasurement("packet_loss"){ |m|
    m.defMetric('packet_loss', :int, 'Packet Loss value of host',
    [['omn-monitoring:SimpleMeasurement','omn-monitoring-data:isMeasurementDataOf','omn-monitoring-metric:PacketLoss'],
    ['omn-monitoring-metric:PacketLoss','omn-monitoring:isMeasurementMetricOf', 'omn-resource:Link'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring:hasUnit','omn-monitoring-unit:percent']])


    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasTimestamp','%value%']])

    m.defMetric('link', :string, 'URI of monitored resource',
    [['omn-resource:Link','omn:hasURI','%value%']])}

app.defMeasurement("delay"){ |m|
    m.defMetric('delay', :double, 'Delay value of host',
    [['omn-monitoring:SimpleMeasurement','omn-monitoring-data:isMeasurementDataOf','omn-monitoring-metric:Delay'],
    ['omn-monitoring-metric:Delay','omn-monitoring:isMeasurementMetricOf', 'omn-resource:Link'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:SimpleMeasurement','omn-monitoring:hasUnit','omn-monitoring-unit:second'],
    ['omn-monitoring-unit:second','omn-monitoring-unit:hasPrefix','omn-monitoring-unit:milli']])

    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:SimpleMeasurement','omn-monitoring-data:hasTimestamp','%value%']])

    m.defMetric('link', :string, 'URI of monitored resource',
    [['omn-resource:Link','omn:hasURI','%value%']])}

end

