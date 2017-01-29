#include <WiFi.h>
#include <WiFiClient.h>

#include <Temboo.h>
#include "TembooAccount.h" // Contains Temboo account information

WiFiClient client;

// The number of times to trigger the action if the condition is met
// We limit this so you won't use all of your Temboo calls while testing
int maxCalls = 10;

// The number of times this Choreo has been run so far in this sketch
int calls = 0;

int outputPin = 29;
int inputPin = 6;

void setup() {
  Serial.begin(9600);
  
  int wifiStatus = WL_IDLE_STATUS;

  // Determine if the WiFi Shield is present
  Serial.print("\n\nShield:");
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("FAIL");

    // If there's no WiFi shield, stop here
    while(true);
  }

  Serial.println("OK");

  // Try to connect to the local WiFi network
  while(wifiStatus != WL_CONNECTED) {
    Serial.print("WiFi:");
    wifiStatus = WiFi.begin(WIFI_SSID);
    if (wifiStatus == WL_CONNECTED) {
      Serial.println("OK");
    } else {
      Serial.println("FAIL");
    }
    delay(5000);
  }

  // Initialize condition pins
  pinMode(outputPin, OUTPUT);
  pinMode(inputPin, INPUT);
  Serial.println("Setup complete.\n");
}

void loop() {
  int sensorValue = analogRead(inputPin);
  Serial.println("Sensor: " + String(sensorValue));
  if (sensorValue == 100) {
    if (calls < maxCalls) {
      Serial.println("\nTriggered! Calling CaptureTextToSpeechPrompt Choreo...");
      runCaptureTextToSpeechPrompt(sensorValue);
      calls++;
    } else {
      Serial.println("\nTriggered! Skipping to save Temboo calls. Adjust maxCalls as required.");
    }
  }
  delay(250);
}

void runCaptureTextToSpeechPrompt(int sensorValue) {
  TembooChoreo CaptureTextToSpeechPromptChoreo(client);

  // Set Temboo account credentials
  CaptureTextToSpeechPromptChoreo.setAccountName(TEMBOO_ACCOUNT);
  CaptureTextToSpeechPromptChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
  CaptureTextToSpeechPromptChoreo.setAppKey(TEMBOO_APP_KEY);

  // Set profile to use for execution
  CaptureTextToSpeechPromptChoreo.setProfile("elec");
  // Identify the Choreo to run
  CaptureTextToSpeechPromptChoreo.setChoreo("/Library/Nexmo/Voice/CaptureTextToSpeechPrompt");

  // Run the Choreo
  unsigned int returnCode = CaptureTextToSpeechPromptChoreo.run();

  // A return code of zero means everything worked
  if (returnCode == 0) {
    while (CaptureTextToSpeechPromptChoreo.available()) {
      String name = CaptureTextToSpeechPromptChoreo.readStringUntil('\x1F');
      name.trim();

      if (name == "CallbackData") {
        if (CaptureTextToSpeechPromptChoreo.findUntil("", "\x1E")) {
          digitalWrite(outputPin, HIGH);
          CaptureTextToSpeechPromptChoreo.find("\x1E");
        }
      }
      else {
        CaptureTextToSpeechPromptChoreo.find("\x1E");
      }
    }
  }
  CaptureTextToSpeechPromptChoreo.close();
}