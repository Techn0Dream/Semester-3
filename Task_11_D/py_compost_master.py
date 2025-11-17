
import serial
import time
import RPi.GPIO as GPIO 
import pyrebase
from rpi_lcd import LCD # Library for I2C LCD control

# --- Configuration ---
SERIAL_PORT = '/dev/ttyACM0'  
BAUD_RATE = 9600

# GPIO Pin Setup (using BCM numbering)
GPIO.setmode(GPIO.BCM)
RELAY_PIN = 26 # GPIO 26 (Pin 37) controls the relay (Pi's Actuation)

# Scientific Thresholds (Rule-Based Logic - RBL)
TEMP_CRITICAL_HIGH = 55.0  # Overheating threshold
TEMP_LOW = 50.0            # Activity slowing down threshold
MOISTURE_DRY_THRESHOLD = 700 # Raw ADC reading: Value above this means too DRY 

# --- FIREBASE CONFIGURATION (Using User's Credentials) ---
FIREBASE_CONFIG = {
  "apiKey": "AIzaSyBrKOg9DmU20P8KYlDwTx5Z2yKi9MQv1i8",
  "authDomain": "compostmaestro.firebaseapp.com",
  "databaseURL": "https://compostmaestro-default-rtdb.firebaseio.com",
  "storageBucket": "compostmaestro.appspot.com"
}

# --- Initialization ---
try:
    # Initialize LCD 
    lcd = LCD() 
    lcd.backlight(True) 
    print("LCD initialized successfully.")
except OSError:
    print("LCD Error: Could not find device. Check I2C address/wiring.")
    lcd = None

try:
    # Initialize Firebase connection
    firebase = pyrebase.initialize_app(FIREBASE_CONFIG)
    db = firebase.database()
except Exception as e:
    print(f"Error initializing Firebase: {e}")
    db = None 

# Initialize GPIO pin for Relay control and ensure it starts OFF
GPIO.setup(RELAY_PIN, GPIO.OUT)
GPIO.output(RELAY_PIN, GPIO.HIGH) # HIGH signal = FAN OFF (Active-LOW assumed)


# --- Output and Logic Functions ---

def display_advice(diagnosis_msg):
    """Writes the diagnosis message to the 16x2 LCD screen."""
    if lcd is None: return
    
    lcd.clear()
    
    # Write to screen (ensuring long messages are split)
    line1 = diagnosis_msg[:16].strip()
    line2 = diagnosis_msg[16:].strip()
        
    lcd.text(line1, 1) # Row 1
    if line2:
        lcd.text(line2, 2) # Row 2


def execute_fan_policy(action):
    """Directly controls the relay based on the RBL output."""
    # This is the Pi's direct Actuation (GPIO 26)
    if action == "FAN_ON":
        # LOW activates the relay (Fan ON)
        GPIO.output(RELAY_PIN, GPIO.LOW)
    elif action == "FAN_OFF":
        # HIGH deactivates the relay (Fan OFF)
        GPIO.output(RELAY_PIN, GPIO.HIGH)
    
    return "ON" if action == "FAN_ON" else "OFF" # Return state for logging


def push_to_firebase(temp_c, moisture_raw, diagnosis, fan_state):
    """Pushes the current state and diagnosis to Firebase RTDB."""
    if db is None: return 

    try:
        data = {
            "timestamp": time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()),
            "temp_c": temp_c,
            "moisture_raw": moisture_raw,
            "diagnosis": diagnosis,
            "fan_state": fan_state,
            "device_id": "Compost_Maestro_Pi3"
        }
        db.child("readings").push(data)
    except Exception as e:
        print(f"Error pushing data to Firebase: {e}")
        
def run_diagnostic(temp_c, moisture_raw):
    """Runs the core Rule-Based Logic (RBL) to generate policy."""
    if temp_c >= TEMP_CRITICAL_HIGH:
        action = "FAN_ON"; display_msg = "CRIT: OVERHEAT! FAN ON"
    elif moisture_raw >= MOISTURE_DRY_THRESHOLD:
        action = "FAN_OFF"; display_msg = "WARNING: ADD WATER NOW"
    elif temp_c < TEMP_LOW and moisture_raw < MOISTURE_DRY_THRESHOLD:
        action = "FAN_OFF"; display_msg = "ALERT: TEMP LOW, TURN PILE"
    else:
        action = "FAN_OFF"; display_msg = f"OPTIMAL: TEMP {temp_c:.1f}C"
    return action, display_msg

# --- Main Program Loop ---
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2) 

    print(f"Master running. Now logging to {'Firebase' if db else 'Local Terminal'}.")

    # Write initial message to LCD
    if lcd:
        display_advice("Compost Maestro V2.0")
        time.sleep(2)

    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            
            if line.startswith("T"):
                try:
                    # --- 1. Data Parsing ---
                    parts = line.split('|')
                    temp_c = float(parts[0].replace('T', ''))
                    moisture_raw = int(parts[1].replace('M', ''))

                    # --- 2. RBL Execution (Diagnosis) ---
                    action, display_msg = run_diagnostic(temp_c, moisture_raw)

                    # --- 3. Actuation and Output ---
                    final_fan_state = execute_fan_policy(action)
                    display_advice(display_msg)   
                    
                    # --- 4. Logging ---
                    push_to_firebase(temp_c, moisture_raw, display_msg, final_fan_state)
                    
                    print(f"[{time.strftime('%H:%M:%S')}] {display_msg} (Fan: {final_fan_state})")

                except Exception as e:
                    print(f"Error processing data: {e} - Received Line: {line}")

        time.sleep(0.1) 

except serial.SerialException as e:
    print(f"\n[FATAL] Could not open serial port {SERIAL_PORT}.")
except KeyboardInterrupt:
    print("\nCompost Maestro Master Shutting Down.")
finally:
    # Clean up GPIO and shut down LCD backlight on exit
    if 'lcd' in locals() and lcd:
        lcd.backlight(False)
        lcd.clear()
    GPIO.cleanup()
    if 'ser' in locals() and ser.is_open:
        ser.close()