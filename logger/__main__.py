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


class MainApp:
    # Options
    options = "hvp:"

    # Long options
    long_options = ["help", 'verbose', 'period=']

    def __init__(self):
        if len(sys.argv) < 2:
            self.show_help()
            exit(1)

        self.com = sys.argv[1]
        self.filename = sys.argv[2]
        self.verbose = False
        self.period = 1
        try:
            # Parsing argument
            arguments, values = getopt.getopt(sys.argv[3:], self.options, self.long_options)
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

    @staticmethod
    def show_help():
        print("Home Power Meter Recorder")
        print("usage: python logger <port> <filename> [-h] [-v] [-p <period>]")
        print("\npositional arguments:")
        print("\tport\t\t\tSerial port.")
        print("\tfilename\t\tOutput file name.")
        print("\npositional arguments:")
        print("\t-v, --verbose\t\tPrint records on screen.")
        print("\t-p, --period\t\tSample period in milliseconds.")
        print("\nnotes:")
        # print("\tThe program needs root privileges to access the serial port, so you must use it with sudo.")
        print("\tList serial linux with the follow command: dmesg | grep tty")
        print("\nexample:")
        print("\treading every second: python logger /dev/ttyUSB0 output.csv -v -p 1000")
        print("")

    @staticmethod
    def get_timestamp():
        return int(datetime.now().timestamp() * 1000)

    def start(self):
        meter = None
        ts = self.get_timestamp()
        columns = ['voltage', 'current', 'power', 'power_factor', 'frequency']
        try:
            meter = PZEM(com=self.com)
            dm = CSVManager(self.filename, columns=['timestamp'] + columns)
            while True:
                new_ts = self.get_timestamp()
                if new_ts - ts < self.period:
                    continue
                d = meter.read()
                row = [new_ts] + [d[k] for k in columns]
                dm.write(row)
                if self.verbose:
                    print(row, new_ts - ts)
                ts = new_ts

        except KeyboardInterrupt:
            print('Exiting...')
        except Exception as e:
            print(e)
        finally:
            meter.close()


# Run!
app = MainApp()
app.start()
