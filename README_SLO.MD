# UPS NUT ESP32-P4

[English version](README.md)

Lokalno pripravljen ESPHome projekt za ESP32-P4 z Ethernetom IP101, USB HID UPS
odjemalcem in NUT strežnikom na TCP 3493. Projekt uporablja popravljeni lokalni
kopiji komponent `ups_hid` in `nut_server`; s tem ne posega v skupni komponenti,
ki ju uporabljajo druge ESPHome naprave.

## Vsebina

- `ups-nut-esp32p4.yaml` — kandidat za napravo.
- `ups-nut-esp32p4-runtime.yaml` — lokalni ovoj za varno kompilacijo z NUT
  poverili samo iz okoljskih spremenljivk.
- `components/ups_nut_esp32p4/` — projektno izolirani komponenti.
- `backup/ha-current/` — nespremenjena lokalna kopija stanja s Home Assistanta.
- `upstream/esphome-components/` — pregledana kopija upstream repozitorija.
- `docs/` — izhodišče, spremembe, testiranje in varen postopek namestitve.

Prave vrednosti `nut_username` in `nut_password` ostanejo v Home Assistantovem
`secrets.yaml`. Lokalni `secrets.yaml` vsebuje samo sintetični vrednosti za
validacijo in je izključen iz različic.

## Lokalna validacija

```powershell
.\.venv\Scripts\esphome.exe config .\ups-nut-esp32p4.yaml
.\.venv\Scripts\esphome.exe compile .\ups-nut-esp32p4.yaml
```

Kompilacija ne pomeni, da so USB enumeracija, HID podatki ali NUT delovanje na
dejanski napravi vedno preverjeni za vsak model. APC Back-UPS RS 650MI je bil
2026-11-09 uspešno preverjen prek USB HID, ESPHome API in samo bralnega NUT
protokola. Noben UPS upravljalni ukaz ni bil izveden.
