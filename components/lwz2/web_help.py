"""Resolve the web UI from the external component, never from the user's config dir."""
from pathlib import Path
import esphome.config_validation as cv
import esphome.final_validate as fv


def validate_web_help(config):
    if not config.get("web_help", False):
        return config
    try:
        web = fv.full_config.get().get_config_for_path(["web_server"])
    except KeyError as err:
        raise cv.Invalid("lwz2.web_help benoetigt web_server mit version: 3") from err
    if web.get("version") != 3:
        raise cv.Invalid("lwz2.web_help benoetigt web_server mit version: 3")
    asset = Path(__file__).with_name("web_help.js").resolve()
    if not asset.is_file():
        raise cv.Invalid("Webserver-Hilfe fehlt in der lwz2-Komponente; vollstaendigen Komponentenstand laden")
    if web.get("js_include") and Path(web["js_include"]).resolve() != asset:
        raise cv.Invalid("Bei lwz2.web_help: true den manuellen web_server.js_include-Eintrag entfernen")
    web["js_include"] = str(asset)
    web["js_url"] = ""
    web["local"] = False
    return config
