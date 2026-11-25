# UPS NUT na ESP32-P4 in ESP32-S3

[English](README.md)

Projekt ESPHome poveže UPS prek USB HID z omrežjem Ethernet. ESP hkrati ponuja
strežnik NUT na TCP 3493 in izvorni API ESPHome za Home Assistant. Repozitorij
vsebuje dve preverjeni kombinaciji; prvotno ime ostaja, konfiguraciji in lokalne
komponente pa sta ločeni.

| | P4 – prvotna rešitev | S3 – rešitev z vzdrževanjem |
|---|---|---|
| Preverjeni UPS | APC Back-UPS RS 650MI, `939.g4` | APC Back-UPS CS 350, `807.q10 .I USB FW:q10` |
| Ethernet | IP101 PHY, EMAC/RMII | W5500, SPI |
| USB podatki | Fiksni razpored APC poročil preverjenega UPS | Razpored iz HID descriptorja: usage, ID, biti in merilo |
| Ime UPS v NUT | Nastavitev YAML | Trajna nastavitev med delovanjem; privzeto `racktest` |
| Prijava NUT | Poverilnice v YAML/secrets | Stikalo med delovanjem; privzeto brez prijave |
| Pisanje prek NUT | Preverjeno branje; brez splošne blokade pisanja | `read_only: true`; pisanje in krmiljenje zavrnjena |
| Vzdrževanje | Ta konfiguracija nima vpisa datuma | Branje datumov/rezultata testa in vpis datuma menjave prek ESPHome |
| Preverjeno na strojni opremi | 2026-11-09 | 2026-11-25 |

Oba UPS uporabljata **USB HID**. Razlika v Ethernetu izvira iz plošče ESP,
razlika v razporedu USB poročil pa iz modela in firmware UPS. S3 komponenta še ni
bila preverjena kot zamenjava kode na P4.

## Dokumentacija in datoteke

- [Primerjava P4 in S3](docs/p4-vs-s3.md): strojna oprema, poti komunikacije in razlogi za popravke.
- [S3 + APC Back-UPS CS 350](docs/esp32-s3-apc-cs350.md): namestitev, nastavitve NUT in vpis datuma baterije.
- [Preverjanje S3](docs/testing-s3.md): rezultati, preizkusi in meje preverjanja.
- [Konfiguracija P4](ups-nut-esp32p4.yaml), [komponente P4](components/ups_nut_esp32p4) in [prvotni preizkusi](docs/testing.md).
- [Konfiguracija S3](ups-nut-esp32s3.yaml), [komponente S3](components/ups_nut_esp32s3) in [ovojnica za okoljske poverilnice](ups-nut-esp32s3-runtime.yaml).

Za S3 kopiraj YAML in pripadajočo mapo komponent v ESPHome. Iz
[secrets.example.yaml](secrets.example.yaml) pripravi svoj `secrets.yaml`.
Sklici na skrivnosti morajo biti razrešeni tudi pri izključeni prijavi NUT.

```powershell
uv run --python 3.14 --with esphome==2026.11.1 esphome config ups-nut-esp32s3.yaml
uv run --python 3.14 --with esphome==2026.11.1 esphome compile ups-nut-esp32s3.yaml
```

S3 prek NUT samo bere `battery.date`, `battery.mfr.date` in `ups.test.result`.
Datum menjave je mogoče zapisati v UPS prek besedilne nastavitve ali akcije
ESPHome. Preverjen je bil zapis obstoječega datuma in ponovni odčitek; sprememba
na drug datum in ohranitev po izpadu napajanja UPS še nista preverjeni.
`Last self test` pomeni **rezultat**, ne datuma zadnjega testa.

Koda in YAML P4 ostajata nespremenjena. Dokumenti o izhodišču, spremembah in
namestitvi P4 so označeni kot zgodovinski. Drugi modeli UPS ter poti za
generic/CyberPower niso preverjeni na strojni opremi. Med preizkusi nismo sprožili
samotesta ali ukaza za izklop UPS.
