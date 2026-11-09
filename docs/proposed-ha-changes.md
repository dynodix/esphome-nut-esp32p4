# Predlog sprememb na Home Assistantu

Predlog je bil odobren in izveden 2026-11-09. Dokument ohranja pregled sprememb.

Spremenila bi se samo glavna konfiguracija
`/config/esphome/ups-nut-esp32p4.yaml`, dodala pa bi se nova, izolirana mapa
`/config/esphome/components/ups_nut_esp32p4/`.

Ključne razlike glede na trenutno glavno konfiguracijo:

```diff
-  - source: github://bullshit/esphome-components
+  - source:
+      type: local
+      path: components/ups_nut_esp32p4

-  level: DEBUG
+  level: INFO

-usb_host:
+# odstranjeno: ups_hid je edini lastnik USB Host sklada

-#ups_hid: ...
+ups_hid:
+  id: ups_monitor
+  protocol: auto
+  protocol_timeout: 15s
+  update_interval: 10s
+  fallback_nominal_voltage: 230V

-#nut_server: ...
+nut_server:
+  ups_hid_id: ups_monitor
+  ups_name: ups-esp32p4
+  port: 3493
+  username: !secret nut_username
+  password: !secret nut_password
+  max_clients: 4
```

Poleg tega se dodajo samo bralne UPS entitete iz lokalnega kandidata. Noben UPS
gumb, stikalo ali upravljalni ukaz ni izpostavljen. Celotna primerjava je na voljo
med `backup/ha-current/ups-nut-esp32p4.yaml` in `ups-nut-esp32p4.yaml`.
