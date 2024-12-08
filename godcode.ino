#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

#define DHTPIN 8
#define DHTTYPE DHT11
#define RX 2
#define TX 3

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial esp8266(RX, TX);

String AP = "NET DAEMON";       // AP NAME
String PASS = "monsoondaily";   // AP PASSWORD
String API = "V9ZMG1QPLK0LHT8H";   // Write API KEY
String HOST = "api.thingspeak.com";
String PORT = "80";

void setup() {
    lcd.init();
    lcd.backlight();
    Serial.begin(9600);
    dht.begin();
    pinMode(A1, INPUT);  // For MQ-135 sensor
    pinMode(A2, INPUT);  // For MQ-7 sensor
    esp8266.begin(115200);

    // ESP8266 setup
    sendCommand("AT", 5, "OK");
    sendCommand("AT+CWMODE=1", 5, "OK");
    sendCommand("AT+CWJAP=\"" + AP + "\",\"" + PASS + "\"", 20, "OK");
}

void loop() {
    int a = analogRead(A1);  // Read MQ-135 sensor value
    int b = analogRead(A2);  // Read MQ-7 sensor value
    float h = dht.readHumidity();   // Read humidity from DHT sensor
    float t = dht.readTemperature();  // Read temperature from DHT sensor

    if (isnan(h) || isnan(t)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        lcd.setCursor(0, 0);
        lcd.print("Error reading DHT");
        delay(1000);
        return;
    }

    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C "));
    Serial.print(F("MQ-135 Value: "));
    Serial.print(a);
    Serial.print(F(" MQ-7 Value: "));
    Serial.println(b);

    // Display MQ-135 sensor value
    lcd.setCursor(0, 0);
    lcd.print("MQ-135: ");
    lcd.print(a);
    delay(2000);
    lcd.clear();

    // Display MQ-7 sensor value
    lcd.setCursor(0, 0);
    lcd.print("MQ-7: ");
    lcd.print(b);
    delay(2000);
    lcd.clear();

    // Display temperature
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(t);
    delay(2000);
    lcd.clear();

    // Display humidity
    lcd.setCursor(0, 0);
    lcd.print("Hum: ");
    lcd.print(h);
    delay(2000);
    lcd.clear();

    // Send data to ThingSpeak
    String getData = "GET /update?api_key=" + API + "&field1=" + a + "&field2=" + b + "&field3=" + t + "&field4=" + h;
    sendCommand("AT+CIPMUX=1", 5, "OK");
    sendCommand("AT+CIPSTART=0,\"TCP\",\"" + HOST + "\"," + PORT, 15, "OK");
    sendCommand("AT+CIPSEND=0," + String(getData.length() + 4), 4, ">");
    esp8266.println(getData);
    sendCommand("AT+CIPCLOSE=0", 5, "OK");
}

// Function to send AT commands and wait for a specific response
void sendCommand(String command, int maxWait, String response) {
    String readString = "";
    esp8266.println(command);  // Send the AT command
    long int time = millis();
    while (millis() - time < (maxWait * 1000)) {
        while (esp8266.available()) {
            char c = esp8266.read();
            readString += c;
        }
        if (readString.indexOf(response) != -1) {
            return;
        }
    }
    Serial.println("Error: No response from ESP8266 for command: " + command);
}
