# coding=utf-8

import serial
import modbus_tk.defines as cst
from modbus_tk import modbus_rtu


class PZEM:
    def __init__(self, com="/dev/ttyUSB0", slave_address=0x01, timeout=2.0, verbose=False):  # Usb serial port
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
        self.sample = dict()

    def read(self):
        d = self.master.execute(self.slave_address, cst.READ_INPUT_REGISTERS, 0, 10)
        self.sample['voltage'] = d[0] / 10.0  # Volts
        self.sample['current'] = (d[1] + (d[2] << 16)) / 1000.0  # Amperes
        self.sample['power'] = (d[3] + (d[4] << 16)) / 10.0  # Watts
        self.sample['energy'] = d[5] + (d[6] << 16)  # Watt/hour
        self.sample['frequency'] = d[7] / 10.0  # Hertz
        self.sample['power_factor'] = d[8] / 100.0
        self.sample['alarm'] = d[9]  # 0 = no alarm
        return self.sample

    def close(self):
        self.master.close()
