const SerialPort = require("serialport");

const port = new SerialPort(
  "/dev/cu.usbmodem141101",
  { baudRate: 9600 },
  error => console.log(error)
);
const bytes = [
  0x59,
  0x02,
  0x00,
  0x00,
  0xff,
  0x00,
  0x00,
  0xff,
  0x00,
  0xff,
  0xff,
  0xff,
  0x00,
  0x32,
  0x01
];
const checksum = bytes.reduce((prev, curr) => prev + curr) & 0xff;
bytes.push(checksum);
port.write(Buffer.of(...bytes, checksum));
