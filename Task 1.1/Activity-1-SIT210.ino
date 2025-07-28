// Morse Code
const int buttonPin = 2;
const int ledPin = LED_BUILTIN;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
}
void loop() {
  if (isButtonPressed()) {
    blinkMorse("YOGYA");
    delay(2000); 
  }
}
bool isButtonPressed() {
  return digitalRead(buttonPin) == LOW;
}
void blinkDot() {
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);
  delay(1000);
}
void blinkDash() {
  digitalWrite(ledPin, HIGH);
  delay(3000);
  digitalWrite(ledPin, LOW);
  delay(1000);
}
void blinkLetter(const char* morseCode) {
  for (int i = 0; morseCode[i] != '\0'; i++) {
    if (morseCode[i] == '.') blinkDot();
    else if (morseCode[i] == '-') blinkDash();
  }
  delay(1000); 
}
const char* getMorseCode(char c) {
  switch (toupper(c)) {
    case 'A': return ".-";
    case 'G': return "--.";
    case 'O': return "---";
    case 'Y': return "-.--";
    default: return "";
  }
}
void blinkMorse(String word) {
  for (int i = 0; i < word.length(); i++) {
    const char* morse = getMorseCode(word[i]);
    blinkLetter(morse);
  }
}
