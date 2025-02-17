/*
 * CollabCanvas code by Eric Yang and Jordan Gonzalez
 *
 * Credit to ESPNOW framework documentation for networking
 *
 * Electronics 2025 Final Project
 */

#include "HardwareSerial.h"
#include "esp32-hal.h"
#include "esp_err.h"
#include "lwip/ip_addr.h"
#include "pgmspace.h"
#include <WiFi.h>
#include <cstdint>
#include <cstring>
#include <esp_now.h>

using namespace std;

// NOTE: define MAC address with an unsigned 8 integer array, thanks!
// Use hexidecimal system, i.e. 0xFF to encode letters A-F in MAC address.
const uint8_t RECEIVER_MAC_ADD[] = {0x20, 0x43, 0xA8, 0x65, 0xEB, 0x68};

String success;

// TODO: implement actual wand_message structure later
// need to send IMU data as floats
typedef struct wand_message {
  String user_message;
} wand_message;

String myText;
String incomingText;

wand_message myMessage;
wand_message incomingMessage;

// esp_now tracking peer information of reciever
esp_now_peer_info peerInfo;

// NOTE: We need an OnDataSent and OnDataRecv for ESPNOW, this is like an
// "OnInterrupt" that triggers when we send/recieve data

void onDataSent(const uint8_t *mac, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  // check if our status is ESP_NOW_SEND_SUCCESS or 0, if it is, we've sent
  // something :)
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Delivery Success");
    success = "Delivery Success :)";
  } else {
    Serial.println("Delivery Fail");
    success = "Delivery Fail :(";
  }
}

void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));
  Serial.print("Bytes received: ");
  Serial.println(len);
  incomingText = incomingMessage.user_message;
  Serial.println(incomingText);
}

String expose_mac_address() {
  /*
   * helper function to return ESP32's MAC address
   * returns: String
   */
  WiFi.mode(WIFI_STA);
  String macAddress = WiFi.macAddress();
  return macAddress;
}

void setup() {
  Serial.begin(115200);
  // set this ESP as a WiFi station
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error starting ESPNOW protocol!");
    return;
  }

  esp_now_register_send_cb(onDataSent);

  // peer registration
  memcpy(peerInfo.peer_addr, RECEIVER_MAC_ADD, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // peer add
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Peer add FAILED");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);
}

String input_handler() {
  if (Serial.available() > 0) {
    String inputString = Serial.readStringUntil('\n');
    inputString.trim();
    return inputString;
  }
}

void loop() {
  String input = input_handler();
  myMessage.user_message = input;
  if (input.length() > 0) {
    esp_err_t result = esp_now_send(RECEIVER_MAC_ADD, (uint8_t *)&myMessage,
                                    sizeof(myMessage));
    if (result == ESP_OK) {
      Serial.println(input);
    } else {
      Serial.println("Failed to send...");
    }
  }
  delay(1000);
}
