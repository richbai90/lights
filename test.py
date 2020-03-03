import serial
ser = serial.Serial("/dev/cu.usbmodem141101")
bytes = [0x59, 0x02,0x00,0x00,0xFF,0x00,0x00,0xFF,0x00,0xFF,0xFF,0xFF,0x00,0x32,0x01]
sum = 0
for byte in bytes:
    sum += byte

bytes.append(sum & 0xFF)

ser.write(bytearray(bytes))