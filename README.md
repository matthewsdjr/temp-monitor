# Monitor de Temperatura — ESP32-C3 + MAX6675

Monitor de temperatura en tiempo real con múltiples termocuplas tipo K, publicación vía MQTT y dashboard web público alojado en GitHub Pages.

## Hardware

- ESP32-C3 Super Mini
- MAX6675 (uno por punto de medición, hasta 4)
- Termocuplas tipo K
- Capacitor 100–1000 µF entre 3.3V y GND (estabilidad WiFi)

## Conexiones

```
ESP32-C3        MAX6675 #0    MAX6675 #1
────────        ──────────    ──────────
GPIO 4  ──────── SCK ─────── SCK
GPIO 5  ──────── SO  ─────── SO
GPIO 6  ──────── CS
GPIO 7  ──────────────────── CS
3.3V    ──────── VCC ─────── VCC
GND     ──────── GND ─────── GND
```

Para agregar sensores 3 y 4, descomentar `GPIO 2` y `GPIO 3` en `PIN_CS[]`.

## Estructura del repositorio

```
├── index.html              # Dashboard web (GitHub Pages)
├── temp_monitor/
│   └── temp_monitor.ino    # Sketch de producción (WiFi + MQTT)
├── temp_sensor/
│   └── temp_sensor.ino     # Sketch de diagnóstico (bit-bang sin librería)
├── wifi_test/
│   └── wifi_test.ino       # Test mínimo de conectividad WiFi
└── blink_test/
    └── blink_test.ino      # Test mínimo del chip
```

## Configuración Arduino IDE

| Parámetro | Valor |
|---|---|
| Board | `ESP32C3 Dev Module` |
| USB CDC On Boot | `Enabled` |
| Upload Speed | `921600` |

### Librerías requeridas (Library Manager)

- `MAX6675 library` — Adafruit
- `PubSubClient` — Nick O'Leary
- `ArduinoJson` — Benoit Blanchon

## Configuración del sketch

En `temp_monitor/temp_monitor.ino`, editar:

```cpp
const char* WIFI_SSID  = "TU_RED";
const char* WIFI_PASS  = "TU_PASSWORD";
const char* MQTT_TOPIC = "temp_monitor/TU_PROYECTO_ID/data";
```

En `index.html`, editar:

```js
const TOPIC = 'temp_monitor/TU_PROYECTO_ID/data';
```

Usar el mismo `PROYECTO_ID` en ambos archivos.

## Procedimiento de upload al ESP32-C3

1. Mantén presionado **BOOT**
2. Presiona y suelta **RST**
3. Suelta **BOOT**
4. Haz clic en **Upload** en Arduino IDE
5. Al terminar, presiona **RST** para arrancar el programa

## MQTT

- Broker: `broker.hivemq.com` (público, sin registro)
- Puerto ESP32: `1883`
- Puerto dashboard (WebSocket): `8884` (WSS)
- Payload JSON: `{"ts":12345,"s":[{"id":0,"t":25.50},{"id":1,"t":26.25}]}`

## Dashboard

Disponible en: `https://TU_USUARIO.github.io/TU_REPO`

Muestra temperatura en tiempo real de cada sensor con historial de las últimas 60 lecturas.
