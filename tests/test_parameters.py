"""Parameter provenance, configuration completeness, and compatibility with existing HA entities."""
from pathlib import Path
import hashlib
import json
import re
import unittest
import yaml

ROOT=Path(__file__).resolve().parents[1]

def config(name):
    return yaml.safe_load(re.sub(r'!secret\s+\S+', '"validation-only"',(ROOT/name).read_text(encoding='utf-8')))

def locate(cfg,path):
    for key,index in re.findall(r'([^.[\]]+)|\[(\d+)\]',path):
        cfg=cfg[int(index)] if index else cfg[key]
    return cfg

class ParameterTest(unittest.TestCase):
    def test_existing_entity_identity_is_unchanged(self):
        baseline=json.loads((ROOT/'tests/fixtures/entities-v0.6.2.json').read_text(encoding='utf-8'))
        for name,snapshot in baseline.items():
            cfg=config(name)
            self.assertEqual({k:cfg['esphome'][k] for k in snapshot['device']},snapshot['device'])
            for path,attributes in snapshot['entities'].items():
                current=locate(cfg,path)
                self.assertEqual({k:current[k] for k in attributes},attributes,(name,path))

    def test_requested_parameters_and_reference(self):
        catalogue=json.loads((ROOT/'components/lwz2/parameters.json').read_text(encoding='utf-8'))
        self.assertEqual(hashlib.sha256((ROOT/catalogue['source']).read_bytes()).hexdigest(),catalogue['source_sha256'])
        rows=catalogue['parameters']
        self.assertEqual(len(rows),70)
        self.assertEqual(len({r['id'] for r in rows}),70)
        requested=set(range(1,16))|{19,31,32,49,79}|set(range(37,43))|set(range(54,59))
        self.assertEqual({r['number'] for r in rows if r['writable']},requested)
        self.assertEqual([r['number'] for r in catalogue['unavailable']],[86])
        # Independent checkpoints from FHEM's hex-character positions (payload starts at 4).
        checkpoints={3:('17',4,2,10),6:('17',10,2,10),9:('17',14,1,1),
                     10:('17',15,2,10),11:('17',17,2,10),12:('17',19,1,1),
                     19:('05',10,2,1),31:('06',11,1,1),32:('07',0,1,10),
                     37:('01',0,2,1),42:('01',10,2,1),49:('06',14,2,10),
                     54:('0A',0,1,1),55:('0A',1,2,1),56:('0A',3,2,10),
                     57:('0A',5,2,10),58:('0A',7,2,1),79:('06',22,1,1)}
        for identifier,expected in checkpoints.items():
            r=next(r for r in rows if r['id']==identifier)
            self.assertEqual(tuple(r[k] for k in ('block','offset','length','divisor')),expected)
        for filename in ('lwz303.yaml','lwz303-sol-example.yaml'):
            cfg=config(filename); component=cfg['lwz2']
            self.assertTrue(component['parameter_overview'])
            self.assertIn('parameters_valid',component)
            for r in rows:
                self.assertIn(r['key'],component)
                self.assertEqual(component[r['key']]['web_server']['sorting_group_id'],'lwz_parameters')
            self.assertNotIn('parameter_p86',component)
            self.assertNotIn('cooling_hours',component)
            names=[entry['name'] for entry in component.values() if isinstance(entry,dict) and 'name' in entry]
            self.assertEqual(len(names),len(set(names)))
            self.assertEqual(cfg['esphome']['project']['version'],'0.7.0')

if __name__=='__main__': unittest.main()
