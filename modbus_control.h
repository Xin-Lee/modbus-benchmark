#include "modbus.h"
#include <stdint.h>
#include <functional>

class ModbusControl {
public:
    // Function pointer types for connection methods
    using ConnectFunc = std::function<bool(modbus_t*)>;
    using DisconnectFunc = std::function<void(modbus_t*)>;

    // Constructors and destructor
    ModbusControl(const char* device, int baud, char parity, int data_bit, int stop_bit, int slave);
    ModbusControl(const char* ip, int port, int slave);
    ~ModbusControl();

    // Connection management
    bool connect();
    void disconnect();
    bool isConnected() const;

    // Modbus operations
    bool readCoils(int addr, int nb, uint8_t* dest);
    bool writeCoil(int addr, uint8_t value);
    bool readHoldingRegisters(int addr, int nb, uint16_t* dest);
    bool readInputRegisters(int addr, int nb, uint16_t* dest);
    bool writeRegister(int addr, uint16_t value);
    bool writeRegisters(int addr, int nb, const uint16_t* values);

private:
    // Helper methods
    void setupRTUFunctions();
    void setupTCPFunctions();
    bool ensureConnection();
    void handleError(const char* context);

    // Member variables
    modbus_t* ctx = nullptr;
    bool connected = false;
    ConnectFunc connectFunc;
    DisconnectFunc disconnectFunc;
};