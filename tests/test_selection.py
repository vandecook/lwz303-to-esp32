"""Selection/configuration contract; no device access or workbook authoring."""
from pathlib import Path
import hashlib
import json
import re
import unittest
import yaml

ROOT=Path(__file__).resolve().parents[1]

class SelectionTest(unittest.TestCase):
    def test_saved_selection_is_fully_configured(self):
        manifest=json.loads((ROOT/'components/lwz2/selection.json').read_text(encoding='utf-8'))
        entities=manifest['entities']
        self.assertEqual(len(entities),80)
        self.assertEqual(sum(not r['optional'] for r in entities),30)
        self.assertEqual(sum(r['optional'] for r in entities),50)
        self.assertEqual(len({r['id'] for r in entities}),80)
        keys=[r['key'] for r in entities if r['kind']!='action']
        self.assertEqual(len(set(keys)),78)
        raw=(ROOT/'lwz303-sol-example.yaml').read_text(encoding='utf-8')
        raw=re.sub(r'!secret\s+\S+', '"validation-only"',raw)
        config=yaml.safe_load(raw)
        configured=config['lwz2']
        migrated={'p01-p12_p07_fan_stage_day':'fan_stage_day',
                  'p01-p12_p08_fan_stage_night':'fan_stage_night'}
        for r in entities:
            if r['kind']=='action':continue
            key=migrated.get(r['key'],r['key'])
            self.assertIn(key,configured,r['id'])
            self.assertEqual(configured[key].get('disabled_by_default',False),False if r['key'] in migrated else r['optional'],r['id'])
        self.assertEqual(configured['hc1_layout'],'2.06')
        self.assertEqual(configured['firmware'],'2.14j')
        self.assertNotIn('solar_pump',configured)
        for key in ['supply_fan_power','exhaust_fan_power','main_fan_power','flow_temperature_hc2','cooling_hours','supply_fan_airflow_set','exhaust_fan_airflow_set']:
            self.assertNotIn(key,configured)
        self.assertEqual(len([r for r in entities if r['kind']=='number']),4)
        self.assertEqual(len([r for r in entities if r['kind']=='action']),2)
        self.assertEqual(config['switch'][0]['restore_mode'],'ALWAYS_OFF')
        for id,key in [('OUT063','s_fan_supply_fan_speed200'),('OUT064','s_fan_exhaust_fan_speed200')]:
            entry=next(r for r in entities if r['id']==id)
            self.assertEqual(entry['choice'],'Optional')
            self.assertTrue(entry['optional'])
            self.assertEqual(configured[key]['accuracy_decimals'],1)
            self.assertTrue(configured[key]['disabled_by_default'])
            self.assertEqual(configured[key]['web_server']['sorting_group_id'],'lwz_expert')
            self.assertNotIn('unit_of_measurement',configured[key])
            self.assertEqual(entry['unit'],'')
        self.assertEqual({r['id'] for r in manifest['selection_overrides']},{'OUT032','OUT063','OUT064'})
        # The approved workbook is an audit source, not needed by firmware or CI.
        workbook=ROOT/'outputs/parameterkatalog-v03'/manifest['source_workbook']
        if workbook.exists():self.assertEqual(hashlib.sha256(workbook.read_bytes()).hexdigest(),manifest['source_sha256'])

    def test_v04_configuration_contract(self):
        for filename in ('lwz303.yaml','lwz303-sol-example.yaml'):
            raw=(ROOT/filename).read_text(encoding='utf-8')
            config=yaml.safe_load(re.sub(r'!secret\s+\S+', '"validation-only"',raw))
            lwz=config['lwz2']
            self.assertNotIn('enable_v03',lwz)
            self.assertNotIn('enable_v04',lwz)
            for key in ('profile_slot','profile_name','profile_gradient','profile_low_end','profile_room_influence','profile_recognized','profile_action_status'):
                self.assertIn(key,lwz)
            self.assertEqual(lwz['fan_stage_diagnostics']['entity_category'],'diagnostic')
            uptime=next(e for e in config['sensor'] if e.get('id')=='lwz_uptime_seconds')
            self.assertTrue(uptime['disabled_by_default'])
            readable=next(e for e in config['text_sensor'] if e.get('id')=='lwz_uptime_readable')
            self.assertIn('lwz_uptime_seconds',readable['lambda'])
            for key,group in (('hc1_gradient','lwz_basic'),('hc1_low_end','lwz_basic'),('hc1_room_influence','lwz_expert')):
                self.assertEqual(lwz[key]['mode'],'slider')
                self.assertEqual(lwz[key]['web_server']['sorting_group_id'],'lwz_parameters')
            for key in ('hc_stage','dhw_stage','hc_stage_text','dhw_stage_text'):
                self.assertEqual(lwz[key]['web_server']['sorting_group_id'],'lwz_basic')
                self.assertNotIn('entity_category',lwz[key])
            for key in ('s_global_booster_stage3','booster_stage2','booster_stage1',
                        's_h_c1_hc_booster_stage','s_h_c1_hc_stage','s_d_h_w_dhw_booster_stage'):
                self.assertEqual(lwz[key]['web_server']['sorting_group_id'],'lwz_expert')
                self.assertEqual(lwz[key]['entity_category'],'diagnostic')
            self.assertEqual(lwz['fan_interval'],'10s')
            profile=next(e for e in config['select'] if e.get('id')=='lwz_firmware')
            self.assertIn('2.16',profile['options'])
            self.assertEqual(config['esphome']['project']['version'],'0.7.0')
            self.assertEqual(config['external_components'][0]['source'],'github://vandecook/lwz303-to-esp32')
            for key in ('exhaust_fault','supply_fault','hc_stage','dhw_stage','clock_mode','fan_stage_day','fan_stage_night'):
                self.assertIn(key,lwz)
            for domain in ('lwz2','sensor','text_sensor','button','switch','select','number'):
                entries=config.get(domain,[])
                if isinstance(entries,dict): entries=entries.values()
                for entry in entries:
                    if isinstance(entry,dict) and 'name' in entry:
                        self.assertNotIn('${device_name}',entry['name'])
                        self.assertNotIn('${friendly_name}',entry['name'])
            buttons=config['button']
            reset=next(b for b in buttons if b.get('id')=='lwz_reset_fan_faults')
            self.assertIn('reset_fan_faults()',reset['on_press'][0]['lambda'])

if __name__=='__main__':unittest.main()
