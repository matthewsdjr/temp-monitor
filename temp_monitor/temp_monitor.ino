// ============================================================
//  ESP32-C3 Super Mini + MAX6675 — Monitor de temperatura (PRODUCCIÓN)
//  Publica datos via MQTT → broker.hivemq.com (público, gratis)
//
//  LIBRERÍAS REQUERIDAS (instalar en Library Manager):
//    - MAX6675 library  (Adafruit)
//    - PubSubClient     (Nick O'Leary)
//    - ArduinoJson      (Benoit Blanchon)
//
//  Arduino IDE → Board: "ESP32C3 Dev Module"
//              → USB CDC On Boot: "Enabled"
//
//  CONEXIONES:
//    GPIO 4 → SCK   |  GPIO 5 → MISO
//    GPIO 6 → CS sensor 0
//    GPIO 7 → CS sensor 1
// ============================================================

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <max6675.h>

// ── WiFi — cambia estos valores ──────────────────────────────
const char* WIFI_SSID = "INOCUIDAD";
const char* WIFI_PASS = "#S3cur1ty#";

// ── MQTT — cambia PROYECTO_ID por un nombre único tuyo ───────
//    Ejemplo: "tony_esp32_lab1"  (sin espacios ni caracteres especiales)
const char* MQTT_BROKER    = "broker.hivemq.com";
const int   MQTT_PORT      = 1883;
const char* MQTT_TOPIC     = "temp_monitor/tony_lab_unf/data";
// Client ID único por sesión: evita que el broker expulse la conexión anterior
char        MQTT_CLIENT_ID[32];

// ── Sensores ─────────────────────────────────────────────────
const int PIN_SCK  = 4;
const int PIN_MISO = 5;

const int PIN_CS[] = {
  6,   // sensor 0
  7,   // sensor 1
//  2,  // sensor 2  ← descomentar al agregar
//  3,  // sensor 3
};
const int NUM_SENSORES = sizeof(PIN_CS) / sizeof(PIN_CS[0]);

MAX6675* sensores[NUM_SENSORES];

// ── Intervalo de publicación ─────────────────────────────────
const unsigned long INTERVALO_MS = 2000;
unsigned long ultimaPublicacion  = 0;

WiFiClient   wifiClient;
PubSubClient mqtt(wifiClient);

// ─────────────────────────────────────────────────────────────
void conectarWifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.printf("\nConectando a WiFi '%s'", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.printf("\nWiFi OK — IP: %s\n", WiFi.localIP().toString().c_str());
}

void conectarMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Conectando a MQTT...");
    if (mqtt.connect(MQTT_CLIENT_ID)) {
      Serial.println(" OK");
    } else {
      Serial.printf(" fallo (rc=%d), reintentando en 5s\n", mqtt.state());
      delay(5000);
    }
  }
}

// ─────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(2000);  // espera a que el USB CDC enumere en el PC

  for (int i = 0; i < NUM_SENSORES; i++) {
    sensores[i] = new MAX6675(PIN_SCK, PIN_CS[i], PIN_MISO);
  }
  delay(500);

  conectarWifi();

  // Client ID único usando el MAC address del ESP32
  uint64_t mac = ESP.getEfuseMac();
  snprintf(MQTT_CLIENT_ID, sizeof(MQTT_CLIENT_ID), "esp32_%08X", (uint32_t)mac);
  Serial.printf("Client ID: %s\n", MQTT_CLIENT_ID);

  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setKeepAlive(60);      // keepalive cada 60 seg
  mqtt.setSocketTimeout(10);  // timeout de socket en 10 seg
  conectarMQTT();

  Serial.printf("Publicando en topic: %s\n", MQTT_TOPIC);
}

// ─────────────────────────────────────────────────────────────
void loop() {
  conectarWifi();
  if (!mqtt.connected()) conectarMQTT();
  mqtt.loop();

  unsigned long ahora = millis();
  if (ahora - ultimaPublicacion < INTERVALO_MS) return;
  ultimaPublicacion = ahora;

  StaticJsonDocument<256> doc;
  doc["ts"] = ahora;
  JsonArray arr = doc.createNestedArray("s");

  for (int i = 0; i < NUM_SENSORES; i++) {
    float t = sensores[i]->readCelsius();
    JsonObject s = arr.createNestedObject();
    s["id"] = i;
    if (isnan(t)) {
      s["err"] = true;
    } else {
      s["t"] = round(t * 100.0) / 100.0;
    }
  }

  char payload[256];
  serializeJson(doc, payload);
  mqtt.publish(MQTT_TOPIC, payload, true);  // retain=true: el dashboard ve el último dato al conectar
  Serial.println(payload);
}
