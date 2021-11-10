"""ENGG3800 PC Configuration software."""

import datetime
import threading
import time
import tkinter as tk
from tkinter import filedialog, scrolledtext, ttk

import numpy as np
import serial
import soundfile
from PIL import Image
from serial.tools import list_ports


def get_com_ports():
    com_ports = list_ports.comports()
    return com_ports


class Application(tk.Frame): # pylint: disable=too-many-ancestors,too-many-instance-attributes
    """Main application class"""

    def __init__(self, master=None):
        super().__init__(master)
        self.master = master
        self.pack()
        self.setup()


    def setup(self):
        self.create_clock_frame()
        self.create_console_frame()

        # Create serial console and setup read loop
        self.buffer = ""
        self.current_console_line = 1
        self.active_com_port = None

        # Start serial input read thread
        serial_input_thread = threading.Thread(target=self.read_from_serial, daemon=True)
        serial_input_thread.start()

        self.file_path_bg = None
        self.file_path_wav = None

        self.selected_wav = None
        self.selected_bg = None

        # Update the PC time label in main loop
        self.update_pc_time()

        # Send USB marker once per second
        self.send_usb_marker()

        # Determine if serial read/write is already in progress
        self.com_port_busy = False


    def printline_to_console(self, line):
        self.text_terminal.insert(tk.END, "%s\n" % line)
        self.current_console_line += 2


    def create_clock_frame(self):
        self.frame_time = tk.Frame(self.master, pady=8)

        self.label_computer_time_text = tk.Label(self.frame_time)
        self.label_computer_time_text["text"] = "Current PC Time"
        self.label_computer_time_text.pack(fill=tk.X)

        self.label_computer_time = tk.Label(self.frame_time, padx=20, pady=5)
        self.label_computer_time.pack(fill=tk.X)

        self.button_sync_time = tk.Button(self.frame_time, pady=2)
        self.button_sync_time["text"] = "Sync Time"
        self.button_sync_time["command"] = self.sync_time
        self.button_sync_time.pack()

        self.label_set_alarm_text = tk.Label(self.frame_time, pady=2)
        self.label_set_alarm_text["text"] = "Set Alarm"
        self.label_set_alarm_text.pack()

        self.alarm_selector_frame = tk.Frame(self.frame_time)

        self.alarm_number_var = tk.StringVar()
        self.combo_alarm_number = ttk.Combobox(
            self.alarm_selector_frame,
            textvariable=self.alarm_number_var,
            width=2)
        self.combo_alarm_number['values'] = ('1', '2', '3', '4')
        self.combo_alarm_number.pack(pady=2)

        self.alarm_time_var = tk.StringVar()
        self.entry_alarm_time = tk.Entry(
            self.alarm_selector_frame,
            textvariable=self.alarm_time_var,
            width=10)
        self.entry_alarm_time.pack(pady=2)

        self.button_select_wav = tk.Button(self.alarm_selector_frame, pady=2)
        self.button_select_wav["text"] = "Set Alarm"
        self.button_select_wav["command"] = self.set_alarm
        self.button_select_wav.pack(side=tk.BOTTOM, pady=2)

        self.alarm_selector_frame.pack()

        self.label_select_wav = tk.Label(self.frame_time, pady=4)
        self.label_select_wav["text"] = "No File Selected"
        self.label_select_wav.pack()

        self.button_select_wav = tk.Button(self.frame_time, pady=1)
        self.button_select_wav["text"] = "Select WAV"
        self.button_select_wav["command"] = self.select_wav
        self.button_select_wav.pack()

        self.button_upload_wav = tk.Button(self.frame_time, pady=1)
        self.button_upload_wav["text"] = "Upload WAV"
        self.button_upload_wav["command"] = self.upload_wav
        self.button_upload_wav.pack()

        self.label_select_bg = tk.Label(self.frame_time, pady=4)
        self.label_select_bg["text"] = "No File Selected"
        self.label_select_bg.pack()

        self.button_select_bg = tk.Button(self.frame_time, pady=1)
        self.button_select_bg["text"] = "Select PNG"
        self.button_select_bg["command"] = self.select_bg
        self.button_select_bg.pack()

        self.button_upload_bg = tk.Button(self.frame_time, pady=1)
        self.button_upload_bg["text"] = "Upload PNG"
        self.button_upload_bg["command"] = self.upload_bg
        self.button_upload_bg.pack()

        self.frame_time.pack(side=tk.LEFT, fill=tk.BOTH)


    def set_alarm(self):
        alarm_number = None
        try:
            alarm_number = int(self.alarm_number_var.get())
        except ValueError:
            self.printline_to_console("Alarm error: incorrect or no alarm number given")
            return
        alarm_time = self.alarm_time_var.get()
        alarm_time = alarm_time.replace(':', '')

        if self.active_com_port is not None and not self.com_port_busy:
            self.active_com_port.write(("alarm:%d:%s\n" % (alarm_number, alarm_time)).encode())
            self.printline_to_console("Alarm %d set for time %s" % (alarm_number, alarm_time))
        elif self.active_com_port is None:
            self.printline_to_console(
                "Alarm error: could not send alarm time (no serial connection established)")
        elif self.com_port_busy:
            self.printline_to_console(
                "Alarm error: could not send alarm time (COM port busy)")


    def select_wav(self):
        self.file_path_wav = filedialog.askopenfilename(
            title="Select Sound File",
            filetypes=[("WAV files", "*.wav")])
        if self.file_path_wav is None or self.file_path_wav == '':
            self.file_path_wav = None
        else:
            try:
                # Process WAV file
                wav, _ = soundfile.read(self.file_path_wav)

                # Take left channel (if stereo)
                if len(wav[0]) > 1:
                    wav = np.delete(wav, 1, 1)
                wav = wav.flatten()

                # Normalize float32/64 WAV format into 8 bit int
                # Scale -1:1 float values to 0:255 integers
                # wav = [(frame + 1) * 255 / 2 for frame in wav]
                wav = (wav + 1) * 255 / 2
                wav = np.uint8(wav).tolist()

                # Create list of 256 byte pages
                pages = list()
                page = list()
                page_index = 0
                frame_index = 0
                for frame in wav:
                    if frame_index < 256:
                        page.append(frame)
                        frame_index = frame_index + 1
                    else:
                        pages.append(page)
                        page = list()
                        frame_index = 0
                        page_index = page_index + 1
                if len(page) > 0:
                    pages.append(page)
                    page_index = page_index + 1

                self.selected_wav = pages

                # Show file name in GUI
                self.label_select_wav["text"] = self.file_path_wav.split('/')[-1]
                self.printline_to_console("WAV file selected: %s" % self.label_select_wav["text"])
            except OSError:
                self.printline_to_console("WAV error: could not load selected WAV file")


    def upload_wav(self):
        if (self.selected_wav is not None and
                self.active_com_port is not None and
                not self.com_port_busy):
            self.printline_to_console("Sending WAV file...")
            try:
                # Send wav marker and number of pages to indicate start of WAV transfer
                self.active_com_port.write(
                    ('wav_begin:%d\n' % len(self.selected_wav)).encode())
                self.com_port_busy = True
            except serial.SerialTimeoutException:
                self.printline_to_console(
                    "WAV error: could not send WAV file (serial error)")
        elif self.selected_wav is None:
            self.printline_to_console(
                "WAV error: could not send WAV file (no file selected)")
        elif self.active_com_port is None:
            self.printline_to_console(
                "WAV error: could not send WAV file (no serial connection established)")
        elif self.com_port_busy:
            self.printline_to_console(
                "WAV error: could not send WAV file (COM port busy)")


    def select_bg(self):
        self.file_path_bg = filedialog.askopenfilename(
            title="Select Background Image",
            filetypes=[("PNG Files", "*.png")])
        if self.file_path_bg is not None and self.file_path_bg != '':
            try:
                with Image.open(self.file_path_bg) as image:
                    if image.size != (64,128):
                        self.printline_to_console("PNG error: can only accept 128x64px PNG files")
                        return

                    # Convert to numpy array and get number of channels
                    new_image = np.array(image, dtype=np.uint8)
                    channels = None
                    if len(new_image.shape) > 2:
                        channels = new_image.shape[2]
                    else:
                        channels = 1

                    # Average values on RGB axis to convert to greyscale
                    if channels == 4:
                        new_image = np.delete(new_image, 3, axis=2)
                        channels = 3
                    if channels == 3:
                        new_image = np.mean(new_image, axis=2)

                    # Convert float averages to int8
                    new_image = new_image.astype(np.uint8)

                    # Flatten the image into 1D list
                    new_image = new_image.flatten()
                    new_image = list(new_image)

                    # Take greyscale/converted RGB pixels and convert to monochrome
                    # using 50% threshold
                    new_image = [1 if pixel > 127 else 0 for pixel in new_image]

                    # Convert array of bits into array of bytes
                    byte_index = 0
                    pixel_byte = 0
                    pixel_bytes = list()
                    last_pixel_index = len(new_image) - 1
                    for pixel_index, pixel_bit in enumerate(new_image):
                        pixel_byte = pixel_byte | (pixel_bit << byte_index)
                        if byte_index == 7 or pixel_index == last_pixel_index:
                            pixel_bytes.append(pixel_byte)
                            pixel_byte = 0
                            byte_index = 0
                        else:
                            byte_index = byte_index + 1

                    self.selected_bg = pixel_bytes
                    self.label_select_bg["text"] = self.file_path_bg.split('/')[-1]
                    self.printline_to_console(
                        "PNG file selected: %s" % self.label_select_bg["text"])
            except OSError:
                self.printline_to_console("PNG error: could not load selected PNG file")


    def upload_bg(self):
        if (self.selected_bg is not None and
                self.active_com_port is not None and
                not self.com_port_busy):
            self.com_port_busy = True
            self.printline_to_console("Sending PNG file...")
            try:
                # Send 'png' markers to indicate incoming/sent PNG data
                # Send PNG pixels over serial as bytes
                self.active_com_port.write('png_begin'.encode())
                self.active_com_port.write(bytes(self.selected_bg))
                self.active_com_port.write('png_end\n'.encode())
                self.printline_to_console("PNG sent successfully")
            except serial.SerialTimeoutException:
                self.printline_to_console("PNG error: could not send PNG file (serial error)")
            self.com_port_busy = False
        elif self.selected_bg is None:
            self.printline_to_console("PNG error: no PNG file selected")
        elif self.active_com_port is None:
            self.printline_to_console("PNG error: no serial connection established")
        elif self.com_port_busy:
            self.printline_to_console("PNG error: COM port busy")


    def sync_time(self):
        current_time = datetime.datetime.now()
        current_time = current_time.strftime("time:%H%M%S")
        current_time += '\n'
        if self.active_com_port is not None and not self.com_port_busy:
            self.com_port_busy = True
            try:
                self.active_com_port.write(current_time.encode())
                self.printline_to_console("Time synced successfully")
            except serial.SerialTimeoutException:
                self.printline_to_console(
                    "Time sync error: could not send time over serial connection")
            self.com_port_busy = False
        elif self.active_com_port is None:
            self.printline_to_console("Time sync error: serial connection not established")
        elif self.com_port_busy:
            self.printline_to_console("Time sync error: COM port busy")


    def change_port(self, *_):
        new_port = None
        try:
            # Get selected port from GUI dropdown
            selected_port = self.com_port_var.get().strip("()',")

            # Initialise serial comms with board
            new_port = serial.Serial(
                selected_port,
                baudrate=115200,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE)
            self.printline_to_console("Serial connection established")
        except serial.SerialException:
            self.printline_to_console("Error establishing serial connection")
        if self.active_com_port is not None:
            self.active_com_port.close()
            self.printline_to_console("Serial connection closed")
        self.active_com_port = new_port


    def create_readings_frame(self):
        self.frame_readings = tk.Frame(self.frame_console)

        self.label_accel_text = tk.Label(self.frame_readings)
        self.label_accel_text["text"] = "Accelerometer: "
        self.label_accel_text.pack(side=tk.LEFT)

        self.label_accel_x_text = tk.Label(self.frame_readings)
        self.label_accel_x_text["text"] = "X: "
        self.label_accel_x_text.pack(side=tk.LEFT)

        self.label_accel_x = tk.Label(self.frame_readings)
        self.label_accel_x.pack(side=tk.LEFT)

        self.label_accel_y_text = tk.Label(self.frame_readings)
        self.label_accel_y_text["text"] = "Y: "
        self.label_accel_y_text.pack(side=tk.LEFT)

        self.label_accel_y = tk.Label(self.frame_readings)
        self.label_accel_y.pack(side=tk.LEFT)

        self.label_accel_z_text = tk.Label(self.frame_readings)
        self.label_accel_z_text["text"] = "Z: "
        self.label_accel_z_text.pack(side=tk.LEFT)

        self.label_accel_z = tk.Label(self.frame_readings)
        self.label_accel_z.pack(side=tk.LEFT)

        self.label_light_text = tk.Label(self.frame_readings)
        self.label_light_text["text"] = "Light: "
        self.label_light_text.pack(side=tk.LEFT)

        self.label_light = tk.Label(self.frame_readings)
        self.label_light.pack(side=tk.LEFT)

        self.label_temp_text = tk.Label(self.frame_readings)
        self.label_temp_text["text"] = "Temperature: "
        self.label_temp_text.pack(side=tk.LEFT)

        self.label_temp = tk.Label(self.frame_readings)
        self.label_temp.pack(side=tk.LEFT)

        self.label_humidity_text = tk.Label(self.frame_readings)
        self.label_humidity_text["text"] = "Humidity: "
        self.label_humidity_text.pack(side=tk.LEFT)

        self.label_humidity = tk.Label(self.frame_readings)
        self.label_humidity.pack(side=tk.LEFT)

        self.label_volume_text = tk.Label(self.frame_readings)
        self.label_volume_text["text"] = "Volume: "
        self.label_volume_text.pack(side=tk.LEFT)

        self.label_volume = tk.Label(self.frame_readings)
        self.label_volume.pack(side=tk.LEFT)

        self.frame_readings.pack(side=tk.TOP, fill=tk.X)


    def create_console_frame(self):
        self.frame_console = tk.Frame(self.master, padx=2, pady=2)

        self.com_port_var = tk.StringVar(self.master)
        ports_unfiltered = get_com_ports()
        ports_filtered = [""] # make less hacky
        self.com_port_var.set(ports_filtered[0])
        # Get human readable list of com ports
        if len(ports_unfiltered) > 0:
            for port in ports_unfiltered:
                ports_filtered.append(port.device)

        self.frame_port_menu = tk.Frame(self.frame_console)

        self.label_mode_text = tk.Label(self.frame_port_menu)
        self.label_mode_text["text"] = "Selected mode: "
        self.label_mode_text.pack(side=tk.LEFT)

        self.label_mode = tk.Label(self.frame_port_menu)
        self.label_mode["text"] = ""
        self.label_mode.pack(side=tk.LEFT)

        self.menu_ports = tk.OptionMenu(
            self.frame_port_menu,
            self.com_port_var,
            *ports_filtered)
        self.menu_ports.pack(side=tk.RIGHT)

        self.frame_port_menu.pack(side=tk.TOP, fill=tk.X)

        self.create_readings_frame()

        # Trace com port variable to handle port change
        self.com_port_var.trace('w', self.change_port)

        self.text_terminal = scrolledtext.ScrolledText(
                self.frame_console,
                bg="black",
                padx=10,
                pady=10,
                fg="white")
        self.text_terminal.config(insertbackground="white")
        self.text_terminal.bind("<Return>", self.handle_input)
        self.text_terminal.pack(side=tk.BOTTOM, fill=tk.BOTH, expand=tk.YES)

        self.frame_console.pack(side=tk.BOTTOM, fill=tk.BOTH, expand=tk.YES)


    def update_pc_time(self):
        current_time = datetime.datetime.now()
        current_time = current_time.strftime("%H:%M:%S:%f")[:-3]
        self.label_computer_time.configure(text=current_time[:-4])
        self.master.after(50, self.update_pc_time)


    def send_usb_marker(self):
        # Send 'USB' marker to mcu to notify of pc connection
        if self.active_com_port is not None and not self.com_port_busy:
            self.com_port_busy = True
            try:
                self.active_com_port.write("USB\n".encode())
            except serial.SerialException:
                self.active_com_port.close()
                self.printline_to_console("Serial connection closed")
                self.active_com_port = None
            self.com_port_busy = False
        self.master.after(1000, self.send_usb_marker)


    def set_mode(self):
        if self.value_mode == 1:
            self.label_mode["text"] = "Stopwatch"
        elif self.value_mode == 2:
            self.label_mode["text"] = "Temperature/Humidity"
        elif self.value_mode == 3:
            self.label_mode["text"] = "Battery Voltage"
        elif self.value_mode == 4:
            self.label_mode["text"] = "Clock"
        elif self.value_mode == 5:
            self.label_mode["text"] = "Volume"


    def write_wav_page(self, requested_page):
        self.active_com_port.write(("wav:").encode())
        self.active_com_port.write(bytes(self.selected_wav[requested_page]))
        self.active_com_port.write("\n".encode())


    def process_sensor_values(self):
        if self.buffer.find('accel_x:') != -1:
            self.value_accelerometer_x = float(self.buffer[8:-1])/13.06
            self.label_accel_x["text"] = str("%.2fm/s^2" % self.value_accelerometer_x)
        elif self.buffer.find('accel_y:') != -1:
            self.value_accelerometer_y = float(self.buffer[8:-1])/13.06
            self.label_accel_y["text"] = str("%.2fm/s^2" % self.value_accelerometer_y)
        elif self.buffer.find('accel_z:') != -1:
            self.value_accelerometer_z = float(self.buffer[8:-1])/13.06
            self.label_accel_z["text"] = str("%.2fm/s^2" % self.value_accelerometer_z)
        elif self.buffer.find('light:') != -1:
            self.value_light = int(self.buffer[-7:-1])
            self.label_light["text"] = str("%slx" % self.value_light)
        elif self.buffer.find('temp:') != -1:
            self.value_temp = float(self.buffer[-6:-1])
            self.label_temp["text"] = str("%s°C" % self.value_temp)
        elif self.buffer.find('humid:') != -1:
            self.value_humidity = float(self.buffer[-6:-1])
            self.label_humidity["text"] = str("%s%%" % self.value_humidity)
        elif self.buffer.find('volume:') != -1:
            self.value_volume = int(self.buffer[-3:-1])
            self.label_volume["text"] = str(self.value_volume)
        elif self.buffer.find('mode:') != -1:
            self.value_mode = int(self.buffer[-2:-1])
            self.set_mode()
        else:
            return False
        return True


    def read_from_serial(self):
        while True:
            try:
                if self.active_com_port is not None and self.active_com_port.in_waiting > 0:
                    self.buffer = self.active_com_port.read_until().decode()
                    # Process serial input from mcu
                    if not self.process_sensor_values():
                        if (self.buffer.find('wav:') != -1 and
                                self.selected_wav is not None):
                            requested_page = int(self.buffer[-4:-1])
                            self.write_wav_page(requested_page)
                            if requested_page == (len(self.selected_wav) - 1):
                                self.printline_to_console("WAV file sent successfully")
                                self.com_port_busy = False
                            else:
                                self.printline_to_console("Sent page: %d of %d" % (requested_page, len(self.selected_wav) - 1))
                        elif self.buffer.find('USB') != -1:
                            continue
                        else:
                            self.printline_to_console("%s" % self.buffer)
                    # Clear buffer after processing input
                    self.buffer = ""
                elif self.active_com_port is None:
                    time.sleep(0.5)
            except UnicodeDecodeError:
                self.printline_to_console("Serial error: could not decode received input")
            except ValueError:
                self.printline_to_console("Serial error: could not interpret received input")
                self.com_port_busy = False
            except serial.SerialException:
                self.active_com_port = None
                self.printline_to_console("Serial connection closed")


    def handle_input(self, _):
        new_console_input = self.text_terminal.get('end-1c linestart', 'end-1c')
        if self.active_com_port is not None and not self.com_port_busy:
            self.com_port_busy = True
            try:
                self.active_com_port.write((new_console_input + '\n').encode())
                self.printline_to_console("Sent input: %s" % new_console_input)
            except serial.SerialException:
                self.printline_to_console("Error sending input: serial error")
            self.com_port_busy = False
        else:
            self.printline_to_console("Error sending input: no serial connection established")
        self.current_console_line += 1


if __name__ == "__main__":
    root = tk.Tk()
    root.geometry("850x450")
    app = Application(master=root)
    app.master.title("ENGG3800 Configuration Software")
    app.mainloop()
