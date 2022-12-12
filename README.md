# Home Power Logger
Power consumption logger using the PZEM-004T AC module, or ARDUINO meter.

## Usage ##

python logger <meter_type> <port> <filename> [-h] [-v] [-p <period>]

## Positional arguments
        meter_type              PZEM or ARDUINO.
        port                    Serial port.
        filename                Output file name.

## Positional arguments
        -v, --verbose           Print records on screen.
        -h, --verbose           Print help info.
        -p, --period            Sample period in milliseconds.

## Notes
        List available serial ports with the follow command (Linux): dmesg | grep tty

## Examples
Print help info:

        python3 logger

Reading every second:

        python3 logger ARDUINO /dev/ttyUSB0 output.csv -v -p 1000
