#include <iostream>
#include <unistd.h>
#include <string.h>
#include <chrono>
#include <vector>
#include <thread>
#include <iomanip>
#include "modbus_control.h"

// Configuration structure for test parameters
struct TestConfig {
    // Connection parameters
    bool isRTU = false;             // Connection type (false=TCP, true=RTU)
    // TCP parameters
    std::string ip = "127.0.0.1";   // Default IP address
    int port = 502;                 // Default ModbusTCP port
    // RTU parameters
    std::string device = "/dev/ttyUSB0"; // Default serial device
    int baudRate = 115200;            // Default baud rate
    char parity = 'N';              // Default parity (N, E, O)
    int dataBits = 8;               // Default data bits
    int stopBits = 1;               // Default stop bits
    // Common parameters
    int slave = 1;                  // Default slave ID
    int startRegister = 0;          // Starting register address
    int registerCount = 10;         // Number of registers to read/write in each operation
    int testDuration = 10;          // Test duration in seconds
    int testMode = 0;               // 0: read holding registers, 1: write registers, 2: read and write
    int interval = 0;               // Interval between operations in milliseconds (0 = as fast as possible)
};

// Function to display help
void showHelp(const char* programName) {
    std::cout << "Modbus Throughput Test Tool\n";
    std::cout << "Usage: " << programName << " [options]\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help                  Show this help message\n";
    std::cout << "  Connection Type:\n";
    std::cout << "  --tcp                       Use ModbusTCP mode (default)\n";
    std::cout << "  --rtu                       Use ModbusRTU mode\n";
    std::cout << "  TCP Options:\n";
    std::cout << "  -i, --ip <address>          ModbusTCP server IP address (default: 127.0.0.1)\n";
    std::cout << "  -p, --port <port>           ModbusTCP server port (default: 502)\n";
    std::cout << "  RTU Options:\n";
    std::cout << "  -d, --device <device>       Serial device (default: /dev/ttyUSB0)\n";
    std::cout << "  -b, --baud <rate>           Baud rate (default: 9600)\n";
    std::cout << "  --parity <N|E|O>            Parity N(one), E(ven), O(dd) (default: N)\n";
    std::cout << "  --data-bits <bits>          Data bits (default: 8)\n";
    std::cout << "  --stop-bits <bits>          Stop bits (default: 1)\n";
    std::cout << "  Common Options:\n";
    std::cout << "  -s, --slave <id>            Modbus slave ID (default: 1)\n";
    std::cout << "  -r, --register <address>    Starting register address (default: 0)\n";
    std::cout << "  -c, --count <count>         Number of registers per operation (default: 10)\n";
    std::cout << "  -t, --duration <seconds>    Test duration in seconds (default: 10)\n";
    std::cout << "  -m, --mode <mode>           Test mode: 0=read, 1=write, 2=read+write (default: 0)\n";
    std::cout << "  -v, --interval <ms>         Interval between operations in ms, 0=max speed (default: 0)\n";
}

// Function to parse command line arguments
bool parseArgs(int argc, char* argv[], TestConfig& config) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            showHelp(argv[0]);
            return false;
        } else if (arg == "--tcp") {
            config.isRTU = false;
        } else if (arg == "--rtu") {
            config.isRTU = true;
        } 
        // TCP options
        else if (arg == "-i" || arg == "--ip") {
            if (i + 1 < argc) {
                config.ip = argv[++i];
            } else {
                std::cerr << "Error: IP address argument missing\n";
                return false;
            }
        } else if (arg == "-p" || arg == "--port") {
            if (i + 1 < argc) {
                config.port = std::stoi(argv[++i]);
            } else {
                std::cerr << "Error: Port argument missing\n";
                return false;
            }
        } 
        // RTU options
        else if (arg == "-d" || arg == "--device") {
            if (i + 1 < argc) {
                config.device = argv[++i];
            } else {
                std::cerr << "Error: Device argument missing\n";
                return false;
            }
        } else if (arg == "-b" || arg == "--baud") {
            if (i + 1 < argc) {
                config.baudRate = std::stoi(argv[++i]);
            } else {
                std::cerr << "Error: Baud rate argument missing\n";
                return false;
            }
        } else if (arg == "--parity") {
            if (i + 1 < argc) {
                config.parity = argv[++i][0];
                if (config.parity != 'N' && config.parity != 'E' && config.parity != 'O') {
                    std::cerr << "Error: Parity must be N, E, or O\n";
                    return false;
                }
            } else {
                std::cerr << "Error: Parity argument missing\n";
                return false;
            }
        } else if (arg == "--data-bits") {
            if (i + 1 < argc) {
                config.dataBits = std::stoi(argv[++i]);
                if (config.dataBits != 7 && config.dataBits != 8) {
                    std::cerr << "Error: Data bits must be 7 or 8\n";
                    return false;
                }
            } else {
                std::cerr << "Error: Data bits argument missing\n";
                return false;
            }
        } else if (arg == "--stop-bits") {
            if (i + 1 < argc) {
                config.stopBits = std::stoi(argv[++i]);
                if (config.stopBits != 1 && config.stopBits != 2) {
                    std::cerr << "Error: Stop bits must be 1 or 2\n";
                    return false;
                }
            } else {
                std::cerr << "Error: Stop bits argument missing\n";
                return false;
            }
        }
        // Common options 
        else if (arg == "-s" || arg == "--slave") {
            if (i + 1 < argc) {
                config.slave = std::stoi(argv[++i]);
            } else {
                std::cerr << "Error: Slave ID argument missing\n";
                return false;
            }
        } else if (arg == "-r" || arg == "--register") {
            if (i + 1 < argc) {
                config.startRegister = std::stoi(argv[++i]);
            } else {
                std::cerr << "Error: Register address argument missing\n";
                return false;
            }
        } else if (arg == "-c" || arg == "--count") {
            if (i + 1 < argc) {
                config.registerCount = std::stoi(argv[++i]);
            } else {
                std::cerr << "Error: Register count argument missing\n";
                return false;
            }
        } else if (arg == "-t" || arg == "--duration") {
            if (i + 1 < argc) {
                config.testDuration = std::stoi(argv[++i]);
            } else {
                std::cerr << "Error: Test duration argument missing\n";
                return false;
            }
        } else if (arg == "-m" || arg == "--mode") {
            if (i + 1 < argc) {
                config.testMode = std::stoi(argv[++i]);
                if (config.testMode < 0 || config.testMode > 2) {
                    std::cerr << "Error: Invalid test mode, must be 0, 1, or 2\n";
                    return false;
                }
            } else {
                std::cerr << "Error: Test mode argument missing\n";
                return false;
            }
        } else if (arg == "-v" || arg == "--interval") {
            if (i + 1 < argc) {
                config.interval = std::stoi(argv[++i]);
                if (config.interval < 0) {
                    std::cerr << "Error: Interval must be non-negative\n";
                    return false;
                }
            } else {
                std::cerr << "Error: Interval argument missing\n";
                return false;
            }
        } else {
            std::cerr << "Error: Unknown option: " << arg << "\n";
            showHelp(argv[0]);
            return false;
        }
    }
    return true;
}

// Print test configuration
void printConfig(const TestConfig& config) {
    std::cout << "Test Configuration:\n";
    std::cout << "  Connection Mode: " << (config.isRTU ? "ModbusRTU" : "ModbusTCP") << "\n";
    
    if (!config.isRTU) {
        std::cout << "  IP Address: " << config.ip << "\n";
        std::cout << "  Port: " << config.port << "\n";
    } else {
        std::cout << "  Device: " << config.device << "\n";
        std::cout << "  Baud Rate: " << config.baudRate << "\n";
        std::cout << "  Parity: " << config.parity << "\n";
        std::cout << "  Data Bits: " << config.dataBits << "\n";
        std::cout << "  Stop Bits: " << config.stopBits << "\n";
    }
    
    std::cout << "  Slave ID: " << config.slave << "\n";
    std::cout << "  Starting Register: " << config.startRegister << "\n";
    std::cout << "  Register Count: " << config.registerCount << "\n";
    std::cout << "  Test Duration: " << config.testDuration << " seconds\n";
    std::cout << "  Test Mode: " << (config.testMode == 0 ? "Read" : 
                                   (config.testMode == 1 ? "Write" : "Read+Write")) << "\n";
    std::cout << "  Operation Interval: " << (config.interval == 0 ? "Max Speed" : 
                                             std::to_string(config.interval) + " ms") << "\n";
    std::cout << "----------------------------------------\n";
}

int main(int argc, char* argv[]) {
    TestConfig config;
    
    // Parse command line arguments
    if (!parseArgs(argc, argv, config)) {
        return 1;
    }
    
    // Print test configuration
    printConfig(config);
    
    // Initialize Modbus control
    ModbusControl* modbus = nullptr;
    
    if (config.isRTU) {
        // RTU mode
        modbus = new ModbusControl(
            config.device.c_str(), 
            config.baudRate, 
            config.parity, 
            config.dataBits, 
            config.stopBits, 
            config.slave
        );
        std::cout << "Connecting to ModbusRTU device at " << config.device << "...\n";
    } else {
        // TCP mode
        modbus = new ModbusControl(
            config.ip.c_str(), 
            config.port, 
            config.slave
        );
        std::cout << "Connecting to ModbusTCP server at " << config.ip << ":" << config.port << "...\n";
    }
    
    // Try to connect to the Modbus server
    if (!modbus->connect()) {
        std::cerr << "Error: Failed to connect to Modbus device\n";
        delete modbus;
        return 1;
    }
    std::cout << "Connected successfully\n";
    
    // Prepare test buffers
    std::vector<uint16_t> readBuffer(config.registerCount);
    std::vector<uint16_t> writeBuffer(config.registerCount);
    
    // Initialize write buffer with test data
    for (int i = 0; i < config.registerCount; i++) {
        writeBuffer[i] = i + 1;  // Simple test pattern
    }
    
    // Statistics variables
    int readOps = 0;
    int writeOps = 0;
    int failedOps = 0;
    
    // Start test
    std::cout << "Starting throughput test for " << config.testDuration << " seconds...\n";
    auto startTime = std::chrono::steady_clock::now();
    auto endTime = startTime + std::chrono::seconds(config.testDuration);
    
    while (std::chrono::steady_clock::now() < endTime) {
        // Read test
        if (config.testMode == 0 || config.testMode == 2) {
            if (modbus->readHoldingRegisters(config.startRegister, config.registerCount, readBuffer.data())) {
                readOps++;
            } else {
                failedOps++;
            }
        }
        
        // Write test
        if (config.testMode == 1 || config.testMode == 2) {
            if (modbus->writeRegisters(config.startRegister, config.registerCount, writeBuffer.data())) {
                writeOps++;
            } else {
                failedOps++;
            }
        }
        
        // Sleep for the specified interval if not at max speed
        if (config.interval > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(config.interval));
        }
    }
    
    auto actualTestDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - startTime).count() / 1000.0;
    
    // Disconnect from Modbus server
    modbus->disconnect();
    delete modbus;
    
    // Calculate throughput
    double totalOps = readOps + writeOps;
    double totalTransferredRegisters = (readOps + writeOps) * config.registerCount;
    double opsPerSecond = totalOps / actualTestDuration;
    double registersPerSecond = totalTransferredRegisters / actualTestDuration;
    double dataThroughputBytes = registersPerSecond * 2; // Each register is 2 bytes
    double dataThroughputKBps = dataThroughputBytes / 1024.0;
    
    // Print results
    std::cout << "\nTest Results:\n";
    std::cout << "  Test Duration: " << std::fixed << std::setprecision(2) << actualTestDuration << " seconds\n";
    std::cout << "  Total Operations: " << totalOps << " (" << readOps << " reads, " << writeOps << " writes)\n";
    std::cout << "  Failed Operations: " << failedOps << "\n";
    std::cout << "  Operations per Second: " << std::fixed << std::setprecision(2) << opsPerSecond << " ops/sec\n";
    std::cout << "  Registers per Second: " << std::fixed << std::setprecision(2) << registersPerSecond << " registers/sec\n";
    std::cout << "  Data Throughput: " << std::fixed << std::setprecision(2) << dataThroughputKBps << " KB/sec\n";
    
    return 0;
}

