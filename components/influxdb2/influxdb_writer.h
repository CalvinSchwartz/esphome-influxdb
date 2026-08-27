#pragma once

#include "esphome/core/component.h"
#include "esphome/core/controller.h"
#include "esphome/core/defines.h"
#include "esphome/core/entity_base.h"
#include "esphome/core/log.h"
#include <vector>

#include "esphome/components/http_request/http_request.h"

namespace esphome {
namespace influxdb {

class InfluxDBWriter : public Component {
public:
  InfluxDBWriter(){};
  void setup() override;
  void loop() override;
  void dump_config() override;
#ifdef USE_BINARY_SENSOR
  void on_sensor_update(binary_sensor::BinarySensor *obj,
                        std::string measurement, std::string tags, std::string field_key, bool state);
#endif
#ifdef USE_SENSOR
  void on_sensor_update(sensor::Sensor *obj, std::string measurement,
                        std::string tags, std::string field_key, float state);
#endif
#ifdef USE_TEXT_SENSOR
  void on_sensor_update(text_sensor::TextSensor *obj, std::string measurement,
                        std::string tags, std::string field_key, std::string state);
#endif

  void set_host(std::string host) { this->host = host; };
  void set_port(uint16_t port) { this->port = port; };

  void set_orgid(std::string orgid) { this->orgid = orgid; };
  void set_token(std::string token) { this->token = token; };
  void set_bucket(std::string bucket) { this->bucket = bucket; };
  void set_send_timeout(int timeout) { send_timeout = timeout; };
  void set_publish_all(bool all) { publish_all = all; };
  void add_setup_callback(std::function<EntityBase *()> fun) {
    setup_callbacks.push_back(fun);
  };
  void set_https(bool https) {this->https = https; };
  void set_precision(int precision) {this->precision = precision; };

protected:
  void write(std::string measurement, std::string tags, std::string field_key, const std::string value, bool is_string);

  uint16_t port;
  std::string host;

  std::string orgid;
  std::string token;
  std::string bucket;
  std::string service_url;
  std::string field_key;

  int send_timeout;
  std::string tags;
  bool publish_all;
  bool https;
  int precision;

  std::vector<std::function<EntityBase *()>> setup_callbacks;

  http_request::HttpRequestComponent *request_;
};

} // namespace influxdb
} // namespace esphome
