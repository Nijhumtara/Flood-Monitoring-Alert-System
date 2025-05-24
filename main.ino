#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define pins for GSM and Ultrasonic sensor
#define TRIG_PIN 9  // Ultrasonic Trigger Pin
#define ECHO_PIN 10 // Ultrasonic Echo Pin

// OLED display settings
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C // I2C address of the OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Initialize SoftwareSerial for GSM module
SoftwareSerial mySerial(3, 2); // RX, TX for GSM

// Variables for Ultrasonic sensor
long duration;
int distance;
const int FLOOD_THRESHOLD = 50; // Threshold distance for flood alert (in cm)

void setup() {
  // Initialize Serial communication
  Serial.begin(9600);
  mySerial.begin(9600);

  // Print initialization message
  Serial.println("Initializing...");
  delay(1000);

  // Initialize GSM module
  mySerial.println("AT");
  updateSerial();

  mySerial.println("AT+CMGF=1"); // Set SMS to text mode
  updateSerial();

  // Initialize Ultrasonic sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // Clear the OLED display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("System Initialized");
  display.display();
  delay(2000);
}

void loop() {
  // Measure distance using the Ultrasonic sensor
  measureDistance();

  // Display the water level on the OLED screen
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel size
  display.setTextColor(SSD1306_WHITE);  // White text
  display.setCursor(0, 0);     // Start at top-left corner
  display.print("Water Level: ");
  display.print(distance);
  display.println(" cm");

  // Check if the distance is below the flood threshold
  if (distance < FLOOD_THRESHOLD) {
    display.setCursor(0, 10);
    display.println("Flood Alert!");
    Serial.println("Flood Alert! Sending SMS...");
    sendAlertSMS(distance);
    delay(10000); // Wait 10 seconds before sending another SMS
  } else {
    display.setCursor(0, 10);
    display.println("No Flood Detected");
  }

  display.display(); // Update the OLED screen with new information

  delay(2000); // Wait for 2 seconds before the next measurement
}

void measureDistance() {
  // Trigger the ultrasonic sensor
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure the echo time
  duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate distance in cm
  distance = duration * 0.034 / 2;
}

void sendAlertSMS(int floodLevel) {
  // Send SMS with flood alert
  mySerial.println("AT+CMGS=\"type phone number here\""); // Replace with your phone number
  updateSerial();

  mySerial.print("Flood Alert! Water level is ");
  mySerial.print(floodLevel);
  mySerial.println(" cm. ");
  updateSerial();

  mySerial.write(26); // Send CTRL+Z to complete the message
}

void updateSerial() {
  delay(500);
  while (Serial.available()) {
    mySerial.write(Serial.read()); // Forward Serial data to GSM module
  }
  while (mySerial.available()) {
    Serial.write(mySerial.read()); // Forward GSM data to Serial Monitor
  }
}