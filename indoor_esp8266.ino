#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Define pins for RGB LED
const int R = D6;  


const char *ssid = "Aztaroz";
const char *password = "14591459";
const char *mqtt_server = "192.168.182.37";
const char *mqtt_clientId = "RGBLEDClient";  // Choose a unique client ID
const char *mqtt_topic = "door_state";  // Change to the topic you want to subscribe to

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  pinMode(R, OUTPUT);
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  reconnect();
 
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // client.subscribe(mqtt_topic);
  // Add any additional logic or tasks you need in the loop
}

void callback(char *topic, byte *payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Received message on topic: ");
  Serial.print(topic);
  Serial.print(", Message: ");
  Serial.println(message);

  // Parse the message and change the LED color accordingly
 
    if (String(topic) == mqtt_topic) {
        if (message == "1") {
            digitalWrite(R, HIGH);
        } else if (message == "0") {
            digitalWrite(R, LOW);
        }
    }
}

void setLEDColor(int red, int green, int blue) {
  analogWrite(R, red);

}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = mqtt_clientId;
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
