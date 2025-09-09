import tkinter as tk
from tkinter import ttk
import RPi.GPIO as GPIO
import time
import asyncio
import platform

# GPIO Setup
LED_PIN = 18  # Example GPIO pin, adjust as needed
GPIO.setmode(GPIO.BCM)
GPIO.setup(LED_PIN, GPIO.OUT)
pwm = GPIO.PWM(LED_PIN, 100)  # 100 Hz frequency
pwm.start(0)

# Timer function for SIT730 students (for multiple lights)
def start_intensity_change():
    while True:
        for duty in range(0, 101, 1):
            pwm.ChangeDutyCycle(duty)
            time.sleep(0.01)
        for duty in range(100, -1, -1):
            pwm.ChangeDutyCycle(duty)
            time.sleep(0.01)

async def main():
    root = tk.Tk()
    root.title("LED Intensity Control")

    # Slider 1 for intensity control
    slider1 = ttk.Scale(root, from_=0, to=100, orient=tk.HORIZONTAL, length=300, command=lambda val: pwm.ChangeDutyCycle(float(val)))
    slider1.set(0)
    slider1.pack(pady=10)

    # Slider 2 for intensity control
    slider2 = ttk.Scale(root, from_=0, to=100, orient=tk.HORIZONTAL, length=300, command=lambda val: pwm.ChangeDutyCycle(float(val)))
    slider2.set(0)
    slider2.pack(pady=10)

    # Slider 3 for intensity control
    slider3 = ttk.Scale(root, from_=0, to=100, orient=tk.HORIZONTAL, length=300, command=lambda val: pwm.ChangeDutyCycle(float(val)))
    slider3.set(0)
    slider3.pack(pady=10)

    # Start timer for intensity change (extend for multiple LEDs)
    if hasattr(root, 'after'):
        root.after(1000, start_intensity_change)

    root.mainloop()

    pwm.stop()
    GPIO.cleanup()

if platform.system() == "Emscripten":
    asyncio.ensure_future(main())
else:
    if __name__ == "__main__":
        asyncio.run(main())