#include <ESP32Servo.h>
#include <Adafruit_SSD1306.h>

// Definiera komponenter
Servo motorServo;
Adafruit_SSD1306 display(128, 64, &Wire);

// FreeRTOS Queues och Mutex
QueueHandle_t motorQueue;
QueueHandle_t responseQueue;
SemaphoreHandle_t serialMutex;

// Variabler för status
float fuelLevel = 50.0; // Exempelstartvärde
bool ventilationOK = true;
int motorSpeed = 90;
int motorRPM = 2500;

// GPIO-pinnar
const int fuelLED = 5;  // LED för bränslenivå
const int fuelSensor = 32; // Potentiometer för bränslenivå

// Struktur för meddelanden
struct Message {
    char sender[15];
    char content[50];
    bool status;
};

// Funktion för att skicka meddelanden till Serial Monitor (med mutex-skydd)
void writeToSerial(const char *message) {
    if (serialMutex != NULL && xSemaphoreTake(serialMutex, pdMS_TO_TICKS(100))) {
        Serial.println(message);
        xSemaphoreGive(serialMutex);
    }
}

// Funktion för att uppdatera display
void updateDisplay(const char *title, const char *content) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(title);
    display.println(content);
    display.display();
}

void motorTask(void *pvParameters) {
    for (;;) {
        // Skicka självkontrollmeddelande
        writeToSerial("Checking motor...");
        if (motorSpeed > 0 && motorRPM > 0) {
            char output[50];
            snprintf(output, sizeof(output), "M.G is ok, speed %d and rpm %d", motorSpeed, motorRPM);
            writeToSerial(output);
            updateDisplay("Motor Check", output);
        } else {
            writeToSerial("x01:Error: M.||Gb.");
            updateDisplay("Motor Check", "x01:Error: M.||Gb.");
        }

        // Skicka fråga till ventilation
        Message ventMessage = {"Motor", "Checking vent.", true};
        xQueueSend(motorQueue, &ventMessage, portMAX_DELAY);

        // Skicka fråga till fuel
        Message fuelMessage = {"Motor", "Checking fuel", true};
        xQueueSend(motorQueue, &fuelMessage, portMAX_DELAY);

        // Vänta på svar från båda systemen
        Message response;
        for (int i = 0; i < 2; i++) {
            if (xQueueReceive(responseQueue, &response, pdMS_TO_TICKS(500))) {
                writeToSerial(response.content);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void ventilationTask(void *pvParameters) {
    for (;;) {
        Message receivedMessage;
        if (xQueueReceive(motorQueue, &receivedMessage, portMAX_DELAY)) {
            if (ventilationOK) {
                Message response = {"Ventilation", "Vent. Is ok", true};
                xQueueSend(responseQueue, &response, portMAX_DELAY);
                writeToSerial("Y*Y*");
                updateDisplay("Ventilation", "Vent. Is ok");
            } else {
                Message response = {"Ventilation", "x02:Error: Vent", false};
                xQueueSend(responseQueue, &response, portMAX_DELAY);
                writeToSerial("N*N*");
                updateDisplay("Ventilation", "x02:Error: Vent");
            }
        }
    }
}

void fuelTask(void *pvParameters) {
    pinMode(fuelLED, OUTPUT); // LED för bränslenivå

    for (;;) {
        Message receivedMessage;
        if (xQueueReceive(motorQueue, &receivedMessage, portMAX_DELAY)) {
            fuelLevel = analogRead(fuelSensor) * (100.0 / 4095.0); // Läst som % från potentiometer

            if (fuelLevel < 10.0) {
                digitalWrite(fuelLED, HIGH); // Tänd LED som varning
                Message response = {"Fuel", "0x3: low fuel", false};
                xQueueSend(responseQueue, &response, portMAX_DELAY);
                writeToSerial("U$U$");
                updateDisplay("Fuel Level", "Low Fuel!");
            } else {
                digitalWrite(fuelLED, LOW); // Släck LED om bränslenivån är bra
                Message response = {"Fuel", "0x4: good fuel", true};
                xQueueSend(responseQueue, &response, portMAX_DELAY);
                writeToSerial("h$h$");
                char buffer[20];
                snprintf(buffer, sizeof(buffer), "Fuel: %.2f%%", fuelLevel);
                updateDisplay("Fuel Level", buffer);
            }
        }
    }
}

void setup() {
    Serial.begin(9600);

    // Initiera Servo
    motorServo.attach(18);
    motorServo.write(0);

    // Initiera OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        Serial.println("OLED init failed!");
        for (;;);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Initiera potentiometer
    pinMode(fuelSensor, INPUT);

    // Initiera FreeRTOS Queues och Mutex
    motorQueue = xQueueCreate(5, sizeof(Message));
    responseQueue = xQueueCreate(5, sizeof(Message));
    serialMutex = xSemaphoreCreateMutex();

    // Starta FreeRTOS-uppgifter
    xTaskCreate(motorTask, "Motor Task", 2048, NULL, 1, NULL);
    xTaskCreate(ventilationTask, "Ventilation Task", 2048, NULL, 1, NULL);
    xTaskCreate(fuelTask, "Fuel Task", 2048, NULL, 1, NULL);
}

void loop() {
    
}
