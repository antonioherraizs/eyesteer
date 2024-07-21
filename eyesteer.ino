/*
Piloto Digital Punta Negra 1.0
*/

/* includes */
#include <SPI.h>
#include <MCP41HVX1.h>
#include <WiFiNINA.h>

/* Uncomment to use the hardware RX/TX serial port, Comment to use USB serial */
//#define Serial Serial1

/* constants */
const int CS_PIN =                     10; // SPI CS pin, D10 on the arduino nano
const int LED_GREEN =                   2; // GPIO pin, D2 on the arduino nano
const int MAX_WIPER_VALUE =           255;
const int MID_WIPER_VALUE =           128;
const int MIN_WIPER_VALUE =             0;
const int WIPER_SPEED =                10; // how quickly the wiper changes to the next position, in ms
//const int DIAL_WIPER_MAP_SIZE =         7; // this has to match NUM_DIALS in the HTML code
//const int dialWiperMap[DIAL_WIPER_MAP_SIZE] = {230, 200, 170, 128, 80, 40, 0};
const int DIAL_WIPER_MAP_SIZE =         5; // this has to match NUM_DIALS in the HTML code
const int dialWiperMap[DIAL_WIPER_MAP_SIZE] = {230, 190, 128, 60, 0};
const char *ssid = "test";
const char *password = "test";

/* globals */
int status = WL_IDLE_STATUS;
enum state {
  idleState,
  towardsBaborState,
  towardsEstriborState,
};
state currentState = idleState;
bool baborRequested = false;
bool estriborRequested = false;
int wiperTarget = 128; // safe default
unsigned long timeSinceLastWiperChange = 0;
WiFiServer server(80);
MCP41HVX1 Digipot(CS_PIN); // Instantiate MCP41HVX1 class with hardwired values for WLAT (low) and SHDN (high)

// =========================================
// =========================================
// =========================================
/*
Web page
*/
const String INDEX_HTML_TOP = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8" name="viewport" content="width=device-width, initial-scale=1">
<style>
html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
body{margin-top: 50px;}
p {font-size: 14px;color: #888;margin-bottom: 10px;}
progress[value] {width: 50%; height: 30px;}
.block {
  width: 20%;
  background-color: #4CAF50;
  color: white;
  padding: 5%;
  font-size: 1.5em;
  font-weight: bold;
  cursor: pointer;
  text-align: center;
}
.block_bg_red {
  background-color:red;
}
.block_disabled {
  background-color: #ddd;
  color: white;
}
</style>
</head>
<body onload="start_timer()">
<div align="center">
<h3>Piloto Digital Punta Negra 1.0</h3>
<p><canvas id="canvas" width="250" height="250" style="background-color:#white"></canvas></p>
<p><progress value="0" max="5" id="progress_bar"></progress></p>
<p><button id="babor" class="block block_disabled" onclick="decreasePosition();navigate('babor');" type="button" disabled>Babor</button>
<button id="estribor" class="block block_disabled" onclick="increasePosition();navigate('estribor');" type="button" disabled>Estribor</button></p>
</div>
<script>
function start_timer()
{
  var WAIT_TIME = 5;
  var timeleft = WAIT_TIME;
  var downloadTimer = setInterval(function()
  {
    if(timeleft <= 0)
    {
      document.getElementById("babor").disabled = false;
      document.getElementById("estribor").disabled = false;
      document.getElementById("babor").className = "block block_bg_red";
      document.getElementById("estribor").className = "block";
      clearInterval(downloadTimer);
    }
    document.getElementById("progress_bar").value = WAIT_TIME - timeleft;
    timeleft -= 1;
  }, 1000);
}
function navigate(side) {
  // disable buttons
  document.getElementById("babor").disabled = true;
  document.getElementById("estribor").disabled = true;
  document.getElementById("babor").className = "block block_disabled";
  document.getElementById("estribor").className = "block block_disabled";
  // send request
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function () {
    if (xhr.readyState === 4) {
      console.log("requested /" + side);
    }
  }
  xhr.open('GET', '/' + side, true);
  xhr.send();
  // start timer to wait
  start_timer();
}
var canvas = document.getElementById("canvas");
var ctx = canvas.getContext("2d");
var radius = canvas.height / 2;
const NUM_DIALS = 5;
const MEDIAN = parseInt(NUM_DIALS / 2) + 1;
var position = )=====";
// =========================================
// =========================================
// =========================================
const String INDEX_HTML_BOTTOM = R"=====(;
ctx.translate(radius, radius);
radius = radius * 0.75;
drawDial()

function drawDial() {
  drawFace(ctx, radius);
  drawNumbers(ctx, radius);
  drawPosition(ctx, radius, position);
}
function drawFace(ctx, radius) {
  var grad;
  ctx.beginPath();
  ctx.arc(0, 0, radius, 0, 2*Math.PI);
  ctx.fillStyle = '#256058';
  ctx.fill();
  ctx.lineWidth = radius*0.1;
  ctx.stroke();
  ctx.fillStyle = '#000000';
}
function drawNumbers(ctx, radius) {
  var ang;
  var num;
  ctx.font = radius*0.25 + "px arial";
  ctx.textBaseline="middle";
  ctx.textAlign="center";
  for(num = 1; num <= NUM_DIALS; num++){
    ang = num * Math.PI / MEDIAN;
    ctx.rotate(ang);
    ctx.translate(0, radius*1.2);
    ctx.rotate(-ang);
    ctx.fillText(num.toString(), 0, 0);
    ctx.rotate(ang);
    ctx.translate(0, -radius*1.2);
    ctx.rotate(-ang);
  }
}
function drawPosition(ctx, radius, number) {
  // 1 == 5, 2 == 6, ...
  var position = ((number+MEDIAN)*Math.PI/MEDIAN);
  drawHand(ctx, position, radius, radius*0.1);
}
function drawHand(ctx, pos, length, width) {
  ctx.moveTo(0,0);
  ctx.rotate(pos);
  ctx.translate(0, -radius);
  ctx.fillRect(-width*0.5, 0, width, length*0.7);
  ctx.translate(0, radius);
  ctx.rotate(-pos);
}
function increasePosition() {
  if (position < NUM_DIALS)
  {
    position += 1;
    drawFace(ctx, radius);
    drawPosition(ctx, radius, position);
  }
}
function decreasePosition() {
  if (position > 1)
  {
    position -= 1;
    drawFace(ctx, radius);
    drawPosition(ctx, radius, position);
  }
}
</script>
</body>
</html>
)=====";
// =========================================
// =========================================
// =========================================

void setup() {
  // prepare LED, it will be off while we boot
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, LOW);
  
  // initialize Serial-to-USB
  Serial.begin(115200);
//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for native USB port only
//  }

  // say hi
  Serial.println("Piloto Digital Punta Negra 1.0");

  /* 
   *  I have to call SPI.begin() here even though
   *  it's also called inside "MCP41HVX1.h". If I 
   *  don't call it the SPI bus won't be initialized
   *  on time and the arduino will crash with the
   *  first call to a Digipot.* function.
   */
  SPI.begin();
  
  // reset digipot wiper
  Serial.print("Starting Position = ");
  Serial.println(Digipot.WiperGetPosition());
  Serial.print("Set Wiper Position = ");
  Serial.println(Digipot.WiperSetPosition(MID_WIPER_VALUE));

  // inform of size of HTML, it cannot be more than 4096
  int sizeOfHTML = INDEX_HTML_TOP.length() + 1 + INDEX_HTML_BOTTOM.length();
  if (sizeOfHTML > 4096) {
    Serial.println("Size of HTML exceeds 4096, aborting");
    while (true);
  }
  Serial.println(String("Size of HTML ") + sizeOfHTML);
  
  // check for the WiFi module
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }
  String fv = WiFi.firmwareVersion();
  Serial.print("WifiNINA firmware version ");
  Serial.println(fv);
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // print the network name (SSID)
  // local IP address will be 192.168.4.1
  Serial.print("Creating access point named: ");
  Serial.println(ssid);

  // create WPA network
  status = WiFi.beginAP(ssid, password);
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    while (true);
  }

  // wait for connection
  delay(1000);

  // start the web server on port 80
  server.begin();

  // bootup complete, light up LED and print WiFi status
  digitalWrite(LED_GREEN, HIGH);
  printWiFiStatus();
}


void loop() {

  // get the current wiper postion from the digipot
  int nWiper = Digipot.WiperGetPosition();
  
  // detect if a client has connected to the AP
  if (status != WiFi.status())
  {
    // status has changed
    status = WiFi.status();
    // a device has connected to the AP
    if (status == WL_AP_CONNECTED)
    {
      Serial.println("Client connected to AP");
    }
    else
    {
      Serial.println("Client disconnected from AP");
    }
  }

  WiFiClient client = server.available();   // listen for incoming clients
  if (client) {                             // if you get a client,
    //Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        //Serial.write(c);                    // enable to debug HTTP request to serial
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // inject current dial position in HTTP response based on current wiper value
            String response = INDEX_HTML_TOP;
            response += getDialFromWiper(nWiper);
            response += INDEX_HTML_BOTTOM;
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.print("Content-Length: ");
            client.println(response.length());
            client.println("Connection: close");
            client.println();
            // send body of response
            client.println(response);
            // HTTP response ends with another blank line
            client.println();
            // break out of the while loop:
            break;
          }
          else
          {
            // we got a new line so check if it's an HTTP GET and apply logic
            if (currentLine.startsWith("GET /babor"))
            {
              baborRequested = true;
            }
            else if (currentLine.startsWith("GET /estribor"))
            {
              estriborRequested = true;
            }
            else
            {
              // could be a different HTTP verb or another HTTP header, do nothing
            }
            if (currentLine.startsWith("GET /"))
            {
              // log any HTPT GET request
              Serial.println(String("[+] ") + currentLine);
            }
            // clear line buffer for next line in HTTP request
            currentLine = "";
          }
        }
        else if (c != '\r')
        { // if you got anything else but a carriage return character,
          currentLine += c;   // add it to the end of the currentLine
        }
      }
    }
    // close the connection:
    client.stop();
    //Serial.println("client disconnected");
  }

  // update status continuously to adjust wiper if necessary
  updateStatus(nWiper);
}

void updateStatus(int nWiper)
{
  // update timer with current time
  unsigned long currentMillis = millis();
  
  // determine direction based on user input and current state
  switch(currentState)
  {
    case idleState:
    {
      // get current dial position
      int dial = getDialFromWiper(nWiper);
      // are we going babor, estribor or staying idle?
      if (baborRequested == true)
      {
        if (dial > 1)
        {
          // dial is within range to change so start state transition
          int dialTarget = dial - 1;
          // wiperTarget will help us track the desired value globally
          wiperTarget = getWiperFromDial(dialTarget);
          // update state
          Serial.println(String("[+] Turning babor from ") + dial + String(" to ") + dialTarget);
          currentState = towardsBaborState;
          displayCurrentState(currentState);
          // reset timer
          timeSinceLastWiperChange = millis();
        }
        else
        {
          Serial.println("[+] Babor dial limit reached, doing nothing");
        }
        // reest request in any case, even if it was not executed
        baborRequested = false;
      }
      else if (estriborRequested == true)
      {
        if (dial < DIAL_WIPER_MAP_SIZE)
        {
          // dial is within range to change so start state transition
          int dialTarget = dial + 1;
          // wiperTarget will help us track the desired value globally
          wiperTarget = getWiperFromDial(dialTarget);
          // update state
          Serial.println(String("[+] Turning estribor from ") + dial + String(" to ") + dialTarget);
          currentState = towardsEstriborState;
          displayCurrentState(currentState);
          // reset timer
          timeSinceLastWiperChange = millis();
        }
        else
        {
          Serial.println("[+] Estribor dial limit reached, doing nothing");
        }
        // reest request in any case, even if it was not executed
        estriborRequested = false;
      }
      else
      {
        // stay idle
      }
      break;
    }
    case towardsBaborState:
    {
      if (nWiper < wiperTarget)
      {
        if (currentMillis - timeSinceLastWiperChange > WIPER_SPEED)
        {
          nWiper = Digipot.WiperIncrement();
          timeSinceLastWiperChange = millis();
        }
      }
      else
      {
        // arrived to babor target
        Serial.print("[+] Wiper position = ");
        Serial.print(nWiper);
        Serial.print(", dial position = ");
        Serial.println(getDialFromWiper(nWiper));
        currentState = idleState;
        displayCurrentState(currentState);
      }
      break;
    }
    case towardsEstriborState:
    {
      if (nWiper > wiperTarget)
      {
        if (currentMillis - timeSinceLastWiperChange > WIPER_SPEED)
        {
          nWiper = Digipot.WiperDecrement();
          timeSinceLastWiperChange = millis();
        }
      }
      else
      {
        // arrived to estribor target
        Serial.print("[+] Wiper position = ");
        Serial.print(nWiper);
        Serial.print(", dial position = ");
        Serial.println(getDialFromWiper(nWiper));
        currentState = idleState;
        displayCurrentState(currentState);
      }
      break; 
    } 
  }  
}

void displayCurrentState(int nState)
{
  switch(currentState)
  {
    case idleState:
      Serial.println("[+] current State: idleState");
      break;
    case towardsBaborState:
      Serial.println("[+] current State: towardsBaborState");
      break;
    case towardsEstriborState:
      Serial.println("[+] current State: towardsEstriborState");
      break;
  }
}

int getWiperFromDial(int nDial)
{
  return dialWiperMap[nDial - 1];
}
int getDialFromWiper(int nWiper)
{
  int result = (DIAL_WIPER_MAP_SIZE / 2) + 1; // median as default
  for (byte i = 0; i < DIAL_WIPER_MAP_SIZE; i++)
  {
    if (nWiper == dialWiperMap[i])
    {
      result = i + 1;
    }
  }
  return result;
}

void printWiFiStatus()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("Visit http://");
  Serial.println(ip);
}
