const iconRev = document.getElementById("iconrev")
const iconFwd = document.getElementById("iconfwd")
const lblCapacity = document.getElementById('capacity')
const lblVoltage = document.getElementById('voltage')
const lblSpeed = document.getElementById('speed')
const lblSsid = document.getElementById('ssid')
const lblName = document.getElementById('name')
const lblVersion = document.getElementById('version')
const btnChangeDirection = document.getElementById('buttonChangeDirection')

// const ws = new WebSocket(`ws://${document.location.host}/ws`,['arduino']);
const ws = new WebSocket('ws://192.168.178.167/ws',['arduino']);

ws.onopen = (e) =>{
  console.log("Websocket Connected");
};
ws.onclose = (e) => {
  console.log("Websocket Disconnected");
};
ws.onerror = (e) => {
  console.log("websocket Error", e);
};

function updateStatus(status) {
  lblSsid.textContent = status.ssid
  lblName.textContent = status.name
  lblVersion.textContent = status.version
  lblVoltage.textContent = status.batVoltage
  lblCapacity.textContent = status.batRate
  lblSpeed.textContent = status.speed

  if (status.speed === 0) {
    btnChangeDirection.classList.remove("pure-button-disabled")
  } else {
    btnChangeDirection.classList.add("pure-button-disabled")
  }

  if (status.direction === 0) {
    // vorwärts
    iconFwd.style.display = ""
    iconRev.style.display = "none"
  } else {
      // rückwärts
      iconFwd.style.display = "none"
      iconRev.style.display = ""
  }
}

ws.onmessage = (e) => {
  try {
      const status = JSON.parse(e.data)
      console.log('message', status)
      updateStatus(status)
  } catch(e) {
      console.log('no json')
      console.log(e)
  }
};

/**
 * Button-Action methods
 */

// Richtungsänderung
function onChangeDirection() {
    console.log('Button-Click: CHANGEDIRECTION')
    ws.send('#CHANGEDIRECTION')
}

// Button Langsamer
function onSlower() {
    console.log('Button-Click: SLOWER')
    ws.send('#SLOWER')
}

// Button Schneller
function onFaster() {
    console.log('Button-Click: FASTER')
    ws.send('#FASTER')
}

// Button Stop
function onStop() {
    console.log('Button-Click: STOP')
    ws.send('#STOP')
}
