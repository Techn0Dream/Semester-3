
from gpiozero import LED
import tkinter as tk
import sys

# --- Configure pins (BCM numbers) ---
RED_PIN = 17
GREEN_PIN = 27
BLUE_PIN = 22

red = LED(RED_PIN)
green = LED(GREEN_PIN)
blue = LED(BLUE_PIN)

def set_led():
    """Turn only the selected LED ON; turn others OFF."""
    sel = var.get()
    red.off(); green.off(); blue.off()
    if sel == 1:
        red.on()
    elif sel == 2:
        green.on()
    elif sel == 3:
        blue.on()

def on_exit():
    """Cleanup and exit safely."""
    try:
        red.off(); green.off(); blue.off()
        red.close(); green.close(); blue.close()
    except Exception:
        pass
    root.destroy()
    sys.exit(0)

# --- Build GUI ---
root = tk.Tk()
root.title("RPi 3-LED Controller")

var = tk.IntVar(value=0)   # 0 means none selected

tk.Label(root, text="Choose LED to turn ON (only one at a time):").pack(padx=10, pady=(10,2))

tk.Radiobutton(root, text="Red",   variable=var, value=1, command=set_led).pack(anchor='w', padx=20)
tk.Radiobutton(root, text="Green", variable=var, value=2, command=set_led).pack(anchor='w', padx=20)
tk.Radiobutton(root, text="Blue",  variable=var, value=3, command=set_led).pack(anchor='w', padx=20)

tk.Button(root, text="Exit", command=on_exit).pack(pady=12)

root.protocol("WM_DELETE_WINDOW", on_exit)  # handle window close
root.mainloop()
