import serial

# Replace 'COM5' with your actual port (or '/dev/ttyACM0' inside WSL)
with serial.Serial('COM9', 115200, timeout=1) as ser:
    while True:
        value = input("LED PC13 (1=ON, 0=OFF): ")

        if value in ("0", "1"):
            ser.write(bytes([int(value)]))
            print(ser.read(1))
