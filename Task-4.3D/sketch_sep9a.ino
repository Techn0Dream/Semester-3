/*
  Refactored SAMD + DHT22 example
  - Uses SAMDTimerInterrupt library for a hardware timer (1 Hz)
  - Button toggles LED1 (with debounce)
  - PIR toggles LED2 (with debounce)
  - Timer toggles LED3 every second
  - DHT22 temperature/humidity is read every 2 seconds

*/

#include <SAMDTimerInterrupt.h>  // Include timer library for SAMD-based boards
#include "DHT.h"               // Include DHT sensor library

// ----------------------------- PIN DEFINITIONS -----------------------------
// Using preprocessor macros (#define) to label physical pins with meaningful names
#define BUTTON_PIN 2   // Push button connected to digital pin 2 (uses internal pull-up)
#define PIR_PIN    3   // PIR motion sensor connected to digital pin 3
#define DHT_PIN    4   // DHT22 data pin connected to digital pin 4
#define LED1_PIN   8   // LED1 (toggled by button) connected to digital pin 8
#define LED2_PIN   9   // LED2 (toggled by PIR) connected to digital pin 9
#define LED3_PIN   10  // LED3 (toggled by timer) connected to digital pin 10

// ----------------------------- DHT SETUP ----------------------------------
#define DHTTYPE DHT22           // Define the DHT sensor type we are using
DHT dht(DHT_PIN, DHTTYPE);      // Create a DHT object bound to DHT_PIN and sensor type

// ----------------------------- TIMER SETUP --------------------------------
SAMDTimer ITimer0(TIMER_TC3);   // Timer object for periodic interrupts (choose TC3 for SAMD)

// ----------------------------- VOLATILE FLAGS ------------------------------
// Flags set inside ISRs (Interrupt Service Routines). Must be volatile
// because they are changed inside interrupts and read in the main loop.
volatile bool led1ToggleFlag = false; // Set by button ISR → processed in loop
volatile bool led2ToggleFlag = false; // Set by PIR ISR → processed in loop
volatile bool timerFlag      = false; // Set by timer ISR → processed in loop

// ----------------------------- LED STATES ----------------------------------
// Track the logical state of each LED so we can toggle them easily.
bool led1State = LOW; // Current state of LED1 (LOW or HIGH)
bool led2State = LOW; // Current state of LED2
bool led3State = LOW; // Current state of LED3

// ----------------------------- DEBOUNCE TIMERS -----------------------------
// Track the last time an event was accepted to implement software debounce.
unsigned long lastButtonTime = 0; // Time (millis) when button was last accepted
unsigned long lastPirTime    = 0; // Time (millis) when PIR event was last accepted
unsigned long lastDHTRead    = 0; // Time (millis) when DHT was last read

// ----------------------------- ISR FUNCTIONS --------------------------------
// Important ISR rule: keep them short and safe (no Serial prints, no heavy processing).

// Button ISR: triggered on FALLING edge when the button is pressed
void handleButton() {
  // Simply set a flag for the main loop to handle the button.
  led1ToggleFlag = true;
}

// PIR ISR: triggered on RISING edge when motion is detected
void handlePIR() {
  // Just set a flag — main loop will apply debounce and toggle LED2.
  led2ToggleFlag = true;
}

// Timer ISR: called by the hardware timer at configured intervals (1 Hz here)
void TimerHandler() {
  // Set a flag so the main loop knows the timer tick has happened.
  timerFlag = true;
}

// ----------------------------- INITIALIZATION -------------------------------
// initializeSystem: configure Serial, pins, sensors, interrupts, and timer.
void initializeSystem() {
  // Start Serial for debugging and information output at 115200 baud.
  Serial.begin(115200);
  // Short delay to let Serial port open properly on some boards and IDEs.
  delay(2000);

  // Print a startup message to the Serial monitor so we know the program started.
  Serial.println("Starting program with SAMDTimerInterrupt (1s timer, DHT every 2s)...");

  // Configure LED pins as outputs so we can drive them HIGH/LOW.
  pinMode(LED1_PIN, OUTPUT); // LED1
  pinMode(LED2_PIN, OUTPUT); // LED2
  pinMode(LED3_PIN, OUTPUT); // LED3

  // Configure input pins
  // BUTTON_PIN uses internal pull-up, so button should connect to GND when pressed.
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // PIR sensor is a digital output -> set as INPUT (no pull-up used here).
  pinMode(PIR_PIN, INPUT);

  // Initialize the DHT sensor library (this may set up internal timings).
  dht.begin();
  // Small stabilization delay recommended for some DHT modules after power-up.
  delay(2000);

  // Attach external interrupts
  // digitalPinToInterrupt(BUTTON_PIN) returns the interrupt number for that pin.
  // FALLING triggers when the button pulls the line low (pressed with pull-up).
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButton, FALLING);

  // Attach PIR interrupt on RISING edge (when PIR output goes HIGH due to motion).
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), handlePIR, RISING);

  // Attach timer interrupt with 1,000,000 microseconds = 1 second interval.
  // attachInterruptInterval returns true on success.
  if (ITimer0.attachInterruptInterval(1000000, TimerHandler)) {
    // If successful, print confirmation to Serial.
    Serial.println("Timer initialized successfully (1s).");
  } else {
    // If failed, print an error message — you may need to check timer choice.
    Serial.println("Timer initialization failed!");
  }
}

// ----------------------------- PER-EVENT PROCESSING -------------------------
// Each function below is responsible for one clearly-defined task. The loop
// simply gets the current millis() and calls these functions in sequence.

// processButton: run in main loop to handle a button press flagged by the ISR
void processButton(unsigned long now) {
  // Check if ISR has flagged a button event AND debounce time has passed.
  // led1ToggleFlag is volatile because it's set in the ISR.
  if (led1ToggleFlag && (now - lastButtonTime > 200)) { // 200ms debounce
    // Clear the ISR flag early to avoid re-processing it.
    led1ToggleFlag = false;

    // Update the lastButtonTime to current time to implement debounce window.
    lastButtonTime = now;

    // Toggle the logical state of LED1 (flip true/false -> HIGH/LOW).
    led1State = !led1State;

    // Write the new state to the physical LED pin.
    digitalWrite(LED1_PIN, led1State);

    // Inform via Serial that the button press was handled and LED1 toggled.
    Serial.println("Button pressed → LED1 toggled");
  }
}

// processPIR: handle motion events signaled by the PIR ISR with debounce
void processPIR(unsigned long now) {
  // Only handle the PIR event if ISR flagged it and enough time has passed.
  if (led2ToggleFlag && (now - lastPirTime > 500)) { // 500ms debounce for PIR
    // Clear the PIR ISR flag to avoid duplicate handling.
    led2ToggleFlag = false;

    // Update the lastPirTime to the present millis() value.
    lastPirTime = now;

    // Toggle the logical state for LED2.
    led2State = !led2State;

    // Drive the LED2 pin with the new state.
    digitalWrite(LED2_PIN, led2State);

    // Log the event to Serial for debugging and demo purposes.
    Serial.println("PIR motion detected → LED2 toggled");
  }
}

// attemptDHTRead: read DHT22 values but only once every 2000 ms (2 seconds).
void attemptDHTRead(unsigned long now) {
  // Check if at least 2000 ms have elapsed since last successful read.
  if (now - lastDHTRead >= 2000) {
    // Update the timestamp immediately so that long reads don't cause faster repeats.
    lastDHTRead = now;

    // Read temperature (Celsius) from the DHT22
    float temp = dht.readTemperature();
    // Read humidity (percentage) from the DHT22
    float hum  = dht.readHumidity();

    // Validate readings: the DHT library returns NaN on failure.
    if (isnan(temp) || isnan(hum)) {
      // Print error message but do not attempt to recover in this simple example.
      Serial.println("Failed to read DHT22!");
    } else {
      // Print nicely formatted temperature and humidity to Serial.
      Serial.print("Timer → LED3 toggled | Temp: ");
      Serial.print(temp);
      Serial.print(" °C, Hum: ");
      Serial.print(hum);
      Serial.println(" %");
    }
  }
}

// processTimer: called when the timerFlag is set by the timer ISR
void processTimer(unsigned long now) {
  // Check whether the Timer ISR has set the flag
  if (timerFlag) {
    // Clear the timer flag as we will process this tick now.
    timerFlag = false;

    // Toggle the logical state of LED3 on every timer tick (1 Hz)
    led3State = !led3State;

    // Physically set LED3 pin to the new state.
    digitalWrite(LED3_PIN, led3State);

    // Attempt a DHT read (this function itself enforces the 2s interval)
    attemptDHTRead(now);
  }
}

// ----------------------------- MAIN PROGRAM --------------------------------

void setup() {
  // Call a single function to initialize hardware and peripherals.
  initializeSystem();
}

void loop() {
  // Always capture millis() once per loop to keep timing consistent.
  unsigned long now = millis();

  // Process each responsibility in sequence. Each function is short and
  // returns quickly — this keeps the main loop responsive.
  processButton(now);   // Handle button events (debounced)
  processPIR(now);      // Handle PIR motion events (debounced)
  processTimer(now);    // Handle timer tick: LED3 toggle + DHT reads

  // No delay() here — we want the loop to run as fast as possible and react
  // promptly to flags set by ISRs. All timing logic is based on millis().
}