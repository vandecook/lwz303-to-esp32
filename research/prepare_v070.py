"""Generate the v0.7 parameter catalogue from the bundled, pinned FHEM source."""
from pathlib import Path
import hashlib
import json
import re

ROOT = Path(__file__).resolve().parents[1]
source = ROOT / 'docs/references/fhem/00_THZ.pm'
fhem = source.read_text(encoding='utf-8')
requested = set(range(1, 16)) | {19, 31, 32, 49, 79, 86} | set(range(37, 43)) | set(range(54, 59))
legacy = {1:'heating_day_setpoint', 2:'heating_night_setpoint', 4:'dhw_day_setpoint',
          5:'dhw_night_setpoint', 7:'fan_stage_day', 8:'fan_stage_night',
          13:'hc1_gradient', 14:'hc1_low_end', 15:'hc1_room_influence'}
labels = {
3:'Raum-Sollwert Bereitschaft einstellen', 6:'Warmwasser Bereitschaft einstellen',
9:'Luefterstufe Bereitschaft einstellen', 10:'Heizung Manuell Sollwert',
11:'Warmwasser Manuell Sollwert', 12:'Luefterstufe Manuell',
19:'Vorlaufanteil HK1', 31:'Maximale Zusatzheizstufen Heizung', 32:'Warmwasser Hysterese',
37:'Zuluft Stufe 1 einstellen',38:'Zuluft Stufe 2 einstellen',39:'Zuluft Stufe 3 einstellen',
40:'Abluft Stufe 1 einstellen',41:'Abluft Stufe 2 einstellen',42:'Abluft Stufe 3 einstellen',
49:'Sommerbetrieb Temperatur',54:'Pumpenzyklen Minimum',55:'Pumpenzyklen Maximum',
56:'Aussentemperatur maximale Pumpenzyklen',57:'Aussentemperatur minimale Pumpenzyklen',
58:'Temperaturerfassung nach Pumpenstart sperren',79:'Zusatzheizung Wartezeit Heizung',
16:'Heizkurve HK2 Gradient',17:'Heizkurve HK2 Fusspunkt',18:'Heizkurve HK2 Raumeinfluss',
20:'Vorlaufanteil HK2',21:'Hysterese Heizstufe 1',22:'Hysterese Heizstufe 2',
23:'Hysterese Heizstufe 3',24:'Hysterese Heizstufe 4',25:'Hysterese Heizstufe 5 (SOL)',
26:'Weitere Hysterese 6',27:'Weitere Hysterese 7',28:'Weitere Hysterese 8',
29:'Hysterese Asymmetrie',30:'Heizungsregelung Integralanteil',
33:'Zusatzheizung Wartezeit Warmwasser',34:'Warmwasser Zusatzheizung Temperaturgrenze',
35:'Legionellenprogramm Intervall',36:'Warmwasser maximale Ladedauer',
43:'Lueftung Stufe 3 Zeitvorgabe',44:'Lueftung Stufe 2 Zeitvorgabe',
45:'Lueftung Stufe 1 Zeitvorgabe',46:'Lueftung Stufe 0 Zeitvorgabe',
47:'Verdichter Wiederanlaufverzoegerung',48:'Hauptluefter Vorgabe',50:'Sommerbetrieb Hysterese',
59:'Wiederanlauf vor Absenkende',70:'Trockenheizen Startzustand',71:'Trockenheizen Sockeltemperatur',
72:'Trockenheizen Maximaltemperatur',73:'Trockenheizen Temperaturdauer',74:'Trockenheizen Temperaturanstieg',
75:'Passive Kuehlung Vorgabe',77:'Aussentemperatur Filterzeit',78:'Bivalenzpunkt',
80:'Solar Freigabe',81:'Solar Ladung Temperaturdifferenz',82:'Solar Verdichterstart Verzoegerung',
84:'Warmwasserpuffer Freigabe',
}
rows = []
for block in ('01','03','05','06','07','08','0A','0E','10','17'):
    match = re.search(r'"'+block+r'pxx206"\s*=>\s*\[(.*?)\n\s*\],?', fhem, re.S)
    assert match, block
    fields = re.findall(r'\["\s*([^":]+):\s*",\s*(\d+),\s*(\d+),\s*"(\w+)",\s*(\d+)\]', match[1])
    assert fields, block
    minimum = max((int(pos)-4+int(length))//2 for _,pos,length,_,_ in fields)
    for name,pos,length,mode,divisor in fields:
        p = re.match(r'p(\d+)',name)
        if not p:
            continue
        n = int(p[1])
        # Duplicate P84 definitions belong to different source blocks; retain both explicitly.
        identifier = 184 if n == 84 and block == '08' else n
        limits = re.search(r'"'+name+r'"\s*=>\s*\{parent=>[^\n]+?argMin\s*=>\s*"(-?\d+)"\s*,\s*argMax\s*=>\s*"(-?\d+)"[^\n]+?unit\s*=>\s*"([^"\n]*)"', fhem)
        writable = n in requested and n != 86
        assert limits or not writable or n == 31, name
        unit = limits[3].strip() if limits else ''
        if n == 31: lower,upper = 0,3  # FHEM p31MaxBoosterStgHtg; conservative 0..3 stage count.
        elif limits: lower,upper = int(limits[1]),int(limits[2])
        else: lower,upper = 0,0
        if n in (37,38,39,40,41,42): unit='m³/h'
        step = 1/int(divisor)
        row=dict(id=identifier,number=n,key=legacy.get(n,f'parameter_p{n:02d}'+('_solar' if identifier==184 else '')),
                 label='Solar Warmwassertemperatur' if identifier==184 else labels.get(n,name),source=name,block=block,offset=(int(pos)-4)//2,
                 length=int(length)//2,divisor=int(divisor),signed=mode=='hex2int',
                 minimum=minimum,minimum_214=15 if block=='01' else minimum,
                 writable=writable,legacy=n in legacy,min=lower,max=upper,step=step,unit=unit,
                 sol_only=block=='08' or n==25)
        rows.append(row)
rows.sort(key=lambda r:r['id'])
assert len({r['id'] for r in rows})==len(rows)
assert {r['number'] for r in rows if r['writable']}==requested-{86}
manifest=dict(version='0.7.0',source='docs/references/fhem/00_THZ.pm',
              source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),parameters=rows,
              unavailable=[dict(number=86,reason='Kein belegter 2.x-Zugriff; FHEM 0A05A3 gehoert zur neueren Registerfamilie.')])
(ROOT/'components/lwz2/parameters.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
lines=['// Generated by research/prepare_v070.py from the pinned FHEM 2.x reference.',
       '#pragma once','#include <cstdint>','namespace esphome::lwz2 {',
       'struct ParameterRule { uint8_t id, command, offset, length, divisor, minimum, minimum_214; bool is_signed, writable, sol_only; float lower, upper, step; };',
       'static constexpr ParameterRule PARAMETER_RULES[]{']
for r in rows:
    lines.append('  {'+', '.join([str(r['id']),'0x'+r['block'],str(r['offset']),str(r['length']),str(r['divisor']),str(r['minimum']),str(r['minimum_214']),str(r['signed']).lower(),str(r['writable']).lower(),str(r['sol_only']).lower(),str(float(r['min']))+'f',str(float(r['max']))+'f',str(float(r['step']))+'f'])+'},')
lines+=['};','}  // namespace esphome::lwz2','']
(ROOT/'components/lwz2/parameters.h').write_text('\n'.join(lines),encoding='utf-8')
print(f'Generated {len(rows)} source-backed parameter definitions; 31 writable, P86 unavailable.')
