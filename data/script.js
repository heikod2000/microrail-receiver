const iconRev = document.getElementById("iconrev")
const iconFwd = document.getElementById("iconfwd")
const lblCapacity = document.getElementById('capacity')
const lblVoltage = document.getElementById('voltage')
const lblSpeed = document.getElementById('speed')
const btnChangeDirection = document.getElementById('buttonChangeDirection')

let ws
window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
}

function initWebSocket() {
  console.log('Trying to open a WebSocket connection...')
  ws = new WebSocket(`ws://${document.location.host}/ws`,['arduino'])
  // ws = new WebSocket('ws://192.168.178.167/ws',['arduino']);
  ws.onopen  = onOpen
  ws.onclose = onClose
  ws.onerror = onError
  ws.onmessage = onMessage
}

function onOpen(event) {
  console.log("WebSocket Connected");
}

function onClose(event) {
  console.log('Connection closed');
  setTimeout(initWebSocket, 2000);
}

function onError(event) {
  console.log("websocket Error", event);
}

function updateDirectionSpeed(direction, speed) {
  lblSpeed.textContent = speed

  if (speed === 0) {
    btnChangeDirection.classList.remove("pure-button-disabled")
  } else {
    btnChangeDirection.classList.add("pure-button-disabled")
  }

  if (direction === 0) {
    // vorwärts
    iconFwd.style.display = ""
    iconRev.style.display = "none"
  } else {
      // rückwärts
      iconFwd.style.display = "none"
      iconRev.style.display = ""
  }
}

function updateBattery(voltage, capacity) {
  lblVoltage.textContent = voltage
  lblCapacity.textContent = capacity
}

function onMessage(event) {
  const data = event.data.split(":");
  const cmd = data[0];
  if (cmd === 'A') {
    updateDirectionSpeed(Number.parseInt(data[1]), Number.parseInt(data[2]))
  } else if (cmd === 'B') {
    updateBattery(data[1], data[2])
  }
};

/**
 * Button-Action methods
 */

// Richtungsänderung
function onChangeDirection() {
    ws.send('#CHANGEDIRECTION')
}

// Button Langsamer
function onSlower() {
    ws.send('#SLOWER')
}

// Button Schneller
function onFaster() {
    ws.send('#FASTER')
}

// Button Stop
function onStop() {
    ws.send('#STOP')
}
