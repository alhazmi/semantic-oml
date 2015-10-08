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

	app.defProperty('link', 'URI of a Link between two nodes', '--link-uri',
                :type => :string)
	app.defProperty('packet_loss', 'Check Packet Loss', '--packet-loss',
                :type => :double)
        app.defProperty('packet_loss_timestamp', 'Time of checking', '--packet-loss-timestamp',
                :type => :string)
        app.defProperty('delay', 'Check Delay', '--delay',
                :type => :double)
        app.defProperty('delay_timestamp', 'Time of checking', '--delay-timestamp',
                :type => :string)

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

