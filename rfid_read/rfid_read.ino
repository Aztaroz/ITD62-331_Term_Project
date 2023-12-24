// #include <SPI.h>
// #include <MFRC522.h>

// #define RST_PIN         9           // Configurable, see typical pin layout above
// #define SS_PIN          10          // Configurable, see typical pin layout above

// MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
// bool door = false;
// //*****************************************************************************************//
// void setup() {
//   pinMode(7, OUTPUT);
//   Serial.begin(115200);                                           // Initialize serial communications with the PC
//   SPI.begin();                                                  // Init SPI bus
//   mfrc522.PCD_Init();                                              // Init MFRC522 card
//   // Serial.println(F("Read personal data on a MIFARE PICC:"));    //shows in serial that it is ready to read
// }

// //*****************************************************************************************//
// void loop() {
//   // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
//   MFRC522::MIFARE_Key key;
//   for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

//   //some variables we need
//   byte buffer2[18];
//   char text[18];

//   byte block;
//   byte len = sizeof(buffer2);
//   MFRC522::StatusCode status;

//   //-------------------------------------------

//   // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
//   if ( ! mfrc522.PICC_IsNewCardPresent()) {
//     return;
//   }

//   // Select one of the cards
//   if ( ! mfrc522.PICC_ReadCardSerial()) {
//     return;
//   }

//   //---------------------------------------- Get ID


//   block = 1;

//   status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
//   if (status != MFRC522::STATUS_OK) {
//     Serial.print(F("Authentication failed: "));
//     Serial.println(mfrc522.GetStatusCodeName(status));
//     return;
//   }

//   status = mfrc522.MIFARE_Read(block, buffer2, &len);
  
//   if (status != MFRC522::STATUS_OK) {
//     Serial.print(F("Reading failed: "));
//     Serial.println(mfrc522.GetStatusCodeName(status));
//     return;
//   }

//   //PRINT LAST NAME
//   for (uint8_t i = 0; i < 8; i++) {   // i < 16
//     // Serial.write(buffer2[i]);
//     text[i] = buffer2[i];
//   }

//   text[8] = '\0';

//   Serial.write(text);
  

//   // if (strcmp(text, "64100738") == 0) {
//   //   if (door == false) {
//   //     digitalWrite(7, HIGH);
//   //     door = true;
//   //     Serial.write(text);
//   //   }else {
//   //     digitalWrite(7, LOW);
//   //     door = false;
//   //   }
//   // }
//   //----------------------------------------



//   // Serial.println(F("\n**End Reading**\n"));

//   delay(1000); //change value if you want to read cards faster

//   mfrc522.PICC_HaltA();
//   mfrc522.PCD_StopCrypto1();
// }
// //*****************************************************************************************//









#include <ESP8266WiFi.h>
#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient client(espClient);

const char* ssid = "Aztaroz";
const char* password = "14591459";
const char* mqtt_server = "192.168.182.37";

const int R = 2;
const int B = 4;
const int G = 12;

const int relay = 16;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  digitalWrite(R, HIGH);
  digitalWrite(B, HIGH);
  digitalWrite(G, HIGH);
  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '0') {
    digitalWrite(relay, LOW);  // Turn the Relay on
    digitalWrite(R, HIGH);
    digitalWrite(B, HIGH);
    digitalWrite(G, LOW);
    delay(1500);
  } else if ((char)payload[0] == '1') {
    digitalWrite(relay, HIGH);  // Turn the Relay off

    digitalWrite(R, HIGH);
    digitalWrite(B, HIGH);
    digitalWrite(G, LOW);
    delay(1500);
  } else {
    for (int x = 0; x <= 2; x++) {
      digitalWrite(R, LOW);
      digitalWrite(B, HIGH);
      digitalWrite(G, HIGH);
      delay(300);
      digitalWrite(R, HIGH);
      digitalWrite(B, HIGH);
      digitalWrite(G, HIGH);
      delay(300);
    }
    delay(1000);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

String text;
bool toggle;

void setup() {
  Serial.begin(115200);
  pinMode(R, OUTPUT);      //LED Red
  pinMode(G, OUTPUT);      //LED Green
  pinMode(B, OUTPUT);      //LED Blue
  pinMode(relay, OUTPUT);  //Relay
  toggle = true;

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  digitalWrite(R, HIGH);    //Use digital pin instead of analog pin so the value can be reversed (HIGH = LOW and LOW = HIGH)
  digitalWrite(G, HIGH);
  digitalWrite(B, LOW);


  if (Serial.available() > 0) {
    text = Serial.readString();

    // if (text == "64100738") {

    snprintf(msg, MSG_BUFFER_SIZE, "%s", text);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("authen", msg);

    client.subscribe("door_state");

    // digitalWrite(16, toggle);
    // toggle = !toggle;
    // } else {
    //   digitalWrite(12, HIGH);
    //   delay(300);
    //   digitalWrite(12, LOW);
    //   delay(300);
    //   digitalWrite(12, HIGH);
    //   delay(300);
    //   digitalWrite(12, LOW);
    //   delay(300);
    //   digitalWrite(12, HIGH);
    //   delay(300);
    //   digitalWrite(12, LOW);
    // }
  }
}