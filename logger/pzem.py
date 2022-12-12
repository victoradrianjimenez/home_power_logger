# coding=utf-8

import serial
import modbus_tk.defines as cst
from modbus_tk import modbus_rtu
from datetime import datetime


class PZEM:

    columns = ['timestamp', 'voltage', 'current', 'power', 'energy', 'power_factor', 'frequency', 'alarm']

    def __init__(self, com="/dev/ttyUSB0", slave_address=0x01, timeout=2.0, period=1000, verbose=False):  # Usb serial port
        ser = serial.Serial(
            port=com,
            baudrate=9600,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS,
            xonxoff=0
        )
        self.master = modbus_rtu.RtuMaster(ser)
        self.master.set_timeout(timeout)
        self.master.set_verbose(verbose)
        self.slave_address = slave_address
        self.sample = [None]*len(self.columns)
        self.period = period
        self.ts = int(datetime.now().timestamp() * 1000)

    def read(self):
        while True:
            new_ts = int(datetime.now().timestamp() * 1000)
            if new_ts - self.ts < self.period:
                continue
            self.ts = new_ts

            d = self.master.execute(self.slave_address, cst.READ_INPUT_REGISTERS, 0, 10)
            self.sample[0] = new_ts
            self.sample[1] = d[0] / 10.0  # Volts
            self.sample[2] = (d[1] + (d[2] << 16)) / 1000.0  # Amperes
            self.sample[3] = (d[3] + (d[4] << 16)) / 10.0  # Watts
            self.sample[4] = d[5] + (d[6] << 16)  # Watt/hour
            self.sample[5] = d[8] / 100.0
            self.sample[6] = d[7] / 10.0  # Hertz
            self.sample[7] = d[9]  # 0 = no alarm
            return self.sample

    def close(self):
        self.master.close()
