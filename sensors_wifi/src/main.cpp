#include <WiFi.h>

#include "homectl/Matrix.h"
#include "homectl/unittest.h"

// WiFi network name and password:
#include "../include/password.h"

// Internet domain to request from:
const char *hostDomain = "xinutec.org";
const int hostPort = 80;

const int LED_PIN = 5;

void connectToWiFi(const char *ssid, const char *pwd);
void requestURL(const char *host, uint8_t port);
void printLine();

void setup() {
  // Initialise hardware:
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);

  // Connect to the WiFi network (see function below loop)
  connectToWiFi(networkName, networkPswd);

  digitalWrite(LED_PIN, LOW);  // LED off
  Serial.print("Press button 0 to connect to ");
  Serial.println(hostDomain);
}

void loop() {
  static bool done = false;
  if (!done) {
    digitalWrite(LED_PIN, HIGH);       // Turn on LED
    requestURL(hostDomain, hostPort);  // Connect to server
    digitalWrite(LED_PIN, LOW);        // Turn off LED

    done = true;
  }
}

void connectToWiFi(const char *ssid, const char *pwd) {
  int ledState = 0;

  printLine();
  Serial.println("Connecting to WiFi network: " + String(ssid));

  WiFi.begin(ssid, pwd);
  WiFi.onEvent(
      [](WiFiEvent_t event, WiFiEventInfo_t info) { WiFi.reconnect(); },
      SYSTEM_EVENT_STA_DISCONNECTED);

  while (WiFi.status() != WL_CONNECTED) {
    // Blink LED while we're connecting:
    digitalWrite(LED_PIN, ledState);
    ledState = (ledState + 1) % 2;  // Flip ledState
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void requestURL(const char *host, uint8_t port) {
  printLine();
  Serial.println("Connecting to domain: " + String(host));

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return;
  }
  Serial.println("Connected!");
  printLine();

  // This will send the request to the server
  client.print((String) "GET / HTTP/1.1\r\n" + "Host: " + String(host) +
               "\r\n" + "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
  client.stop();
}

void printLine() {
  Serial.println();
  for (int i = 0; i < 30; i++) Serial.print("-");
  Serial.println();
}
