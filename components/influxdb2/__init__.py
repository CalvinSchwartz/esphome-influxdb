import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PORT, CONF_USERNAME, CONF_PASSWORD
from esphome.core import coroutine_with_priority
from esphome.core import CORE

DEPENDENCIES = ['network']
AUTO_LOAD = ['http_request']

influxdb_ns = cg.esphome_ns.namespace('influxdb')
InfluxDBWriter = influxdb_ns.class_(
    'InfluxDBWriter', cg.Component, cg.Controller)

CONF_HOST = 'host'
CONF_ORG_ID = 'orgid'
CONF_TOKEN = 'token'
CONF_BUCKET = 'bucket'
CONF_SEND_TIMEOUT = 'send_timeout'
CONF_TAGS = 'tags'
# CONF_DEVICE = 'device'
CONF_PUBLISH_ALL = 'publish_all'
CONF_SENSORS = 'sensors'
CONF_IGNORE = 'ignore'
CONF_MEASUREMENT = 'measurement'
CONF_HTTPS = 'https'
CONF_PRECISION = 'precision'
CONF_FIELD_KEY = 'field_key'


def _escape_line_protocol(value):
    return (
        value.replace('\\', '\\\\')
        .replace(',', '\\,')
        .replace(' ', '\\ ')
        .replace('=', '\\=')
    )


def _escape_line_protocol_string(value):
    return (
        value.replace('\\', '\\\\')
        .replace('"', '\\"')
        .replace('\n', '\\n')
        .replace('\r', '\\r')
    )


def _format_tags(tag_map):
    return ''.join(
        ',{}={}'.format(_escape_line_protocol(tag), _escape_line_protocol(value))
        for tag, value in tag_map.items()
    )


SENSOR_SCHEMA = cv.Schema({
    cv.validate_id_name:
    cv.Schema({
        cv.Optional(CONF_IGNORE, default=False): cv.boolean,
        cv.Optional(CONF_MEASUREMENT): cv.string,
        cv.Optional(CONF_TAGS, default={}): cv.Schema({
            cv.string: cv.string
        }),
        cv.Optional(CONF_FIELD_KEY, default='value'): cv.string_strict,
    })
})

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(InfluxDBWriter),
    cv.Required(CONF_HOST): cv.domain,
    cv.Optional(CONF_PORT, default=8086): cv.port,
    cv.Required(CONF_ORG_ID): cv.string_strict,
#    cv.Optional(CONF_DEVICE): cv.string_strict,
    cv.Required(CONF_TOKEN): cv.string_strict,
    cv.Required(CONF_BUCKET): cv.string_strict,
    cv.Optional(CONF_SEND_TIMEOUT, default='500ms'): cv.positive_time_period_milliseconds,
    cv.Optional(CONF_PUBLISH_ALL, default=True): cv.boolean,
    cv.Optional(CONF_TAGS, default={'node': CORE.name}): cv.Schema({
        cv.string: cv.string
    }),
    cv.Optional(CONF_SENSORS, default={}): SENSOR_SCHEMA,
    cv.Optional(CONF_HTTPS, default=False): cv.boolean,
    cv.Optional(CONF_PRECISION, default=8): cv.int_,
}).extend(cv.COMPONENT_SCHEMA)


@coroutine_with_priority(40)
def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)

    cg.add(var.set_host(config[CONF_HOST]))
    cg.add(var.set_port(config[CONF_PORT]))
    cg.add(var.set_orgid(config[CONF_ORG_ID]))
    cg.add(var.set_token(config[CONF_TOKEN]))
    cg.add(var.set_bucket(config[CONF_BUCKET]))
    cg.add(var.set_tags(_format_tags(config[CONF_TAGS])))
    cg.add(var.set_send_timeout(config[CONF_SEND_TIMEOUT]))
    cg.add(var.set_publish_all(config[CONF_PUBLISH_ALL]))
    cg.add(var.set_https(config[CONF_HTTPS]))
    cg.add(var.set_precision(config[CONF_PRECISION]))


    for sensor_id, sensor_config in config[CONF_SENSORS].items():
        if sensor_config[CONF_IGNORE] == False:
            tags = _format_tags({**config[CONF_TAGS], **sensor_config[CONF_TAGS]})
            field_key = sensor_config[CONF_FIELD_KEY]
            if 'measurement' in sensor_config:
                measurement = _escape_line_protocol_string(sensor_config[CONF_MEASUREMENT])
                measurement = f'"{measurement}"'
            else:
                measurement = f"{sensor_id}->get_object_id()"

            cg.add(var.add_setup_callback(cg.RawExpression(
                f"[]() -> EntityBase* {{ {sensor_id}->add_on_state_callback([](auto state) {{ {config[CONF_ID]}->on_sensor_update({sensor_id}, {measurement}, \"{tags}\", \"{field_key}\", state); }}); return {sensor_id}; }}")))
        else:
            cg.add(var.add_setup_callback(cg.RawExpression(
                f"[]() -> EntityBase* {{ return {sensor_id}; }}")))

    cg.add_define('USE_INFLUXDB')
    cg.add_global(influxdb_ns.using)
