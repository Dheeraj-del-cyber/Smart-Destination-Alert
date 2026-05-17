!/*
 * =====================================================================================
 *            IoT SMART DESTINATION ALERT SYSTEM - WEARABLE WRISTBAND FIRMWARE
 * =====================================================================================
 * Microcontroller: ESP32 (NodeMCU / DevKit v1)
 * Communication:   Bluetooth Low Energy (BLE)
 * Power Supply:    3.7V Li-Po Battery (Rechargeable via TP4056 Module)
 * Haptic Output:   Vibration Motor (Switched via BC547 NPN Transistor)
 * 
 * --- HARDWARE ASSEMBLY & WIRING GUIDE ---
 * 
 * 1. POWER & CHARGING BLOCK (TP4056 & Li-Po)
 *    ┌─────────────────────┐          ┌───────────────────────┐
 *    │   Li-Po Battery     │          │      TP4056 Module    │
 *    │   [ Positive (+) ] ───────────►│ [ B+ ] (Battery +)    │
 *    │   [ Negative (-) ] ───────────►│ [ B- ] (Battery -)    │
 *    └─────────────────────┘          └───────────────────────┘
 *                                          │             │
 *                                      (OUT +)       (OUT -)
 *                                          │             │
 *                                          ▼             ▼
 *                                     [ SPDT Switch ]   GND (Common Ground)
 *                                          │
 *                                          ▼
 *                                    ESP32 VCC/VIN
 * 
 * 2. HAPTIC DRIVE CIRCUIT (Transistor Driver to prevent ESP32 GPIO burnout)
 *    - ESP32 GPIO 12 ─────► [ 1kΩ Resistor ] ─────► Base (B) of BC547 NPN Transistor
 *    - Emitter (E) of BC547 ──► GND (Common Ground)
 *    - Collector (C) of BC547 ──► Vibration Motor Negative (-) Lead
 *    - TP4056 OUT (+) / 3.3V ──► Vibration Motor Positive (+) Lead
 *    - Flyback Diode (1N4007) ──► Connect in parallel with motor leads (Anode to -, Cathode to +)
 * 
 * 3. CONNECTION INDICATOR
 *    - Built-in LED on GPIO 2 provides visual pairing state:
 *      * Fast Blink (500ms): Scanning / Advertising BLE
 *      * Solid ON: Connected to Smart Destination Web App
 *      * Double Blink: Vibrating (Active Alert)
 * =====================================================================================
 */

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// UUID configs matching the Web Bluetooth API settings
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// Hardware Pin Definitions
const int MOTOR_PIN = 12;      // GPIO pin connected to BC547 transistor base
const int STATUS_LED = 2;      // GPIO pin for ESP32 Built-in LED

// System State Variables
bool deviceConnected = false;
bool isVibrating = false;
unsigned long lastBlinkTime = 0;
bool ledState = false;

// Forward Declarations
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        digitalWrite(STATUS_LED, HIGH); // Solid ON when connected
        Serial.println("🌐 [BLE] Mobile App / Web App Connected!");
    }

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        isVibrating = false;
        digitalWrite(MOTOR_PIN, LOW); // Safeguard: stop motor on disconnect
        Serial.println("🔌 [BLE] Disconnected. Re-advertising...");
        pServer->startAdvertising();  // Restart advertising immediately
    }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            char command = value[0];
            Serial.print("📥 [BLE] Data Received: ");
            Serial.println(command);

            if (command == '1') {
                isVibrating = true;
                digitalWrite(MOTOR_PIN, HIGH);
                Serial.println("📳 [MOTOR] Vibration Motor ACTIVATED!");
            } else if (command == '0') {
                isVibrating = false;
                digitalWrite(MOTOR_PIN, LOW);
                Serial.println("⏸️ [MOTOR] Vibration Motor DEACTIVATED.");
            }
        }
    }
};

void setup() {
    Serial.begin(115200);
    Serial.println("🚀 Initializing Wearable Wristband Smart System...");

    // Pin configuration
    pinMode(MOTOR_PIN, OUTPUT);
    pinMode(STATUS_LED, OUTPUT);
    digitalWrite(MOTOR_PIN, LOW);
    digitalWrite(STATUS_LED, LOW);

    // 1. Initialize BLE Device
    BLEDevice::init("NearBand Wearable");

    // 2. Create BLE Server
    BLEServer* pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // 3. Create BLE Service
    BLEService* pService = pServer->createService(SERVICE_UUID);

    // 4. Create BLE Characteristic with WRITE properties
    BLECharacteristic* pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );

    // Attach callbacks to handle incoming vibration signals
    pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

    // Add descriptor for notifications
    pCharacteristic->addDescriptor(new BLE2902());

    // 5. Start the Service
    pService->start();

    // 6. Start Advertising
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // Android connection parameters
    pAdvertising->setMinPreferred(0x12);  // iOS connection parameters
    BLEDevice::startAdvertising();

    Serial.println("📡 [BLE] Bluetooth Advertising Started! Device Name: 'NearBand Wearable'");
    Serial.println("🔌 Ready for pairing with Chrome/Edge Web App...");
}

void loop() {
    unsigned long currentMillis = millis();

    // Manage status LED feedback loops
    if (!deviceConnected) {
        // Blink LED fast (500ms) to indicate pairing mode
        if (currentMillis - lastBlinkTime >= 500) {
            lastBlinkTime = currentMillis;
            ledState = !ledState;
            digitalWrite(STATUS_LED, ledState ? HIGH : LOW);
        }
    } else {
        if (isVibrating) {
            // Triple blink fast when vibrating
            if (currentMillis - lastBlinkTime >= 100) {
                lastBlinkTime = currentMillis;
                ledState = !ledState;
                digitalWrite(STATUS_LED, ledState ? HIGH : LOW);
            }
        } else {
            // Keep solid ON when connected and idle
            digitalWrite(STATUS_LED, HIGH);
        }
    }

    // Small delay to prevent watchdog reset
    delay(10);
}
