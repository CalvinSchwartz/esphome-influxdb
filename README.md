# InfluxDB2 custom component for ESPHome
Changes from Jepsson/esphome-influxdb:
- Changed to InfluxDB2 API
- Made usable as external component
- Remove "device" as it can be replaced by a tag
- Made tags by sensor usable
- Added option to switch to https
- field_key can be set manually

## Installation
Add this repository as an submodule in your esphome custom_compontents;
`git clone https://github.com/CalvinSchwartz/esphome-influxdb custom_components/influxdb2`

## Usage

Add `influxdb2` section to your ESPHome configuration file.

### Example configuration

```yaml
external_components:
  - source: github://CalvinSchwartz/esphome-influxdb
    components: [ influxdb2 ]

influxdb2:
  host: "influxdb-host"
  orgid: "influx_org"
  token: "Token XXX"
  bucket: "influx_bucket"
  https: false
  precision: 2
  tags: # Optional
    room: MyRoom
    device: MyDevice
  sensors:
    meter_id:
      ignore: True
    ams_temperature:
      measurement: 'temperature'
      tags:
        sensortype: MySensor
```

### Configuration variables

* **host** (Required, string): Hostname or IP for the InfluxDB server
* **port** (Optional, int, default: 8086): Port number the InfluxDB server is listening on.
* **orgid** (Required, string): Organization ID when connecting to influxdb.
* **token** (Required, string): Token used when connecting to influxdb. Format: "Token XXXX"
* **bucket** (Required, string): Name of influxdb bucket.
* **https** (Optional, bool, default: false): Uses https and ignores the port setting.
* **precision** (Optional, int, default: 6): Number of decimal places.
* **send_timeout** (Optional, time, default: "500ms"): Time to wait before sending UDP packets which have not been filled to max size.
* **publish_all** (Optional, boolean, default: True): If true, publish updates from all sensors unless explicitly ignored in per sensor configuration. If false, only publish updates from sensors explicitly configured.
* **tags** (Optional, mapping, default 'node: <esphome.name>'): Mapping of tag keys and values. 
* **sensors** (Optional, mapping, default: {}): Per sensor configuration. Keys are sensor IDs. All types of sensors are included in this mapping, there is no distinction between float, binary and text sensors.

#### Sensor configuration variables

* **ignore** (Optional, boolean, default: False): Whether or not to include updates for this sensor.
* **measurement** (Optional, string): Name of measurements with update from this sensor. Defaults to the sanitized name of the sensor.
* **tags** (Optional, mapping, default: {}): Additional tags added for this sensor.
* **field_key** (Optional, string, default: value): Set field_key


## Update notes for ESPHome 2024.06
With ESPHome 2024.06 the http component was rewritten. Therefore this component had to be updated. In regards to this component the user has to do nothing. However, as the http component is utilized the corresponding configuration variables for ESP8266 have to be set, see: [HTTP Request](https://esphome.io/components/http_request.html#configuration-variables).

This means setting
```
http_request:
  verify_ssl: false
```
for ESP8266.

## ESPHome older than 2024.06
For ESPHome older than 2024.06 include the code as
```
external_components:
  - source: github://CalvinSchwartz/esphome-influxdb@old
    components: [ influxdb2 ]
    refresh: 0s
```
Refresh is optional and triggers a new pull from Github.