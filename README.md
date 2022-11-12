# Home Power Logger
Power consumption logger using the PZEM-004T AC module.

## Usage ##

python logger <port> <filename> [-h] [-v] [-p <period>]

## Positional arguments
        port                    Serial port.
        filename                Output file name.

## Positional arguments
        -v, --verbose           Print records on screen.
        -h, --verbose           Print help info.
        -p, --period            Sample period in milliseconds.

## Notes
        List available serial ports with the follow command (Linux): dmesg | grep tty

## Example
Reading every second: 
        python logger /dev/ttyUSB0 output.csv -v -p 1000
