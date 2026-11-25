# Preverjanje S3 + APC Back-UPS CS 350

Datum: 2026-11-25. Spodnje meritve veljajo za delujočo enoto »ESP32-S3 POE ETH
UPS«, ki je bila posodobljena prek OTA. Objavljeni YAML je njen prenosljiv primer
z drugačnim imenom vozlišča/prikaza in brez zasebnih omrežnih komentarjev.
Izvorna koda komponent je enaka preizkušeni, razen dokumentacije README.

## Okolje in gradnja

| Podatek | Rezultat |
|---|---|
| ESP | ESP32-S3, board `esp32-s3-devkitc-1`, W5500 SPI |
| UPS | APC Back-UPS CS 350 |
| UPS firmware | `807.q10 .I USB FW:q10` |
| ESPHome / ESP-IDF / GCC | 2026.11.1 / 5.5.6 / 14.2.0 |
| Konfigurirani flash | 4 MB; fizična velikost ni ločeno izmerjena |
| RAM po prevajanju | 73.979 / 341.760 bajtov, 21,6 % |
| Aplikacija po prevajanju | 760.815 / 1.835.008 bajtov, 41,5 % |
| Velikost OTA bin s poravnavo | 760.928 bajtov |

Preverjeni so bili celotna gradnja produkcijske konfiguracije, OTA in ponovna
povezava API ter NUT. Lokalni in nameščeni izvori komponent so bili primerjani
po vsebini/hashih. Objavljeni primer je dodatno preverjen z `esphome config` in
`compile --only-generate`; sam pod novim generičnim imenom ni bil naložen.

## Meritve na priključenem UPS

| Podatek | Odčitek |
|---|---|
| NUT ime | `racktest` |
| Status | `OL` |
| Napolnjenost | 100 % |
| Čas delovanja | NUT 1320 s, ESPHome 22 min |
| Napetost baterije | NUT 13,6 V; ESPHome približno 13,59 V |
| Izhodna napetost | 230 V |
| Obremenitev | 0 % |
| Izmerjena vhodna napetost | Nedosegljiva; brez izmišljene nadomestne meritve |
| Nazivna vhodna napetost | 230 V; ločeno od meritve |
| `battery.date` | `2015-12-09` |
| `battery.mfr.date` | `2015-12-09` |
| `ups.test.result` | `No test initiated` |

Datuma opisujeta shranjeni vrednosti v UPS; dejanski datum zadnjega fizičnega
vzdrževanja ni bil neodvisno potrjen. Serijska številka, MAC in naslovi delujoče
namestitve so iz javnega poročila izpuščeni.

## Funkcionalni preizkusi

| Preizkus | Rezultat |
|---|---|
| USB identiteta in HID descriptor | Pravilna identiteta CS 350 in podatki po descriptorju |
| ESPHome API ter NUT `LIST UPS`, `LIST VAR`, `GET VAR` | Uspešno branje in skladnost vrednosti |
| Privzeto brez prijave | Branje brez uporabniškega imena in gesla |
| Vklop prijave | Neprijavljeni odjemalec zavrnjen; pravilna prijava deluje |
| Standardni `LOGIN <ups>` | Podprt v prijavljeni seji |
| TCP fragmentacija/pipelining | Ukazi sestavljeni prek paketov in več ukazov v enem paketu |
| Sprememba imena NUT | Veljavno ime deluje; neveljavno ne prepiše shranjenega |
| Ponovni zagon ESP | Ime in stikalo prijave ohranjena; UPS podatki ponovno na voljo |
| NUT samo branje | `SET VAR`, `INSTCMD`, `FSD` zavrnjeni; seznam ukazov prazen |
| Vzdrževalni podatki | Datuma in rezultat testa prek NUT ter API ESPHome |
| ESPHome zapis obstoječega datuma | `2015-12-09`: USB ACK in potrjen GET_REPORT odčitek iste vrednosti |
| Neveljaven datum | `2025-02-30` zavrnjen; shranjeni datum ostane enak |

## Ponovljivi lokalni preizkusi

Prenosljiva C++ preizkusa uporabljata sintetične HID descriptorje in vrednosti.
Zaženeta se brez UPS, omrežja ali ESPHome. Zahtevata prevajalnik C++17;
spodnji primer uporablja `uv` in paket `ziglang`. Zaženi iz korena repozitorija:

```powershell
New-Item -ItemType Directory -Force validation | Out-Null
uv run --with ziglang python -m ziglang c++ -std=c++17 -I . tests/s3/test_hid_descriptor.cpp -o validation/test_hid_descriptor.exe
if ($LASTEXITCODE -ne 0) { throw "HID test compilation failed" }
./validation/test_hid_descriptor.exe
if ($LASTEXITCODE -ne 0) { throw "HID test failed" }
uv run --with ziglang python -m ziglang c++ -std=c++17 -I . tests/s3/test_maintenance_values.cpp -o validation/test_maintenance_values.exe
if ($LASTEXITCODE -ne 0) { throw "Maintenance test compilation failed" }
./validation/test_maintenance_values.exe
if ($LASTEXITCODE -ne 0) { throw "Maintenance test failed" }
```

- [HID test](../tests/s3/test_hid_descriptor.cpp): nestandardni report ID, merilo
  napetosti, bitna polja statusa, predznačene vrednosti, meje poročil in globalni
  HID PUSH/POP.
- [Vzdrževalni test](../tests/s3/test_maintenance_values.cpp): APC BCD in SBS datumi,
  veljavnost dni/prestopnih let, dovoljeni razpon za zapis in preslikave rezultatov
  samotesta; preveri tudi ohranitev ostalih polj pri vstavljanju datuma.

Za preverjanje YAML kopiraj `secrets.example.yaml` v lokalni, iz Git izključeni
`secrets.yaml`. Za samo lokalno preverjanje zadoščajo priložene nadomestne vrednosti.

```powershell
uv run --python 3.14 --with esphome==2026.11.1 esphome config ups-nut-esp32s3.yaml
uv run --python 3.14 --with esphome==2026.11.1 esphome compile --only-generate ups-nut-esp32s3.yaml
```

## Kaj ostaja nepreverjeno

- Zapis datuma, ki se razlikuje od obstoječega, in njegova ohranitev po fizičnem
  izklopu napajanja UPS. ACK in odčitek istega datuma potrdita prenosno pot,
  ne dokazujeta spremembe datuma ali trajne hrambe po izgubi napajanja.
- Fizični prehod z omrežja na baterijo, dolgotrajen preizkus pod bremenom,
  praznjenje in izklop/ponovni vklop izhoda UPS.
- Sprožitev samotesta. Odčitani rezultat ne vsebuje časovnega žiga testa.
- Drugi UPS modeli/firmware, generic/CyberPower in prenos parserja na P4.

Samotest in ukazi za izklop niso bili izvedeni. Ponovni zagon monitorja pomeni
ponovni zagon ESP, ne UPS. Javno poročilo je povzetek preverjanj; zasebni surovi
dnevniki in poverilnice niso del repozitorija.
