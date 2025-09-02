const int buttonPin = 2;   // Push button
const int pirPin    = 3;   // PIR sensor OUT
const int led1      = 6;   // LED controlled by button
const int led2      = 7;   // LED controlled by PIR

// Flags for ISR communication
volatile bool buttonEvent = false;
volatile bool pirEvent = false;

// LED states
volatile bool led1State = LOW;
volatile bool led2State = LOW;

void setup() {
  Serial.begin(115200);
  while (!Serial) { }  // wait for serial monitor to open (only needed on some boards)

  pinMode(buttonPin, INPUT_PULLUP); // button to GND
  pinMode(pirPin, INPUT);           // PIR output pin
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  // Attach interrupts
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonISR, FALLING); // press = LOW
  attachInterrupt(digitalPinToInterrupt(pirPin), pirISR, RISING);        // PIR pulse HIGH

  Serial.println("System ready: Button + PIR interrupts armed.");
}

void loop() {
  // Handle button event
  if (buttonEvent) {
    noInterrupts();
    buttonEvent = false;
    bool state = led1State;
    interrupts();

    Serial.print("Button pressed -> LED1 is ");
    Serial.println(state ? "ON" : "OFF");
  }

  // Handle PIR event
  if (pirEvent) {
    noInterrupts();
    pirEvent = false;
    bool state = led2State;
    interrupts();

    Serial.print("PIR triggered -> LED2 is ");
    Serial.println(state ? "ON" : "OFF");
  }

  delay(5); // avoid busy looping
}

void buttonISR() {
  led1State = !led1State;
  digitalWrite(led1, led1State);
  buttonEvent = true;
}

void pirISR() {
  led2State = !led2State;
  digitalWrite(led2, led2State);
  pirEvent = true;
}
