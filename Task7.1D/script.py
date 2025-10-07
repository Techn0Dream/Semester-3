import speech_recognition as sr # pip install SpeechRecognition, This library allows to use speech to text features
import RPi.GPIO as GPIO # pip install RPi.GPIO, This library allows to control GPIO pins of Raspberry Pi
import time # This library allows to use time related functions
import pyttsx3 # pip install pyttsx3, This library allows to use text to speech features

LED_PIN = 18 # GPIO pin where the LED is connected
GPIO.setmode(GPIO.BCM) # Use BCM pin numbering
GPIO.setup(LED_PIN, GPIO.OUT) # Set LED_PIN as output
GPIO.output(LED_PIN, GPIO.LOW) # Ensure LED is off initially

r = sr.Recognizer() # Create a recognizer object to process the audio and recognize the speech
tts = pyttsx3.init() # Initialize the text-to-speech engine so that it can be used later to give voice feedback

microphone = sr.Microphone(device_index=2) # Sets up the microphone for recording with the correct device index

def listen_for_command(): # Function to listen for a voice command and return it as text
    with microphone as source: # this line opens the microphone and starts listening for audio input
        r.adjust_for_ambient_noise(source) # Adjusts for ambient noise to improve recognition accuracy
    print("Listening for command...")
    try:
        with microphone as source: # Opens the microphone and starts listening for audio input
            audio = r.listen(source, timeout=10) # Listen for up to 10 seconds for a command
        command = r.recognize_google(audio).lower() # Use Google's speech recognition to convert audio to text and makes it in lower case
        print(f"Recognized: {command}") # Print the recognized command
        return command
    except sr.UnknownValueError: # Exception handling for unrecognized speech
        print("Could not understand audio")
        return None
    except sr.RequestError: # Exception handling for API request errors
        print("Could not request results")
        return None

def control_led(command): # Function to control the LED based on the command
    if "turn on" in command or "switch on" in command: # Check if the command is to turn on the LED
        GPIO.output(LED_PIN, GPIO.HIGH) # Turn on the LED
        print("LED turned ON") 
        tts.say("Light is now on") # this tells what we want to say
        tts.runAndWait() # this runs the speech engine and waits until it finishes saying
        return True
    elif "turn off" in command or "switch off" in command:
        GPIO.output(LED_PIN, GPIO.LOW)
        print("LED turned OFF")
        tts.say("Light is now off")
        tts.runAndWait()
        return True
    return False

def main(): # Main function to run the voice-controlled LED system
    try:
        print("Voice-controlled LED system started")
        tts.say("Voice control system ready")
        tts.runAndWait()
        while True:
            command = listen_for_command() 
            if command:
                if control_led(command):
                    continue
                elif "exit" in command or "quit" in command:
                    print("Exiting system.")
                    tts.say("Exiting system")
                    tts.runAndWait()
                    break
                else:
                    print("Command not recognized")
                    tts.say("Command not recognized")
                    tts.runAndWait()
            time.sleep(1)
    except KeyboardInterrupt: 
        print("\nProgram interrupted")
    finally:
        GPIO.cleanup() 
        print("GPIO cleaned up")

if _name_ == "_main_": 
    main()