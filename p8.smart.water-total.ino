#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "Spark";
const char* password = "laputalpt";
String Chip_ID = "p8_dash_water_0001";
int Bps = 2400;
ESP8266WebServer server(80);

uint8_t cur_code[] = { 0x68, 0x19, 0x04, 0x80, 0x59, 0x09, 0x14, 0x00, 0x00, 0x01, 0x03, 0xA5, 0x8A, 0x01, 0xAF, 0x16 };
uint8_t cur_result[90];
uint8_t total_code[] = { 0x68, 0x19, 0x04, 0x80, 0x59, 0x09, 0x14, 0x00, 0x00, 0x01, 0x03, 0x1F, 0x90, 0x01, 0x2F, 0x16 };
uint8_t total_result[35];

void handleRoot() {
  server.send(200, "text/json", "\"chip_id\":\"p8_dash_water_0001\"");
}

void UpdateIPAddress() {
  uint16_t port = 80;
  char * host = "pipa.joinp8.com";
  String IP = WiFi.localIP().toString();
  Serial.print("connecting to ");
  Serial.println(host);
  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    Serial.println("wait 5 sec...");
    delay(1000);
  }
  String url = "/api/setDeviceIpAddress";
  url += "?chip=" + Chip_ID + "&ip=";
  url += IP;
  Serial.print("Requesting URL: ");
  Serial.println(url);
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
}

void setup(void) {
  Serial.begin(Bps, SERIAL_8E1);
  //builtin led
  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("room404")) {
    Serial.println("MDNS responder started");
  }

  digitalWrite(LED_BUILTIN, HIGH);

  server.on("/", handleRoot);

  server.begin();
  Serial.println("HTTP server started");

  UpdateIPAddress();
}

void sendValue(String chip_id, String device, String value)
{
  uint16_t port = 80;
  char * host = "pipa.joinp8.com";
  String IP = WiFi.localIP().toString();
  WiFiClient client;
  if (!client.connect(host, port)) {
    delay(1000);
  }
  String url = "/api";
  url += "/setdevicevalue/?chip=" + chip_id + "&device=" + device + "&value=" + value;
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
}

String zeroAdd(uint8_t value)
{
  return value < 0x0F ? "0" + String(value, HEX) : String(value, HEX);
}

int hexToInt(char value)
{
  if (value == '0')
  {
    return 0;
  }
  if (value == '1')
  {
    return 1;
  }
  if (value == '2')
  {
    return 2;
  }
  if (value == '3')
  {
    return 3;
  }
  if (value == '4')
  {
    return 4;
  }
  if (value == '5')
  {
    return 5;
  }
  if (value == '6')
  {
    return 6;
  }
  if (value == '7')
  {
    return 7;
  }
  if (value == '8')
  {
    return 8;
  }
  if (value == '9')
  {
    return 9;
  }
  if (value == 'A')
  {
    return 10;
  }
  if (value == 'B')
  {
    return 11;
  }
  if (value == 'C')
  {
    return 12;
  }
  if (value == 'D')
  {
    return 13;
  }
  if (value == 'E')
  {
    return 14;
  }
  if (value == 'F')
  {
    return 15;
  }
}

long hexStringToLong(String value) {
  int ori = 0;
  for (int i = 0; i < value.length(); i++)
  {
    ori += hexToInt(value[i]) * pow(16, value.length() - i - 1);
  }
  return ori;
}

String getCur() {
  Serial.write(cur_code, 16);
  Serial.readBytes(cur_result, 90);
  //  for (int i = 0; i < 90; i++)
  //  {
  //    Serial.print(cur_result[i], HEX);
  //    Serial.print(" ");
  //  }
  //  Serial.println();
  for (int i = 0; i < 6; i++)
  {
    if (String(cur_result[i], HEX) == "68")
    {
      String result = zeroAdd(cur_result[i + 41]) +
                      zeroAdd(cur_result[i + 40]) +
                      zeroAdd(cur_result[i + 39]);
      long int_result = hexStringToLong(result);
      char valueStr[30];
      double float_result = double(int_result) / double(1000000);
      dtostrf(float_result, 2, 2, valueStr);
      Serial.print("current water:");
      Serial.println(String(valueStr));
      return String(valueStr);
    }
  }
  return "0";
}

String getTotal() {
  Serial.write(total_code, 16);
  Serial.readBytes(total_result, 35);
  for (int i = 0; i < 6; i++)
  {
    if (String(total_result[i], HEX) == "68")
    {
      String result = zeroAdd(total_result[i + 17]) +
                      zeroAdd(total_result[i + 16]) +
                      zeroAdd(total_result[i + 15]) + "." +
                      zeroAdd(total_result[i + 14]);
      float value = result.toFloat();
      Serial.print("total_water:");
      Serial.println(String(value));
      return String(value);
    }
  }
  return "0";
}

void loop(void) {
  if (millis()%600000 == 0)
  {
    sendValue("p8_dash_water_0001", "total_water", getTotal());
    delay(200);
  }
  if (millis()%5000 == 0)
  {
    sendValue("p8_dash_water_0001", "cur_water", getCur());
    delay(200);
  }
}
