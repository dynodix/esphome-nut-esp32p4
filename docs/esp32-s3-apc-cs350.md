# ESP32-S3 PoE Ethernet UPS z APC Back-UPS CS 350

Preverjena kombinacija: ESP32-S3, W5500 Ethernet prek SPI in APC Back-UPS CS 350,
firmware `807.q10 .I USB FW:q10`. Datum preverjanja: 2026-11-25.
[Primerjava s P4](p4-vs-s3.md) pojasnjuje razlike v strojni opremi in USB poročilih.

## Namestitev

V mapo konfiguracij ESPHome kopiraj:

- [ups-nut-esp32s3.yaml](../ups-nut-esp32s3.yaml);
- celotno mapo [components/ups_nut_esp32s3](../components/ups_nut_esp32s3).

Primer uporablja ime vozlišča `ups-nut-esp32s3`, prikazno ime `UPS NUT ESP32-S3`
in NUT ime `racktest`. Imeni vozlišča in prikaza prilagodi pred prevajanjem.
Objavljeni primer je posplošitev preizkušene konfiguracije; izvorna koda komponent
je enaka preizkušeni, z dopolnjenima README datotekama. Ne vsebuje naslovov ali
poverilnic delujoče namestitve.

Ethernet pini preverjene naprave so MOSI GPIO11, MISO GPIO12, SCLK GPIO13,
CS GPIO14, INT GPIO10 in RESET GPIO9. Točen proizvajalec/revizija plošče ni
neodvisno potrjen; pred uporabo na drugi plošči preveri njen pinout.
YAML uporablja `esp32-s3-devkitc-1` in ESP-IDF. UPS priključi na izvorni USB
host priključek plošče. Konzola je `UART0`, zato ne zasede izvornega USB.

`ups_hid` sam namesti ESP-IDF USB Host. Ne dodajaj še ESPHome `usb_host:`, ker
bi dva sklada inicializirala isti gostitelj. Lokalne komponente ohrani skupaj
z YAML; komponente P4 imajo drugačen razpored pričakovanih poročil.

Pripravi `secrets.yaml` po [primeru](../secrets.example.yaml) z vrednostma
`nut_username` in `nut_password`. Sklici na skrivnosti se razrešijo ob prevajanju
tudi, ko je prijava izključena. Skrivnosti so izključene iz Git.

```powershell
uv run --python 3.14 --with esphome==2026.11.1 esphome config ups-nut-esp32s3.yaml
uv run --python 3.14 --with esphome==2026.11.1 esphome compile ups-nut-esp32s3.yaml
```

Za poverilnice iz okolja uporabi [ovojnico](../ups-nut-esp32s3-runtime.yaml):
nastavi `UPS_NUT_RUNTIME_USERNAME` in `UPS_NUT_RUNTIME_PASSWORD` ter kot ciljni
YAML podaj `ups-nut-esp32s3-runtime.yaml`. Osnovni YAML še vedno zahteva veljavne
sklice `!secret`. Ovojnica sama ne vključi prijave; to določa spodnje stikalo.

Pred OTA shrani obstoječi YAML in njegove komponente ter preveri ciljno vozlišče.
Primer nima statičnega IP; naslov dobi prek DHCP. OTA uporablja TCP 3232.
Objavljeni primer ima enako nastavitev kot preverjena naprava: brez šifriranja
API ESPHome in brez gesla OTA. Stikalo prijave NUT velja samo za TCP 3493;
ne spremeni dostopa prek API 6053 ali OTA 3232.

## Nastavitve med delovanjem

V integraciji ESPHome odpri konfiguracijske entitete naprave:

| Nastavitev | Delovanje |
|---|---|
| `NUT UPS name` | Privzeto `racktest`. Dovoljeno 1–32 ASCII črk, številk ali `_ . -`. Neveljaven vnos ne prepiše zadnjega veljavnega imena. |
| `NUT authentication` | Privzeto izključena: branje brez uporabniškega imena in gesla. Po vklopu zahteva pripravljene poverilnice. |
| `Set battery replacement date` | Datum `YYYY-MM-DD` za zapis v UPS; podrobnosti spodaj. |
| `Restart monitor` | Ponovni zagon ESP; ne izvede ponovnega zagona ali izklopa UPS. |

Ime in stikalo prijave se ohranita prek ponovnega zagona. Hramba nastavitev ima
interval 10 sekund, zato po spremembi počakaj pred izklopom napajanja ESP.
Sprememba imena NUT ne preimenuje vozlišča ESPHome ali imen entitet, ki so bila
ustvarjena iz substitucij ob prevajanju. Odjemalce NUT prilagodi novemu imenu.

NUT: gostitelj `DEVICE_IP`, vrata `3493`, UPS `racktest` oziroma trenutno ime.
Pri izključeni prijavi pusti uporabniško ime/geslo prazna. Strežnik ostaja
`read_only: true` tudi po vklopu prijave: `SET VAR`, `INSTCMD` in `FSD` so
zavrnjeni, `LIST CMD` ne ponuja krmilnih ukazov.

Za odjemalec `upsc` na sistemu z nameščenim NUT:

```sh
upsc -l DEVICE_IP:3493
upsc racktest@DEVICE_IP:3493
upsc racktest@DEVICE_IP:3493 battery.date
upsc racktest@DEVICE_IP:3493 battery.mfr.date
upsc racktest@DEVICE_IP:3493 ups.test.result
```

`DEVICE_IP` nadomesti z naslovom ESP. `upsc` je primer za branje pri izključeni
prijavi; delovanje posameznega odjemalca s prijavo preveri v njegovi dokumentaciji.

## Podatki za vzdrževalce

| NUT spremenljivka | ESPHome entiteta | Pomen in preverjena vrednost |
|---|---|---|
| `battery.date` | `Battery date` | Datum menjave, shranjen v UPS: `2015-12-09` |
| `battery.mfr.date` | `Battery manufacturing date` | Datum izdelave iz UPS: `2015-12-09` |
| `ups.test.result` | `Last self test` | Rezultat: `No test initiated` |

Datuma sta odčitka iz UPS, ne neodvisna potrditev dejanske izvedbe vzdrževanja.
`Last self test` kljub imenu vsebuje rezultat, **ne datuma ali ure testa**.
Ta UPS po uporabljeni HID poti ne ponuja časovnega žiga zadnjega samotesta.
Za prikaz v HA preveri tudi onemogočene diagnostične entitete. Nekatere
diagnostične entitete NUT so privzeto onemogočene; razpoložljivost je odvisna
tudi od različice integracije. Za vse tri podatke so neposredne ESPHome entitete.

## Zapis datuma menjave v UPS

Po dejanski menjavi v `Set battery replacement date` vpiši njen datum
`YYYY-MM-DD`. Druga možnost je akcija ESPHome z nizovnim parametrom `date`.
V Developer Tools → Actions izberi akcijo svojega vozlišča. Za objavljeni primer:

```yaml
action: esphome.ups_nut_esp32s3_set_battery_replacement_date
data:
  date: "YYYY-MM-DD"  # nadomesti z dejanskim datumom menjave
```

Predpona akcije je odvisna od imena vozlišča; po preimenovanju uporabi akcijo,
ki jo dejansko ponuja HA. Vnos ni lokalna opomba v HA: ESP pošlje zapis v UPS.
Nastavitev se ne zapisuje samodejno ob vsakem zagonu.

Izvedba preveri koledarski datum, prestopna leta in razpon 1980–2069 za APC BCD
zapis. Neveljaven datum zavrne pred USB pisanjem. Nato iz descriptorja poišče
ustrezno Feature poročilo, ga prebere, spremeni samo bite datuma, ohrani ID in
preostala polja, pošlje `SET_REPORT` in z `GET_REPORT` preveri odčitek.
Ob nepotrjenem zapisu API zabeleži opozorilo; uspeh potrdi z novo vrednostjo
`Battery date` oziroma `battery.date`, ne zgolj z zaključkom akcije HA.

Na delujoči enoti je bil potrjen zapis **istega datuma `2015-12-09`**, USB ACK
in ponovni odčitek. Zapis drugega datuma in ohranitev skozi fizični izklop UPS
še nista preizkušena. Napačen datum `2025-02-30` je bil zavrnjen brez spremembe.
NUT ostaja samo berljiv: isti podatek ni zapisljiv z `SET VAR`.

## Home Assistant pravi »UPS je že konfiguriran«

Ime `racktest` je omrežno ime NUT. Integracija HA NUT prepozna fizični UPS po
proizvajalcu, modelu in serijski številki, zato drugo ime ali IP ne ustvarita
nove identitete. V Settings → Devices & services → NUT preveri obstoječe vnose,
tudi onemogočene; obstoječi vnos ponovno omogoči oziroma konfiguriraj na trenutni
naslov in prijavo. Tako lahko ohraniš zgodovino namesto ustvarjanja dvojnika.

## Kaj preveriti po namestitvi

Preveri model/firmware in smiselne meritve v ESPHome ter NUT. CS 350 je objavil
izhod 230 V; izmerjena vhodna napetost ni bila dosegljiva. Nazivnih 230 V ni
nadomestek manjkajoče vhodne meritve. Po spremembi imena/prijave preveri še
odjemalce in obstojnost nastavitev po ponovnem zagonu ESP.

[Preizkusi S3](testing-s3.md) ločujejo potrjeno delovanje od še nepreizkušenih
posegov. Za povratek uporabi svoj shranjeni YAML skupaj s pripadajočo različico
komponent; P4 konfiguracija ni povratna slika za S3/W5500.

Viri: [ESPHome external components](https://esphome.io/components/external_components/),
[Home Assistant NUT](https://www.home-assistant.io/integrations/nut/),
[identiteta UPS v integraciji HA](https://github.com/home-assistant/core/blob/dev/homeassistant/components/nut/__init__.py).
