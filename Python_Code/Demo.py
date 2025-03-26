import serial
import time
import threading
import tkinter as tk
from tkinter import ttk, scrolledtext

class UNO_Serial:
    def __init__(self, port='COM3', baudrate=115200):
        try:
            self.arduino = serial.Serial(port=port, baudrate=baudrate, timeout=0.1)
            time.sleep(2)
            self.arduino.reset_input_buffer()
        except serial.SerialException:
            print(f"Could not open port {port}. Check device connection.")
            self.arduino = None

    def write(self, data):
        if self.arduino and self.arduino.is_open:
            if isinstance(data, int):
                data = self.format_input(data)
            self.arduino.write(bytes(data, 'ASCII'))
            print(f"[TX] {data.strip()}")

    def format_input(self, input_int):
        return f"${input_int}$;"

    def read_line(self):
        if self.arduino and self.arduino.is_open:
            return self.arduino.readline().decode('ASCII', errors='ignore').strip()
        return None

class SerialGUI:
    def __init__(self, master):
        self.master = master
        self.master.title("STM32 Serial Control Panel")
        self.master.geometry("700x500")

        self.serial = UNO_Serial()
        self.marquee1_running = False
        self.marquee2_running = False

        self.create_widgets()
        self.start_receiving()

    def create_widgets(self):
        input_frame = ttk.Frame(self.master)
        input_frame.pack(pady=10)

        self.input_label = ttk.Label(input_frame, text="Send Integer (0-65535):")
        self.input_label.pack(side=tk.LEFT, padx=5)

        self.input_entry = ttk.Entry(input_frame, width=20)
        self.input_entry.pack(side=tk.LEFT, padx=5)

        self.send_button = ttk.Button(input_frame, text="Send", command=self.send_single_value)
        self.send_button.pack(side=tk.LEFT, padx=5)

        control_frame = ttk.Frame(self.master)
        control_frame.pack(pady=5)

        self.marquee_button = ttk.Button(control_frame, text="Toggle Marquee", command=self.toggle_marquee1)
        self.marquee_button.pack(side=tk.LEFT, padx=10)

        self.marquee2_button = ttk.Button(control_frame, text="Toggle Marquee 2", command=self.toggle_marquee2)
        self.marquee2_button.pack(side=tk.LEFT, padx=10)

        # 按钮控制区
        self.button_frame = ttk.Frame(self.master)
        self.button_frame.pack(pady=10)

        self.button_states = [0] * 16
        self.bit_buttons = []
        for i in range(16):
            btn = tk.Button(self.button_frame, text=str(15 - i), width=2, height=1,
                            bg='lightgray', relief="raised",
                            command=lambda idx=i: self.toggle_bit(idx))
            btn.grid(row=0, column=i, padx=2)
            self.bit_buttons.append(btn)

        # LED 显示面板
        self.led_frame = ttk.Frame(self.master)
        self.led_frame.pack(pady=10)

        self.leds = []
        for i in range(16):
            led = tk.Label(self.led_frame, text=" ", bg='gray', width=2, height=1, borderwidth=1, relief="solid")
            led.grid(row=0, column=i, padx=2)
            self.leds.append(led)

        self.output_label = ttk.Label(self.master, text="Received Data:")
        self.output_label.pack()

        self.received_data_text = scrolledtext.ScrolledText(self.master, height=12, width=80, wrap=tk.WORD)
        self.received_data_text.pack(padx=10, pady=5, fill=tk.BOTH, expand=True)

    def toggle_bit(self, index):
        self.button_states[index] ^= 1  # Toggle
        new_state = self.button_states[index]

        self.bit_buttons[index].configure(
            bg='lime green' if new_state else 'lightgray'
        )

        val = sum((bit << i) for i, bit in enumerate(self.button_states))
        self.serial.write(val)
        self.update_leds(val)

    def reset_buttons(self):
        self.button_states = [0] * 16
        for btn in self.bit_buttons:
            btn.configure(bg='lightgray')

    def update_leds(self, val):
        binary = format(val, '016b')
        for i, bit in enumerate(reversed(binary)):
            color = 'lime green' if bit == '1' else 'gray'
            self.leds[i].configure(bg=color)

    def send_single_value(self):
        try:
            value = int(self.input_entry.get())
            if 0 <= value <= 65535:
                self.serial.write(value)
                self.update_leds(value)
                self.reset_buttons()  # Reset button states on manual send
            else:
                self.append_output("Value must be between 0 and 65535.\n")
        except ValueError:
            self.append_output("Invalid input. Please enter an integer.\n")

    def toggle_marquee1(self):
        if self.marquee1_running:
            self.marquee1_running = False
            self.marquee_button.config(text="Toggle Marquee")
        else:
            self.marquee2_running = False
            self.marquee2_button.config(text="Toggle Marquee 2")
            self.marquee1_running = True
            self.marquee_button.config(text="Stop Marquee")

            pattern = [7, 14, 28, 56, 112, 224, 448, 896, 1792, 3584, 7168, 14336,
                       28672, 57344, 28672, 14336, 7168, 3584, 1792, 896, 448, 224,
                       112, 56, 28, 14, 7]

            threading.Thread(target=self.run_marquee_generic, args=(pattern, 'marquee1_running'), daemon=True).start()

    def toggle_marquee2(self):
        if self.marquee2_running:
            self.marquee2_running = False
            self.marquee2_button.config(text="Toggle Marquee 2")
        else:
            self.marquee1_running = False
            self.marquee_button.config(text="Toggle Marquee")
            self.marquee2_running = True
            self.marquee2_button.config(text="Stop Marquee 2")

            pattern = [
                0b0000000000000001,
                0b0000000000000011,
                0b0000000000000111,
                0b0000000000001111,
                0b0000000000011111,
                0b0000000000111111,
                0b0000000001111111,
                0b0000000011111111,
                0b0000000111111111,
                0b0000001111111111,
                0b0000011111111111,
                0b0000111111111111,
                0b0001111111111111,
                0b0011111111111111,
                0b0111111111111111,
                0b1111111111111111,
                0b0111111111111111,
                0b0011111111111111,
                0b0001111111111111,
                0b0000111111111111,
                0b0000011111111111,
                0b0000001111111111,
                0b0000000111111111,
                0b0000000011111111,
                0b0000000001111111,
                0b0000000000111111,
                0b0000000000011111,
                0b0000000000001111,
                0b0000000000000111,
                0b0000000000000011,
                0b0000000000000001,
                0b0000000000000000,
            ]
            threading.Thread(target=self.run_marquee_generic, args=(pattern, 'marquee2_running'), daemon=True).start()

    def run_marquee_generic(self, pattern, flag_name):
        self.reset_buttons()  # Reset buttons when starting marquee
        while getattr(self, flag_name):
            for val in pattern:
                if not getattr(self, flag_name):
                    break
                self.serial.write(val)
                self.update_leds(val)
                time.sleep(0.2)
        self.update_leds(0)

    def start_receiving(self):
        threading.Thread(target=self.receive_data, daemon=True).start()

    def receive_data(self):
        while True:
            line = self.serial.read_line()
            if line:
                self.append_output(f"[RX] {line}\n")
            time.sleep(0.1)

    def append_output(self, text):
        self.received_data_text.insert(tk.END, text)
        self.received_data_text.see(tk.END)

if __name__ == '__main__':
    root = tk.Tk()
    app = SerialGUI(root)
    root.mainloop()
