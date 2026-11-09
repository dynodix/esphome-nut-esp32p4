# Preverjanje

## Opravljeno

| Preverjanje | Rezultat | Opomba |
|---|---|---|
| SSH brez gesla | PASS | Oddaljeni uporabnik `hassio` |
| Lokalna YAML validacija | PASS | ESPHome 2026.10.2 |
| Čista ESP32-P4 kompilacija | PASS | ESP-IDF 5.5.6, GCC 14.2.0 |
| RAM | PASS | 80.134 / 576.464 B (13,9 %) |
| Aplikacijska slika | PASS | 757.108 / 8.126.464 B (9,3 %) |
| USB HID na fizičnem UPS-u | PASS | APC Back-UPS RS 650MI, VID `0x051D` |
| NUT TCP protokol | PASS | Avtenticirana `LIST UPS` in `LIST VAR` na 3493 |
| ESPHome API | PASS | Handshake, dnevniki in objava entitet na 6053 |
| Home Assistant NUT integracija | NOT TESTED | Integracija še ni bila dodana v HA UI |

Končni firmware je bil 2026-11-09 uspešno nameščen z OTA na testni `DEVICE_IP`.
Pred spremembo je bila ustvarjena povratna kopija v
`/config/esphome/backups/ups-nut-esp32p4-before-ota-20261109`.

Preverjene samo bralne vrednosti: proizvajalec American Power Conversion, model
Back-UPS RS 650MI, firmware 939.g4, stanje `OL`, baterija 100 %, runtime 2151 s,
vhod 236 V/50 Hz, obremenitev 6 % in baterijska napetost 13,5 V. UPS ne poda
zanesljive izhodne napetosti; ta spremenljivka je zato pravilno izpuščena.

## Omejitve

- Fizična USB enumeracija in stabilne 10-sekundne meritve so bile potrjene na
  priključenem APC UPS-u, ne pa na drugih modelih.
- `online`/`on_battery` implementacija sklepa iz veljavnosti vhodne napetosti;
  dejanski prehod na baterijo ni bil sprožen, ker bi bil bolj motilen test.
- Ločeni entiteti za USB povezavo in svežino zadnje komunikacije v trenutnem API
  komponente nista implementirani. NUT stanje in dnevniki ju lahko pomagajo
  diagnosticirati, vendar tega ne smemo predstavljati kot ločen senzor zdravja.
- Ostajajo nenevarna opozorila za neznano APC testno vrednost 134 in kratko
  poročilo nominalne vhodne napetosti. Noben test ali SET_REPORT ni bil sprožen.
