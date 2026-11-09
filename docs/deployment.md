# Predlagana varna namestitev

Namestitev je bila po izrecni odobritvi izvedena 2026-11-09. Spodnji zapis je
hkrati postopek za prihodnje posodobitve.

## Predlagani obseg spremembe na Home Assistantu

1. Ohraniti obstoječo `/config/esphome/ups-nut-esp32p4.yaml` kot povratno kopijo.
2. Ustvariti samo novo mapo
   `/config/esphome/components/ups_nut_esp32p4/` in vanjo kopirati lokalna
   `ups_hid` ter `nut_server`.
3. Zamenjati samo `/config/esphome/ups-nut-esp32p4.yaml` s pregledanim kandidatom.
4. Ne kopirati lokalnega `secrets.yaml`; uporabiti obstoječa HA ključa
   `nut_username` in `nut_password`.
5. Validirati in kompilirati, OTA pa izvesti samo po ločenem dovoljenju.

Pri izvedeni namestitvi se `esp-nut.yaml`, `esp32s3-poe-eth-ups.yaml` ter skupni
mapi `components/ups_hid` in `components/nut_server` niso spremenili.

Lokalna OTA slika je bila sestavljena z `ups-nut-esp32p4-runtime.yaml`. Obstoječi
HA skrivnosti sta bili podani samo kot procesni okoljski spremenljivki, nato
odstranjeni; njuni vrednosti nista bili zapisani v projekt ali dnevnike.

## Predlagan varen runtime preizkus

- Po posebej odobrenem flashanju preveriti Ethernet, nato USB enumeracijo in
  samo bralne vrednosti v dnevniku.
- Z NUT odjemalcem preveriti `LIST UPS`, `LIST VAR ups-esp32p4` in bralne `GET VAR`
  zahteve po prijavi.
- Ne izvajati `INSTCMD`, testov baterije, izklopa, `load.off`, `shutdown` ali
  drugih upravljalnih ukazov.
- Nato dodati Home Assistant NUT integracijo za
  `ups-esp32p4@DEVICE_IP:3493` in preveriti podprte meritve.

## Povrnitev

Pred flashanjem je povrnitev samo odstranitev nove projektne mape in obnovitev
prejšnjega YAML. Po flashanju bi povrnitev firmware zahtevala posebej odobreno
ponovno namestitev prejšnje delujoče slike; tega ta priprava ni izvedla.
