#include "influxdb_writer.h"
#include "esphome/core/application.h"
#include "esphome/core/log.h"
#include <algorithm>
#include <string>
#include <iomanip>
#include <sstream>
#include "esphome/components/http_request/http_request.h"
//#include "esphome/components/http_request/http_request_idf.h" // ESP32 IDF
#include "esphome/components/http_request/http_request_arduino.h"

#ifdef USE_LOGGER
#include "esphome/components/logger/logger.h"
#endif

namespace esphome {
namespace influxdb {
static const char *TAG = "influxdb_jab";

void InfluxDBWriter::setup() {
  ESP_LOGCONFIG(TAG, "Setting up InfluxDB Writer...");
  std::vector<EntityBase *> objs;
  for (auto fun : setup_callbacks)
    objs.push_back(fun());

  if(this->https) {
    this->service_url = "https://" + this->host + "/api/v2/write?org=" + this->orgid + "&bucket=" + this->bucket + "&precision=ns";
  } else {
  this->service_url = "http://" + this->host + ":" + to_string(this->port) +
                      "/api/v2/write?org=" + this->orgid + "&bucket=" + this->bucket + "&precision=ns";
  }

  this->request_ = new http_request::HttpRequestArduino();

  this->request_->set_useragent("ESPHome InfluxDB Bot");
  this->request_->set_timeout(this->send_timeout);

  if (publish_all) {
#ifdef USE_BINARY_SENSOR
    for (auto *obj : App.get_binary_sensors()) {
      if (!obj->is_internal() &&
          std::none_of(objs.begin(), objs.end(),
                       [&obj](EntityBase *o) { return o == obj; }))
        obj->add_on_state_callback([this, obj](bool state) {
          this->on_sensor_update(obj, obj->get_object_id(), tags, field_key, state);
        });
    }
#endif
#ifdef USE_SENSOR
    for (auto *obj : App.get_sensors()) {
      if (!obj->is_internal() &&
          std::none_of(objs.begin(), objs.end(),
                       [&obj](EntityBase *o) { return o == obj; }))
        obj->add_on_state_callback([this, obj](float state) {
          this->on_sensor_update(obj, obj->get_object_id(), tags, field_key, state);
        });
    }
#endif
#ifdef USE_TEXT_SENSOR
    for (auto *obj : App.get_text_sensors()) {
      if (!obj->is_internal() &&
          std::none_of(objs.begin(), objs.end(),
                       [&obj](EntityBase *o) { return o == obj; }))
        obj->add_on_state_callback([this, obj](std::string state) {
          this->on_sensor_update(obj, obj->get_object_id(), tags, field_key, state);
        });
    }
#endif
  }
}

void InfluxDBWriter::loop() {}

void InfluxDBWriter::write(std::string measurement,
                           std::string tags,
                           const std::string field_key,
                           const std::string value,
                           const bool is_string) {
  std::replace(measurement.begin(), measurement.end(), '-', '_');
  for (size_t i = 0; i < tags.length(); ++i){ // Add the escape char "\" to all whitespaces in the tags with an "\ "
    if (tags[i] == ' ') {
      tags.insert(i, "\\");
      i++; // Skip the inserted backslash
    }
  }
  std::string line =
      measurement + tags + " " + field_key + "=" + (is_string ? ("\"" + value + "\"") : value);

  std::list<http_request::Header> headers;
  http_request::Header header;
  header.name = "Content-Type";
  header.value = "text/plain";
  headers.push_back(header);
  if ((this->orgid.length() > 0) && (this->token.length() > 0)) {
    header.name = "Authorization";
    header.value = this->token.c_str();
    headers.push_back(header);
  }

  this->request_->post(this->service_url, line.c_str(), headers);

  ESP_LOGD(TAG, "InfluxDB packet: %s", line.c_str());
}

void InfluxDBWriter::dump_config() {
  ESP_LOGCONFIG(TAG, "InfluxDB Writer:");
  ESP_LOGCONFIG(TAG, "  Address: %s:%u", host.c_str(), port);
  ESP_LOGCONFIG(TAG, "  Bucket: %s", bucket.c_str());
}

#ifdef USE_BINARY_SENSOR
void InfluxDBWriter::on_sensor_update(binary_sensor::BinarySensor *obj,
                                      std::string measurement, std::string tags, std::string field_key, bool state) {
  write(measurement, tags, field_key0 state ? "t" : "f", false);
}
#endif

#ifdef USE_SENSOR
void InfluxDBWriter::on_sensor_update(sensor::Sensor *obj,
                                      std::string measurement, std::string tags, std::string field_key, float state) {
  if (!isnan(state)){
    std::stringstream  value;
    value << std::fixed << std::setprecision(this->precision) << state;
    write(measurement, tags, field_key, value.str(), false);
  }
}
#endif

#ifdef USE_TEXT_SENSOR
void InfluxDBWriter::on_sensor_update(text_sensor::TextSensor *obj,
                                      std::string measurement, std::string tags, std::string field_key,
                                      std::string state) {
  write(measurement, tags, field_key, state, true);
}
#endif

} // namespace influxdb
} // namespace esphome
