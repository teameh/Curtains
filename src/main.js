var SerialPort = require('serialport');
var delay = require('delay');

// Use a `\r\n` as a line terminator
const parser = new SerialPort.parsers.Readline({
  delimiter: '\r\n'
});

var port = new SerialPort('/dev/cu.SLAB_USBtoUART', {
    baudRate: 9600
});

port.pipe(parser);

// Open errors will be emitted as an error event
port.on('error', function(err) {
  console.log('Error: ', err.message);
})

port.on('open', () => console.log('Port open'));

parser.on('data', console.log);

const main = async () => {

    port.write('ROTATE_X_STEPS 9990\r\n', err => {
        if (err) {
          return console.log('Error on write: ', err.message);
        }
    });

    // port.write('SET_SETTINGS 9999 9999\r\n', err => {
    //     if (err) {
    //       return console.log('Error on write: ', err.message);
    //     }
    // });

    // await delay(1000);

    // port.write('ROTATE_X_STEPS 250\r\n', err => {
    //     if (err) {
    //       return console.log('Error on write: ', err.message);
    //     }
    // });

    // port.write('SET_SETTINGS 9999 9999\r\n', err => {
    //     if (err) {
    //       return console.log('Error on write: ', err.message);
    //     }
    // });


    // await delay(1000);

    // port.write('ROTATE_X_STEPS 10\r\n', err => {
    //     if (err) {
    //       return console.log('Error on write: ', err.message);
    //     }
    // });

    // port.write('ROTATE_X_STEPS -50\r\n', err => {
    //     if (err) {
    //       return console.log('Error on write: ', err.message);
    //     }
    // });

    // await delay(2000);

    // port.write('ROTATE_X_STEPS 75\r\n', err => {
    //     if (err) {
    //       return console.log('Error on write: ', err.message);
    //     }
    // });
    // await delay(2000);

    // port.write('ROTATE_X_STEPS -50\r\n', err => {
    //     if (err) {
    //       return console.log('Error on write: ', err.message);
    //     }
    // });
    // await delay(2000);

    // await delay(2000);

    // port.write('STOP_ROTATION\r\n', err => {
    //     if (err) {
    //       return console.log('Error on write: ', err.message);
    //     }
    // });

    // await delay(500);
};

main();
