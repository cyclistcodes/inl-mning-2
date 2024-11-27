# Motor Control and Monitoring System

## Overview
This project involves a motor control and monitoring system built on the ESP32 platform, using FreeRTOS for task management. 
It reads data from a fuel sensor, checks the motor's status, and monitors the ventilation system.
The system uses an OLED display to provide real-time feedback and utilizes FreeRTOS queues and mutexes for communication and synchronization between tasks.

## Components
- **ESP32 Microcontroller**: Main controller.
- **Servo Motor**: Controlled using the ESP32 to simulate a motor.
- **OLED Display**: Displays status messages and values (fuel, ventilation, motor).
- **Fuel Sensor**: Reads the fuel level from a potentiometer.
- **Ventilation System**: Monitors ventilation status.
- **LED Indicator**: A red LED lights up when fuel level is low.

## Libraries
- `ESP32Servo`: Library for controlling servo motors.
- `Adafruit_SSD1306`: Library for OLED display.

## FreeRTOS Tasks
1. **Motor Task**: 
   - Checks motor status (speed and RPM).
   - Sends messages to check the ventilation and fuel systems.
   - Waits for responses from ventilation and fuel systems.
   
2. **Ventilation Task**: 
   - Monitors the ventilation system status.
   - Sends responses back to the motor task regarding the ventilation status.
   
3. **Fuel Task**: 
   - Monitors the fuel level via a potentiometer.
   - Lights up an LED when the fuel level is below 10% and sends a low fuel warning.

## Key Variables
- `fuelLevel`: Current fuel level (from 0 to 100%).
- `ventilationOK`: Status of the ventilation system.
- `motorSpeed`: Current speed of the motor (0-90 degrees).
- `motorRPM`: Revolutions per minute of the motor.

## GPIO Pin Setup
- **Fuel LED**: GPIO 5 (Lights up when fuel is low).
- **Fuel Sensor**: GPIO 32 (Analog input from a potentiometer).

## Display Information
- The OLED display shows the current status of the motor, ventilation, and fuel systems.
- The following statuses are displayed:
  - Motor health (speed and RPM).
  - Ventilation status (OK or Error).
  - Fuel level (percentage and warning when low).

## Mutex and Queues
- **Serial Mutex**: Ensures thread-safe communication to the Serial Monitor.
- **Motor Queue**: Used for sending status requests between the motor and other tasks.
- **Response Queue**: Used to send responses back from the ventilation and fuel tasks.

## Setup
1. **Servo Motor**: Attach the servo to GPIO 18 and control its angle.
2. **OLED Display**: Initialize the display at I2C address `0x3C`.
3. **Fuel Sensor**: Read the fuel level from an analog potentiometer connected to GPIO 32.
4. **FreeRTOS Queues and Mutex**: Create queues for inter-task communication and a mutex for Serial Monitor access.

## How to Run
1. **Upload** the code to the ESP32 using the Arduino IDE.
2. **Open the Serial Monitor** to view the status of the motor, fuel, and ventilation systems.
3. **Watch the OLED Display** for real-time updates.
4. The system will run continuously, checking the motor, fuel, and ventilation systems and displaying the results on the OLED screen.

## Troubleshooting
- If the OLED display doesn't initialize, check the I2C wiring and ensure the correct address (`0x3C`) is set.
- If the system reports an error with the motor, check the motor connections and ensure the servo is properly powered.
- Low fuel warnings will trigger the LED and display appropriate messages on the screen.