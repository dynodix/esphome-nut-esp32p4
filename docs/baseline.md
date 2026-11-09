# Izhodišče in diagnostika

Datum pregleda: 2026-11-09.

## Okolje

- SSH: `ssh -o BatchMode=yes homeassistant "whoami"` je vrnil `hassio`.
- ESPHome konfiguracije so v `/config/esphome`.
- Pregledani so bili `ups-nut-esp32p4.yaml`, `esp-nut.yaml` in
  `esp32s3-poe-eth-ups.yaml` ter lokalne komponente v
  `/config/esphome/components`.
- SSH dodatek Home Assistanta je v zaščitenem načinu. Ukazi `ha apps list/info`
  nimajo avtorizacijskega žetona, ESPHome CLI pa v dodatku ni nameščen, zato
  natančne različice nameščenega HA dodatka po tej poti ni bilo mogoče zanesljivo
  ugotoviti.
- Za izolirano lokalno preverjanje je bil nameščen ESPHome 2026.10.2. Med gradnjo
  je razrešil ESP-IDF 5.5.6 in GCC 14.2.0.

## Varnostna kopija

`backup/ha-current` vsebuje nespremenjene kopije treh YAML datotek in komponent
`ups_hid` ter `nut_server`. SHA-256 izvora in lokalnih kopij je bil po prenosu
primerjan. Pravi `secrets.yaml` ni bil kopiran ali izpisan.

## Ugotovljene težave

1. Glavni YAML je vključeval ESPHome `usb_host:`, komponenta `ups_hid` pa sama
   kliče `usb_host_install()`. Obe implementaciji bi upravljali isti ESP-IDF USB
   Host in povzročili dvojno inicializacijo.
2. `ups_hid` in `nut_server` sta bila v glavnem YAML zakomentirana, zato naprava
   ni spremljala UPS-a in ni poslušala na TCP 3493.
3. Upstream komponenta je dokumentirana predvsem za ESP32-S3. ESP32-P4 sicer ima
   podprt USB Host, vendar je bila potrebna prilagoditev inicializacije strukture
   `usb_host_config_t` za aktualni ESP-IDF.
4. Upstream revizija `39fe49e0ec2b0bc3aea7a02a15e7da395624bbc1`
   (2025-09-08) se z ESPHome 2026.10.2 ni prevedla brez popravkov: manjkajoča
   samodejna odvisnost `button`/`number`, konflikt imena `socket()` in odstranjeni
   runtime setterji metapodatkov entitet.
5. Upstream `online` binarni senzor pomeni prisotnost vhodne napetosti, ne USB
   transportne povezave. Zato ga ni dovoljeno predstaviti kot `connectivity`.

Ta dokument opisuje stanje pred namestitvijo. Poznejša odobrena namestitev je
spremenila samo glavni P4 YAML in dodala izolirano mapo komponent; glej
`deployment.md` in `testing.md`.
