import string
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text
from esphome.const import CONF_INITIAL_VALUE, CONF_RESTORE_VALUE
from . import nut_server_ns, NutServerComponent

DEPENDENCIES = ["nut_server"]
CONF_NUT_SERVER_ID = "nut_server_id"
NutUpsNameText = nut_server_ns.class_("NutUpsNameText", text.Text, cg.Component)

def ups_name(value):
    value = cv.string_strict(value)
    if not 1 <= len(value) <= 32 or any(c not in string.ascii_letters + string.digits + "_.-" for c in value):
        raise cv.Invalid("UPS name must be 1-32 letters, digits, dots, underscores or hyphens")
    return value

CONFIG_SCHEMA = text.text_schema(NutUpsNameText).extend({
    cv.GenerateID(CONF_NUT_SERVER_ID): cv.use_id(NutServerComponent),
    cv.Optional(CONF_INITIAL_VALUE, default="ups"): ups_name,
    cv.Optional(CONF_RESTORE_VALUE, default=True): cv.boolean,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = await text.new_text(config, min_length=1, max_length=32, pattern=r"[A-Za-z0-9_.-]+")
    await cg.register_component(var, config)
    parent = await cg.get_variable(config[CONF_NUT_SERVER_ID])
    cg.add(var.set_parent(parent))
    cg.add(var.set_initial_value(config[CONF_INITIAL_VALUE]))
    cg.add(var.set_restore_value(config[CONF_RESTORE_VALUE]))
