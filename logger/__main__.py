# coding=utf-8

__author__ = "V. Adrian Jimenez"
__copyright__ = "Copyright 2022, V. Adrian Jimenez"
__credits__ = ["V. Adrian Jimenez"]
__maintainer__ = "V. Adrian Jimenez"
__email__ = "victoradrianjimenez@gmail.com"
__status__ = "Production"
__version__ = "1.0.0"
__licence__ = "Custom licence"

import sys
import getopt
from datetime import datetime
from storage import CSVManager
from pzem import PZEM
from arduino import ARDUINO


class MainApp:
    # Options
    options = "hvp:"

    # Long options
    long_options = ["help", 'verbose', 'period=']

    def __init__(self):
        if len(sys.argv) < 2:
            self.show_help()
            exit(1)

        self.meter_type = sys.argv[1]
        self.com = sys.argv[2]
        self.filename = sys.argv[3]
        self.verbose = False
        self.period = 1000
        try:
            # Parsing argument
            arguments, values = getopt.getopt(sys.argv[4:], self.options, self.long_options)
            # checking each argument
            for currentArgument, currentValue in arguments:
                if currentArgument in ("-h", "--help"):
                    self.show_help()
                elif currentArgument in ("-v", "--verbose"):
                    self.verbose = True
                elif currentArgument in ("-p", "--period"):
                    self.period = int(currentValue)
        except getopt.error as err:
            # output error, and return with an error code
            print(str(err))
            self.show_help()
            exit(1)
        print('Parameters', self.meter_type, self.com, self.filename, self.verbose, self.period)

    @staticmethod
    def show_help():
        print("Home Power Meter Recorder")
        print("usage: python logger <meter_type> <port> <filename> [-h] [-v] [-p <period>]")
        print("\npositional arguments:")
        print("\tmeter_type\t\tPZEM or ARDUINO.")
        print("\tport\t\t\tSerial port.")
        print("\tfilename\t\tOutput file name.")
        print("\npositional arguments:")
        print("\t-v, --verbose\t\tPrint records on screen.")
        print("\t-p, --period\t\tSample period in milliseconds.")
        print("\nnotes:")
        # print("\tThe program needs root privileges to access the serial port, so you must use it with sudo.")
        print("\tList serial linux with the follow command: dmesg | grep tty")
        print("\nexample:")
        print("\treading every second: python3 logger ARDUINO /dev/ttyUSB0 output.csv -v -p 1000")
        print("")

    def start(self):
        meter = None
        try:
            if self.meter_type == 'PZEM':
                meter = PZEM(com=self.com, period=self.period, verbose=self.verbose)
            elif self.meter_type == 'ARDUINO':
                meter = ARDUINO(com=self.com, period=self.period, verbose=self.verbose)
            else:
                show_help()
                exit(1)

            if self.verbose:
                print(meter.columns)

            dm = CSVManager(self.filename, meter.columns)
            while True:
                ts = meter.ts
                row = meter.read()
                if row:
                    if len(row) == len(meter.columns):
                        dm.write(row)
                        if self.verbose:
                            print(row, meter.ts - ts)
                    #else:
                    #    print("Data Length Error.")
                #else:
                #    print("Null Data.")

        except KeyboardInterrupt:
            print('Exiting...')
        except Exception as e:
            print(e)
        finally:
            if meter:
                meter.close()

# Run!
app = MainApp()
app.start()
