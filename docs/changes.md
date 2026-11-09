# Izvedene lokalne spremembe

## YAML

- Vir zunanjih komponent je preusmerjen na projektno izolirano mapo
  `components/ups_nut_esp32p4`.
- Odstranjen je ESPHome `usb_host:`; lastnik USB Host sklada je samo `ups_hid`.
- Omogočen je `ups_hid` z avtomatskim protokolom, 10-sekundnim osveževanjem in
  15-sekundnim časom za zaznavo protokola.
- Omogočen je avtenticiran `nut_server` na TCP 3493 za največ štiri odjemalce.
- Dodane so podprte API entitete za baterijo, obremenitev, napetosti, stanje,
  proizvajalca, model in protokol.
- `online` je poimenovan `UPS mains online`; komponenta ne izpostavlja ločenega,
  zanesljivega senzorja USB povezave ali časa zadnje uspešne komunikacije.
- Niso dodani gumbi, stikala ali druge upravljalne entitete UPS-a.

## Komponenti

- `ups_hid/__init__.py`: dodani manjkajoči odvisnosti `button` in `number`.
- `ups_hid/transport_esp32.cpp`: varna ničelna inicializacija USB Host
  konfiguracije in konstruktorja; združljivo z ESP-IDF 5.5.6 in ESP32-P4.
- `ups_hid/sensor.py` in `binary_sensor.py`: odstranjeni setterji, ki jih ESPHome
  2026 nima več; metapodatki so zdaj deklarirani v YAML.
- `nut_server/nut_server.cpp`: klic globalnega `::socket()` odpravi konflikt z
  imenskim prostorom ESPHome `socket`.
- `ups_hid/protocol_factory.cpp`: vgrajeni APC, CyberPower in generični protokol
  so registrirani z eksplicitnimi referencami. S tem jih linker ne zavrže iz
  statičnega arhiva; napaka je bila odkrita na APC UPS-u z VID `0x051D`.
- APC USB nizi uporabljajo standardne indekse proizvajalec 1, izdelek 2 in
  serijska številka 3. Report `0x09` z vrednostjo okoli 13,5 V je baterijska
  napetost in se ne predstavlja več kot lažna izhodna napetost 135 V.
- Odpravljena so opozorila o formatih, primerjavah brez predznaka in inicializaciji
  netrivialnega objekta.

Popravki so ločeni od `/config/esphome/components/ups_hid` in `nut_server`, zato
referenčni ESP32-S3 napravi nista spremenjeni.
