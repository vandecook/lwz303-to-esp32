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
        self.assertEqual(sum(not r['optional'] for r in entities),33)
        self.assertEqual(sum(r['optional'] for r in entities),47)
        self.assertEqual(len({r['id'] for r in entities}),80)
        keys=[r['key'] for r in entities if r['kind']!='action']
        self.assertEqual(len(set(keys)),78)
        raw=(ROOT/'lwz303-sol-example.yaml').read_text(encoding='utf-8')
        raw=re.sub(r'!secret\s+\S+', '"validation-only"',raw)
        config=yaml.safe_load(raw)
        configured=config['lwz2']
        for r in entities:
            if r['kind']=='action':continue
            self.assertIn(r['key'],configured,r['id'])
            self.assertEqual(configured[r['key']].get('disabled_by_default',False),r['optional'],r['id'])
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
            self.assertEqual(entry['choice'],'Ja')
            self.assertFalse(entry['optional'])
            self.assertEqual(configured[key]['accuracy_decimals'],1)
            self.assertNotIn('unit_of_measurement',configured[key])
            self.assertEqual(entry['unit'],'')
        self.assertEqual({r['id'] for r in manifest['selection_overrides']},{'OUT063','OUT064'})
        # The approved workbook is an audit source, not needed by firmware or CI.
        workbook=ROOT/'outputs/parameterkatalog-v03'/manifest['source_workbook']
        if workbook.exists():self.assertEqual(hashlib.sha256(workbook.read_bytes()).hexdigest(),manifest['source_sha256'])

if __name__=='__main__':unittest.main()
