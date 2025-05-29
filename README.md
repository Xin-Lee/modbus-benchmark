# Modbus Throughput Test Tool

This tool allows you to test the throughput of both ModbusTCP and ModbusRTU devices by reading and/or writing to registers at a specified rate. It provides useful statistics about the device's performance.

## Prerequisites

- libmodbus library must be installed on your system
- A C++ compiler with C++11 support

## Building

To build the tool, run:

```bash
g++ -std=c++11 -o modbus_test main.cpp ../crane-car-read-modbus/modbus_control.cpp -lmodbus -pthread
```

This will create the executable `modbus_test`.

## Usage

```
./modbus_test [options]
```

### Command Line Options

| Option | Description |
|--------|-------------|
| `-h, --help` | Show help message |
| **Connection Type:** |
| `--tcp` | Use ModbusTCP mode (default) |
| `--rtu` | Use ModbusRTU mode |
| **TCP Options:** |
| `-i, --ip <address>` | ModbusTCP server IP address (default: 127.0.0.1) |
| `-p, --port <port>` | ModbusTCP server port (default: 502) |
| **RTU Options:** |
| `-d, --device <device>` | Serial device (default: /dev/ttyUSB0) |
| `-b, --baud <rate>` | Baud rate (default: 9600) |
| `--parity <N\|E\|O>` | Parity N(one), E(ven), O(dd) (default: N) |
| `--data-bits <bits>` | Data bits (default: 8) |
| `--stop-bits <bits>` | Stop bits (default: 1) |
| **Common Options:** |
| `-s, --slave <id>` | Modbus slave ID (default: 1) |
| `-r, --register <address>` | Starting register address (default: 0) |
| `-c, --count <count>` | Number of registers per operation (default: 10) |
| `-t, --duration <seconds>` | Test duration in seconds (default: 10) |
| `-m, --mode <mode>` | Test mode: 0=read, 1=write, 2=read+write (default: 0) |
| `-v, --interval <ms>` | Interval between operations in ms, 0=max speed (default: 0) |

## Example Usage

### ModbusTCP Examples

#### Test read throughput at maximum speed
```bash
./modbus_test --tcp -i 192.168.1.26 -r 100 -c 10 -t 30 -m 0
```
This will read 10 registers starting from address 100 as fast as possible for 30 seconds.

#### Test write throughput with an interval
```bash
./modbus_test --tcp -i 192.168.1.100 -r 100 -c 5 -t 20 -m 1 -v 50
```
This will write 5 registers starting from address 100 every 50ms for 20 seconds.

### ModbusRTU Examples

#### Test read throughput using a serial port
```bash
./modbus_test --rtu -d /dev/ttyUSB0 -b 9600 --parity N --data-bits 8 --stop-bits 1 -s 1 -r 100 -c 10 -t 30 -m 0
```
This will read 10 registers from a ModbusRTU device at address 100 for 30 seconds.

#### Test write throughput with different baud rate
```bash
./modbus_test --rtu -d /dev/ttyS0 -b 19200 --parity E -s 1 -r 100 -c 5 -t 20 -m 1
```
This will write 5 registers to a ModbusRTU device at address 100 for 20 seconds with 19200 baud rate and even parity.

#### Test both read and write operations
```bash
./modbus_test --rtu -d /dev/ttyUSB0 -s 1 -r 100 -c 10 -t 30 -m 2
```
This will perform both read and write operations on 10 registers for 30 seconds via ModbusRTU.

## Output

The tool will output:
1. The test configuration
2. Connection status
3. Progress information during the test
4. Test results including:
   - Total operations performed (reads and writes)
   - Operations per second
   - Data throughput in KB/sec
   - Number of failed operations (if any)

## Troubleshooting

### Common Issues for ModbusTCP
- Ensure the IP address and port are correct
- The ModbusTCP device is running and accessible from your machine
- There are no firewall rules blocking the connection

### Common Issues for ModbusRTU
- Verify the correct serial port (e.g., /dev/ttyUSB0, /dev/ttyS0)
- Ensure you have the correct baud rate, parity, data bits, and stop bits
- Check that you have the correct permissions to access the serial port
- For some systems, you may need to run with sudo to access serial ports
- Verify the cables and hardware connections

### General Issues
- The register addresses you're trying to access are valid for the device
- The slave ID is correct
- The device has enough registers to read/write the requested count 