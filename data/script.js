const iconRev = document.getElementById("iconrev")
const iconFwd = document.getElementById("iconfwd")
const lblCapacity = document.getElementById('capacity')
const lblVoltage = document.getElementById('voltage')
const lblSpeed = document.getElementById('speed')
const lblSsid = document.getElementById('ssid')
const lblName = document.getElementById('name')
const lblVersion = document.getElementById('version')
const btnChangeDirection = document.getElementById('buttonChangeDirection')

//const urlPrefix = 'http://192.168.178.167/' // Debug
const urlPrefix = ''

const evtSource = new EventSource('/events');
//const evtSource = new EventSource('http://192.168.178.167/events'); // DEBUG

evtSource.addEventListener('open', (e) => {
  console.log("Event: Connected");
})

evtSource.addEventListener('error', (e) =>  {
  if (e.target.readyState !== EventSource.OPEN) {
    console.log("Event: Disconnected", e);
  }
})

evtSource.addEventListener('event.start', (e) => {
  console.log("message connect: ", e.data);
  try {
      const data = JSON.parse(e.data)
      lblSsid.textContent =data.ssid
      lblName.textContent = data.name
      lblVersion.textContent = data.version
  } catch(e) {
      console.log('no json', e)
  }
})

evtSource.addEventListener('event.direction', (e) => {
  const direction = Number(e.data)
  console.log("update direction", e.data)

  if (direction === 0) {
      // vorwärts
      iconFwd.style.display = ""
      iconRev.style.display = "none"
  } else {
      // rückwärts
      iconFwd.style.display = "none"
      iconRev.style.display = ""
  }
})

evtSource.addEventListener('event.speed', (e) => {
  const speed = Number(e.data)
  console.log("update speed", speed)
  lblSpeed.textContent = speed
  if (speed === 0) {
    btnChangeDirection.classList.remove("pure-button-disabled")
  } else {
    btnChangeDirection.classList.add("pure-button-disabled")
  }
})

evtSource.addEventListener('event.batvoltage', (e) => {
  lblVoltage.textContent = e.data
})

evtSource.addEventListener('event.batrate', (e) => {
  lblCapacity.textContent = e.data
})

/**
 * Button-Action methods
 */

// Richtungsänderung
function onChangeDirection() {
    console.log('Button-Click: CHANGEDIRECTION')
    fetch(`${urlPrefix}cmd?command=CHANGEDIRECTION`)
      .then(data => {})
      .catch(error => {})
}

// Button Langsamer
function onSlower() {
    console.log('Button-Click: SLOWER')
    fetch(`${urlPrefix}cmd?command=SLOWER`)
      .then(data => {})
      .catch(error => {})
}

// Button Schneller
function onFaster() {
    console.log('Button-Click: FASTER')
    fetch(`${urlPrefix}cmd?command=FASTER`)
      .then(data => {})
      .catch(error => {})
}

// Button Stop
function onStop() {
    console.log('Button-Click: STOP')
    fetch(`${urlPrefix}cmd?command=STOP`)
      .then(data => {})
      .catch(error => {})
}
