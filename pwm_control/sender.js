const { SerialPort } = require('serialport');

const port = new SerialPort({
    path: '', // Path to the port
    baudRate: 115000 * 2,
});

port.on('data', (data) => {
    console.log('Raw data:', data.toString());
});

port.on('open', () => {
    console.log('Port opened');

    let angle = 0;
    const MOTOR_ID = 0;
    setInterval(() => {
        angle += 1;
        if (angle > 180) {
            angle = 0;
        }
        const command = `${MOTOR_ID};${angle}`;
        port.write(command)
    })
});