# coding=utf-8

from datetime import datetime
import serial
import time

class ARDUINO:

    columns = ['timestamp', 'voltage', 'current', 'power', 'power_factor', 'frequency']

    def __init__(self, com="/dev/ttyUSB0", timeout=1.0, period=1000, verbose=False):  # Usb serial port
        self.arduino = serial.Serial(
            port=com,
            baudrate=38400,
            timeout=timeout
        )
        self.init_ts = int(datetime.now().timestamp() * 1000)
        self.ts = self.init_ts
        self.verbose = verbose
        # set the sample period
        time.sleep(2)
        self.config(period)
        # clean buffer
        self.arduino.readline()

    def config(self, period):
        params = str(period) + "\n"
        self.arduino.write(params.encode('utf-8'))

    def read(self):
        raw_line = self.arduino.readline()
        if raw_line:
            str_values = raw_line[:-1].decode().split(',')
            try:
                self.ts = self.init_ts + int(str_values[0])
                str_values[0] = self.ts
                for i in range(1, len(str_values)):
                    str_values[i] = float(str_values[i])
                return str_values
            except ValueError:
                if self.verbose:
                    print('ValueError')
        return None

    def close(self):
        self.config(0)
        self.arduino.close()
