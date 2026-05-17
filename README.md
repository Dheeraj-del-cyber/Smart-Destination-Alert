# Near | Smart Destination Alert

Near is a smart destination tracking web application designed to alert you when you are approaching your destination. It features real-time GPS tracking, interactive maps, and Bluetooth Low Energy (BLE) integration with a custom ESP32 wearable wristband for haptic feedback.

## Features

- **Real-Time GPS Tracking:** Uses your device's geolocation to track your progress toward your destination.
- **Demo Mode:** Simulates a journey if you are not currently traveling.
- **Interactive Map:** Built with Leaflet, utilizing satellite imagery for clear visual routing.
- **Wearable Integration:** Connects to an ESP32-based wristband via Web Bluetooth API to trigger physical vibrations when you are near your destination.
- **Customizable Alerts:** Set your preferred alert distance and choose from multiple alert sounds or upload your own.
- **Backend Simulation:** Node.js + Express backend to manage journey states and simulation routing.

## Project Structure

- `server.js`: Express backend server handling journey state, routing calculations, and simulation logic.
- `public/`: Frontend web application (HTML, CSS, JS).
  - `index.html`: The main web interface for map visualization and controls.
  - `style.css`: Custom styling and UI design.
- `firmware/esp32_wristband/`: Contains the Arduino/C++ firmware (`esp32_wristband.ino`) for the ESP32 wearable device.

## Hardware Setup (Wearable Wristband)

The project includes firmware for an ESP32-based wearable. The wristband connects to the web app via BLE and vibrates when you approach your destination.

**Components needed:**
- ESP32 (NodeMCU / DevKit v1)
- 3.7V Li-Po Battery & TP4056 Charging Module
- Vibration Motor
- BC547 NPN Transistor & 1kΩ Resistor
- 1N4007 Diode

**Instructions:**
1. Flash the `firmware/esp32_wristband/esp32_wristband.ino` onto your ESP32.
2. Follow the wiring guide detailed in the comments at the top of the `.ino` file.
3. Power on the device. The onboard LED will blink fast indicating it is ready to pair.

## Installation & Running Locally

1. **Install Dependencies:**
   Make sure you have Node.js installed.
   ```bash
   npm install
   ```

2. **Start the Server:**
   ```bash
   npm start
   ```

3. **Access the App:**
   Open your browser and navigate to `http://localhost:3000`. 
   
   *Note: For the Web Bluetooth API (BLE) and Live GPS features to work, the app must be served over HTTPS or accessed via `localhost`. We recommend using Google Chrome or Microsoft Edge.*

## Usage

1. Open the web app.
2. Search for your starting location or use "Live GPS" / "Demo GPS".
3. Search for a destination or tap on the map to set a pin.
4. Customize your alert distance and sound by clicking the settings (⚙️) icon.
5. If you have the ESP32 wristband, click **Connect** in the Wearable section and pair your device.
6. Click **START REAL** for live navigation or **DEMO** to simulate the journey!
