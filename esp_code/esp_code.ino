#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// Unique IDs for Bluetooth sync (DO NOT CHANGE)
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

const int MOTOR_PIN = 12;      // GPIO 12 connected to the vibrator
const int STATUS_LED = 2;      // Built-in blue LED for connection status

bool deviceConnected = false;
bool isVibrating = false;
unsigned long lastBlinkTime = 0;
bool ledState = false;

// Handle device Bluetooth connection and disconnection
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        digitalWrite(STATUS_LED, HIGH); // Built-in LED turns solid when paired
        Serial.println("✅ Web App Connected!");
    }

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        isVibrating = false;
        digitalWrite(MOTOR_PIN, LOW);   // Turn off motor
        Serial.println("❌ Web App Disconnected. Scanning again...");
        pServer->startAdvertising();    // Restart Bluetooth advertising
    }
};

// Handle incoming signal from web app
class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        String value = pCharacteristic->getValue();
        if (value.length() > 0) {
            char command = value[0];
            
            if (command == '1') {
                isVibrating = true;
                digitalWrite(MOTOR_PIN, HIGH); // Turn haptic motor ON
                Serial.println("📳 HAPTIC ALERTS ON");
            } else if (command == '0') {
                isVibrating = false;
                digitalWrite(MOTOR_PIN, LOW);  // Turn haptic motor OFF
                Serial.println("⏸️ HAPTIC ALERTS OFF");
            }
        }
    }
};

void setup() {
    Serial.begin(115200);
    
    pinMode(MOTOR_PIN, OUTPUT);
    pinMode(STATUS_LED, OUTPUT);
    digitalWrite(MOTOR_PIN, LOW); // Start with motor turned off
    digitalWrite(STATUS_LED, LOW);

    // Initialize BLE
    BLEDevice::init("NearBand Wearable");
    BLEServer* pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService* pService = pServer->createService(SERVICE_UUID);
    BLECharacteristic* pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );

    pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
    pCharacteristic->addDescriptor(new BLE2902());
    
    pService->start();

    // Start advertising so the web app can find the ESP32
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    BLEDevice::startAdvertising();

    Serial.println("📡 BLE Active! Device Name: 'NearBand Wearable'");
}

void loop() {
    unsigned long currentMillis = millis();

    // Visual pairing feedback on the built-in LED
    if (!deviceConnected) {
        // Blink built-in LED while waiting for connection
        if (currentMillis - lastBlinkTime >= 500) {
            lastBlinkTime = currentMillis;
            ledState = !ledState;
            digitalWrite(STATUS_LED, ledState ? HIGH : LOW);
        }
    } else {
        if (isVibrating) {
            // Blink very fast if alerts are vibrating
            if (currentMillis - lastBlinkTime >= 100) {
                lastBlinkTime = currentMillis;
                ledState = !ledState;
                digitalWrite(STATUS_LED, ledState ? HIGH : LOW);
            }
        } else {
            digitalWrite(STATUS_LED, HIGH); // Solid ON when connected and waiting
        }
    }
    delay(10);
}
