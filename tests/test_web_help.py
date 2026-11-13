"""Regression: external-component assets must not depend on /config/web."""
import importlib.util
from pathlib import Path
import unittest
import esphome.final_validate as fv
import esphome.config_validation as cv

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("lwz_web_help", ROOT/'components/lwz2/web_help.py')
helper = importlib.util.module_from_spec(spec)
spec.loader.exec_module(helper)


class FullConfig(dict):
    def get_config_for_path(self, path):
        value = self
        for key in path:
            value = value[key]
        return value


class WebHelpTest(unittest.TestCase):
    def test_component_local_asset_is_injected(self):
        web = {"version": 3, "local": True, "js_url": "https://oi.esphome.io/v3/www.js"}
        token = fv.full_config.set(FullConfig(web_server=web))
        try:
            helper.validate_web_help({"web_help": True})
        finally:
            fv.full_config.reset(token)
        asset = Path(web['js_include'])
        self.assertEqual(asset.parent, ROOT/'components/lwz2')
        self.assertTrue(asset.is_file())
        self.assertEqual(web['js_url'], '')
        self.assertFalse(web['local'])

    def test_disabled_does_not_require_web_server(self):
        self.assertEqual(helper.validate_web_help({"web_help": False}), {"web_help": False})

    def test_missing_or_old_web_server_is_rejected(self):
        for config in (FullConfig(), FullConfig(web_server={"version": 2})):
            token = fv.full_config.set(config)
            try:
                with self.assertRaises(cv.Invalid):
                    helper.validate_web_help({"web_help": True})
            finally:
                fv.full_config.reset(token)

    def test_custom_javascript_is_not_silently_replaced(self):
        web = {"version": 3, "js_include": "custom.js"}
        token = fv.full_config.set(FullConfig(web_server=web))
        try:
            with self.assertRaises(cv.Invalid):
                helper.validate_web_help({"web_help": True})
        finally:
            fv.full_config.reset(token)
        self.assertEqual(web['js_include'], 'custom.js')


if __name__ == '__main__':
    unittest.main()
