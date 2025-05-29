#include <iostream>
#include <string>
#include <memory>
#include <functional>
#include "modbus_control.h"

ModbusControl::ModbusControl(const char* device, int baud, char parity, int data_bit, int stop_bit, int slave) {
    ctx = modbus_new_rtu(device, baud, parity, data_bit, stop_bit);
    modbus_set_slave(ctx, slave);
    
    setupRTUFunctions();
}

    // Constructor for TCP
ModbusControl::ModbusControl(const char* ip, int port, int slave) {
    ctx = modbus_new_tcp(ip, port);
    modbus_set_slave(ctx, slave);
    setupTCPFunctions();
}

ModbusControl::~ModbusControl() {
    disconnect();
    if (ctx) {
        modbus_free(ctx);
    }
}

bool ModbusControl::connect() {
    if (!ctx) return false;
    return connectFunc(ctx);
}

void ModbusControl::disconnect() {
    if (ctx) {
        disconnectFunc(ctx);
    }
}

bool ModbusControl::isConnected() const {
    return connected;
}

// Read holding registers
bool ModbusControl::readHoldingRegisters(int addr, int nb, uint16_t* dest) {
    if (!ensureConnection()) return false;
    
    if (modbus_read_registers(ctx, addr, nb, dest) == -1) {
        handleError("Read holding registers failed");
        return false;
    }
    return true;
}

bool ModbusControl::readCoils(int addr, int nb, uint8_t* dest) {
    if (!ensureConnection()) return false;
    if (modbus_read_bits(ctx, addr, nb, dest) == -1) {
        handleError("Read coils failed");
        return false;
    }
    return true;
}

bool ModbusControl::writeCoil(int addr, uint8_t value) {
    if (!ensureConnection()) return false;
    if (modbus_write_bit(ctx, addr, value) == -1) {
        handleError("Write coil failed");
        return false;
    }
    return true;
}

// Read input registers
bool ModbusControl::readInputRegisters(int addr, int nb, uint16_t* dest) {
    if (!ensureConnection()) return false;
    if (modbus_read_input_registers(ctx, addr, nb, dest) == -1) {
        handleError("Read input registers failed");
        return false;
    }
    return true;
}

// Write single register
bool ModbusControl::writeRegister(int addr, uint16_t value) {
    if (!ensureConnection()) return false;

    if (modbus_write_register(ctx, addr, value) == -1) {
        handleError("Write register failed");
        return false;
    }
    return true;
}

// Write multiple registers
bool ModbusControl::writeRegisters(int addr, int nb, const uint16_t* values) {
    if (!ensureConnection()) return false;

    if (modbus_write_registers(ctx, addr, nb, values) == -1) {
        handleError("Write registers failed");
        return false;
    }
    return true;
}

void ModbusControl::setupRTUFunctions() {
    // RTU specific connect/disconnect functions
    connectFunc = [this](modbus_t* ctx) {
        if (modbus_connect(ctx) == -1) {
            handleError("RTU connection failed");
            return false;
        }
        connected = true;
        return true;
    };

    disconnectFunc = [this](modbus_t* ctx) {
        modbus_close(ctx);
        connected = false;
    };
}

void ModbusControl::setupTCPFunctions() {
    // TCP specific connect/disconnect functions
    connectFunc = [this](modbus_t* ctx) {
        if (modbus_connect(ctx) == -1) {
            handleError("TCP connection failed");
            return false;
        }
        connected = true;
        return true;
    };

    disconnectFunc = [this](modbus_t* ctx) {
        modbus_close(ctx);
        connected = false;
    };
}

bool ModbusControl::ensureConnection() {
    if (!ctx) return false;
    if (!connected) {
        return connect();
    }
    return true;
}

void ModbusControl::handleError(const char* context) {
    connected = false;
    // Log error: context + modbus_strerror(errno)
    std::cerr << "Error: " << context << " - " << modbus_strerror(errno) << std::endl;
}

