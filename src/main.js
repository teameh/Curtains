var SerialPort = require("serialport");
var delay = require("delay");
const meow = require("meow");
const prompt = require("prompt-async");
var colors = require("colors/safe");

// Use a `\r\n` as a line terminator
const parser = new SerialPort.parsers.Readline({
  delimiter: "\r\n"
});

var port = new SerialPort("/dev/cu.SLAB_USBtoUART", {
  baudRate: 115200
});

port.pipe(parser);

// Open errors will be emitted as an error event
port.on("error", function(err) {
  console.log("Error: ", err.message);
});

port.on("open", () => console.log("Serial port open"));

parser.on("data", console.log);

prompt.message = colors.rainbow("");
// prompt.delimiter = colors.green("><");

const cli = meow({
  flags: {
    interactive: { type: "boolean" },
  }
});

const execCommand = cmd => {
  port.write(`${cmd}\r\n`, err => {
    if (err) {
      return console.log("Error on write: ", err.message);
    }
  });
};

const getInt = async (name, description, defautlValue) => {
  const res = await prompt.get({
    properties: {
      [name]: {
        description: description,
        type: "integer",
        pattern: /^[0-9]+$/,
        default: defautlValue,
        message: "please input an interger",
        required: true
      }
    }
  });

  return res[name];
};

const recursivePrompt = async () => {
  // Start the prompt.
  prompt.start();

  // Get two properties from the user: the `username` and `email`.
  const { action } = await prompt.get({
    properties: {
      action: {
        description: colors.green(
          "Actions: \n 0) Rotate \n 1) On \n 2) Off \n 3) Set speed \n 4) Set accel \n 5) stop"
        ),
        type: "integer",
        pattern: /^[0-9]+$/,
        message: "Please choose a valid action",
        required: true
      }
    }
  });

  switch (action) {
    case 0:
      const steps = await getInt("steps", "steps:", 1000);
      execCommand(`STEPS ${steps}`);
      break;
    case 1:
      execCommand("MOTOR_ON");
      break;
    case 2:
      execCommand("MOTOR_OFF");
      break;
    case 3:
      const speed = await getInt("speed", "speed:", 1000);
      execCommand(`SPEED ${speed}`);
      break;
    case 4:
      const accel = await getInt("accel", "accel:", 1000);
      execCommand(`ACCEL ${accel}`);
      break;
      case 5:
      execCommand("STOP");
      break;
    default:
      console.log("Invalid action, please enter another");
  }

  await recursivePrompt();
};

const main = async () => {
  await delay(100);

  if(cli.flags.interactive) {
    await recursivePrompt();
  }
};

main();
