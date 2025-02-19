/*
 * CollabCanvas code by Eric Yang and Jordan Gonzalez
 *
 * Credit to ESPNOW framework documentation for networking
 *
 * Electronics 2025 Final Project
 */

#include "HWCDC.h"
#include "esp32-hal.h"
#include "esp_err.h"
#include "lwip/ip_addr.h"
#include "pgmspace.h"
#include <WiFi.h>
#include <cstdint>
#include <cstring>
#include <esp_now.h>
#include <iterator>

using namespace std;

HWCDC USBSerial;

#define MESSAGE_BUFFER_SIZE 100

// NOTE: define MAC address with an unsigned 8 integer array, thanks!
// Use hexidecimal system, i.e. 0xFF to encode letters A-F in MAC address.

// FLASH THIS FOR WAND 1
// const uint8_t RECEIVER_MAC_ADD[] = {0x20, 0x43, 0xA8, 0x65, 0xEB, 0x68};

// FLASH THIS FOR WAND 2
const uint8_t RECEIVER_MAC_ADD[] = {0x20, 0x43, 0xA8, 0x65, 0xC0, 0x7C};

String success;

// TODO: implement actual wand_message structure later
// need to send IMU data as floats
typedef struct wand_message {
  char user_message[MESSAGE_BUFFER_SIZE];
} wand_message;

wand_message myMessage;
wand_message incomingMessage;

// ESP-NOW peer info for the receiver
esp_now_peer_info peerInfo;

void onDataSent(const uint8_t *mac, esp_now_send_status_t status) {
  USBSerial.print("\r\nLast Packet Send Status:\t");
  if (status == ESP_NOW_SEND_SUCCESS) {
    USBSerial.println("Delivery Success");
    success = "Delivery Success :)";
  } else {
    USBSerial.println("Delivery Fail");
    success = "Delivery Fail :(";
  }
}

void onDataRecv(const esp_now_recv_info *mac, const uint8_t *incomingData,
                int len) {
  // Copy the incoming data into incomingMessage
  memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));
  USBSerial.print("Bytes received: ");
  USBSerial.println(len);
  // Print the received message stored in the char array
  USBSerial.println(incomingMessage.user_message);
}

String expose_mac_address() {
  // Helper function to return ESP32's MAC address
  WiFi.mode(WIFI_STA);
  String macAddress = WiFi.macAddress();
  return macAddress;
}

String input_handler() {
  if (USBSerial.available() > 0) {
    String inputString = USBSerial.readStringUntil('\n');
    return inputString;
  }
  return "";
}

void setup() {
  USBSerial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    USBSerial.println("Error starting ESPNOW protocol!");
    return;
  }

  esp_now_register_send_cb(onDataSent);

  // Set up peer information for the receiver
  memcpy(peerInfo.peer_addr, RECEIVER_MAC_ADD, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add the peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    USBSerial.println("Peer add FAILED");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);
}

void loop() {
  String input = input_handler();
  if (input.length() > 0) {
    // Convert the input String to a C-style char array and copy it into our
    // message struct
    input.toCharArray(myMessage.user_message, MESSAGE_BUFFER_SIZE);

    // Send the message via ESP-NOW
    esp_err_t result = esp_now_send(RECEIVER_MAC_ADD, (uint8_t *)&myMessage,
                                    sizeof(myMessage));
    if (result == ESP_OK) {
      USBSerial.println("Sent: " + input);
    } else {
      USBSerial.println("Failed to send...");
    }
  }
  delay(1000);
}
