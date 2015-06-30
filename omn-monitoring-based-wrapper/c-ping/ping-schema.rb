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


defApplication('oml:ping', 'ping') do |app|
        app.version(1,0)
        app.shortDescription = "Ping Monitoring"
        app.description = %{an application to provide measurement of Packet Loss and Delay}
        app.path = "/home/ubuntu/"

app.defMeasurement("packet_loss"){ |m|
    m.defMetric('packet_loss', :int, 'Packet Loss value of host',
    [['omn-monitoring:Measurement','omn-monitoring:isMeasurementOf','omn-monitoring-metric:PacketLoss'],
    ['omn-monitoring-metric:PacketLoss','omn-monitoring:isMeasurementMetricOf', 'omn-resource:Link'],
    ['omn-monitoring-metric:PacketLoss','omn-monitoring-data:hasMeasurementData','omn-monitoring-data:MeasurementData'],
    ['omn-monitoring-data:MeasurementData','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:MeasurementData','omn-monitoring:hasUnit','omn-monitoring-unit:percent']])


    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:MeasurementData','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('source_ip', :string, 'Source IP of monitored host',
    [['omn-resource:Link','omn-monitoring-data:hasSourceIP','%value%']])


    m.defMetric('destination_ip', :string, 'Destination IP of monitored host',
    [['omn-resource:Link','omn-monitoring-data:hasDestinationIP','%value%']])

    }


app.defMeasurement("delay"){ |m|
    m.defMetric('delay', :double, 'Delay value of host',
    [['omn-monitoring:Measurement','omn-monitoring:isMeasurementOf','omn-monitoring-metric:Delay'],
    ['omn-monitoring-metric:Delay','omn-monitoring:isMeasurementMetricOf', 'omn-resource:Link'],
    ['omn-monitoring-metric:Delay','omn-monitoring-data:hasMeasurementData','omn-monitoring-data:MeasurementData'],
    ['omn-monitoring-data:MeasurementData','omn-monitoring-data:hasMeasurementDataValue','%value%'],
    ['omn-monitoring-data:MeasurementData','omn-monitoring:hasUnit','omn-monitoring-unit:second'],
    ['omn-monitoring-unit:second','omn-monitoring-unit:hasPrefix','omn-monitoring-unit:milli']])


    m.defMetric('timestamp', :datetime, 'Time when the metric is measured',
    [['omn-monitoring-data:MeasurementData','omn-monitoring-data:hasTimestamp','%value%']])


    m.defMetric('source_ip', :string, 'Source IP of monitored host',
    [['omn-resource:Link','omn-monitoring-data:hasSourceIP','%value%']])


    m.defMetric('destination_ip', :string, 'Destination IP of monitored host',
    [['omn-resource:Link','omn-monitoring-data:hasDestinationIP','%value%']])
    }

end

