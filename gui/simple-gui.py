import serial

def checksum(val):
    return ((val[0] & 0xF) ^ ((val[0] >> 4) & 0xF) ^ (val[1] & 0xF) ^ 0xA) == ((val[1] >> 4) & 0xF)

def valToHex(val):
    return hex(val[0]) + " " + hex(val[1])

def valToDec(val):
    return ((val[1] & 0xF) << 8) | val[0]

def printHexLine(name, val):
    print(name + ": " + str(valToDec(val)) + " | " + str(valToHex(val)) + " [" + str(checksum(val)) + "]")


with serial.Serial('COM8', 19200, timeout=60) as ser:
    ser.setDTR(True)
    while True: 
        line = ser.readline()
        if "Live Data" in line.decode("utf-8"):
            data = ser.readline()
            printHexLine("RPM", data[0:2])
            printHexLine("Lambda", data[2:4])
            printHexLine("Supply Voltage", data[4:6])
            printHexLine("Throttle Pos", data[6:8])
            printHexLine("Short Fuel Correction", data[8:10])
            printHexLine("Throttle mode", data[10:12])
            printHexLine("Coolant (V)", data[12:14])
            printHexLine("Coolant (C)", data[14:16])
            printHexLine("Air Temp (V)", data[16:18])
            printHexLine("Air Temp (C)", data[18:20])
            printHexLine("Idle Valve", data[20:22])
            printHexLine("Airflow Meter", data[22:24])
            printHexLine("EGR Diff Pressure", data[24:26])
            printHexLine("Injection Pulse", data[26:28])
            printHexLine("Ignition Timing", data[28:30])
            printHexLine("Sensors Power Voltage", data[30:32])
            print("")