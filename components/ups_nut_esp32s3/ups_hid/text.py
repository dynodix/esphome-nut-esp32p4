import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text
from . import ups_hid_ns, UpsHidComponent, CONF_UPS_HID_ID

DEPENDENCIES = ["ups_hid"]
UpsBatteryDateText = ups_hid_ns.class_("UpsBatteryDateText", text.Text, cg.Component)
CONFIG_SCHEMA = text.text_schema(UpsBatteryDateText).extend({
    cv.GenerateID(CONF_UPS_HID_ID): cv.use_id(UpsHidComponent),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = await text.new_text(config, min_length=10, max_length=10, pattern=r"[0-9]{4}-[0-9]{2}-[0-9]{2}")
    await cg.register_component(var, config)
    parent = await cg.get_variable(config[CONF_UPS_HID_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.register_battery_date_text(var))
