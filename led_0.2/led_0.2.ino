

/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-websocket-server-arduino/
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

// Import required libraries
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Servo.h>
#include <Adafruit_NeoPixel.h>

#define PIN D5
#define NUMPIXELS 120
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int c[3]={0,0,100}, d[3]={0,0,100};
int i=0,j=0;
int kecerahan=100; //0-100

Servo myservo;

// Replace with your network credentials
const char* ssid = "Rimba Raya_plus";
const char* password = "lampuajaib";

bool ledState = 0;
const int ledPin = 2;
String datawarna;
bool colorshift=false;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>

<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport"
        content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, shrink-to-fit=no">
    <title>RGB Controller</title>
    <link rel="shortcut icon"
        href="data:image/x-icon;base64,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" />
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Nunito&display=swap');

        * {
            padding: 0;
            margin: 0;
            font-family: 'Nunito', sans-serif;
            font-size: 16pt;
        }

        html {
            background: black;
        }

        body {
            background: #222438;
            display: inline;
            overflow: hidden;
        }

        .side {
            width: 30vw;
            min-width: 300px;
            position: relative;
            float: left;
        }

        .title {
            color: #fff;
            background: #2B2D46;
            line-height: 3em;
            height: 3em;
            text-align: center;

        }

        .lampcontainer {
            height: calc(100vh - 3em);
            background: #2b2D46;
            text-align: center;
            padding-top: 2em;
        }

        #lamp {
            background: white;
            width: 250px;
            height: 250px;
            border-radius: 250px;
            margin: 0px auto;
        }

        .main {
            width: 70vw;
            max-width: calc(100vw - 300px);
            position: relative;
            float: right;
        }

        .topbar {
            background: #1a1c2c;
            height: 3em;
            text-align: right;
            padding-right: 1em;

        }

        .nav_control {
            display: inline-block;
            height: .75em;
            width: .75em;
            border-radius: 250px;
            margin-top: 1.1125em;
            margin-left: .5em;
        }

        .close {
            background: #FC474C;
        }

        .maximize {
            background: #2BD735;
        }

        .minimize {
            background: #FCC336;
        }

        .color {
            background: #222438;
            height: 100vh;
            padding: 2em;
            text-align: center;

        }

        .swatchGrid {
            display: grid;
            grid-template-columns: repeat(8, 32px);
            grid-template-rows: repeat(2, 32px);
            justify-content: center;
            grid-gap: 6px;
            margin-bottom: 1em;
            margin-top: 1em;
        }

        .swatch {
            border-radius: 4px;
            cursor: pointer;
        }
    </style>
</head>

<body>
    <div class="side">
        <div class="title">
            Control Panel
        </div>
        <div class="lampcontainer">
            <div id="lamp"></div>
        </div>
    </div>
    <div class="main">
        <div class="topbar">
            <div class="nav_control minimize"></div>
            <div class="nav_control maximize"></div>
            <div class="nav_control close"></div>
        </div>
        <div class="color">
            <div class="ColorPicker" id="sliderPicker"></div>
            <div class="swatchGrid" id="swatch-grid">
                <div class="swatch" data-color="#ffffff" style="background: #ffffff"></div>
                <div class="swatch" data-color="#ff0000" style="background: #ff0000"></div>
                <div class="swatch" data-color="#ffff00" style="background: #ffff00"></div>
                <div class="swatch" data-color="#00ff00" style="background: #00ff00"></div>
                <div class="swatch" data-color="#00ffff" style="background: #00ffff"></div>
                <div class="swatch" data-color="#0000ff" style="background: #0000ff"></div>
                <div class="swatch" data-color="#ff00ff" style="background: #ff00ff"></div>
                <div class="swatch" data-color="#555555" style="background: #555555"></div>

                <div class="swatch" data-color="#da3855" style="background: #da3855"></div>
                <div class="swatch" data-color="#eda541" style="background: #eda541"></div>
                <div class="swatch" data-color="#f5ea5a" style="background: #f5ea5a"></div>
                <div class="swatch" data-color="#75b85b" style="background: #75b85b"></div>
                <div class="swatch" data-color="#66a5d7" style="background: #66a5d7"></div>
                <div class="swatch" data-color="#817799" style="background: #817799"></div>
                <div class="swatch" data-color="#e17ea5" style="background: #e17ea5"></div>
                <div class="swatch" data-color="#f4cdaf" style="background: #f4cdaf"></div>
            </div>
        </div>
    </div>
    <script src="https://cdn.jsdelivr.net/npm/@jaames/iro@5"></script>
    <script type="text/javascript">
        var gateway = `ws://${window.location.hostname}/ws`;
        var websocket;
        var datawarna;
        var oldwarna;
        window.addEventListener('load', onLoad);
        function initWebSocket() {
            console.log('Trying to open a WebSocket connection...');
            websocket = new WebSocket(gateway);
            websocket.onopen = onOpen;
            websocket.onclose = onClose;
        }
        function onOpen(event) {
            console.log('Connection opened');
        }
        function onClose(event) {
            console.log('Connection closed');
            setTimeout(initWebSocket, 2000);
        }
        function onLoad(event) {
            initWebSocket();
            initButton();
        }
        function initButton() {
            document.getElementById('sliderPicker').addEventListener('touchmove', dor);
            document.getElementById('sliderPicker').addEventListener('mousemove', dor);
            document.getElementById('sliderPicker').addEventListener('touchend', notdor);
            document.getElementById('sliderPicker').addEventListener('mouseup', notdor);
            document.getElementById('swatch-grid').addEventListener('touchstart', dor);
            document.getElementById('swatch-grid').addEventListener('mousedown', dor);
            
            document.getElementById('sliderPicker').addEventListener('touchstart', dor);
            document.getElementById('sliderPicker').addEventListener('mousedown', dor);
        }
        function dor() {
            if (datawarna != oldwarna) {
                websocket.send(datawarna);
            }
            oldwarna = datawarna;
        }
        function notdor() {
            //            websocket.send('out');
        }
        var swatchGrid = document.getElementById('swatch-grid');
        swatchGrid.addEventListener('mousedown', function (e) {
            var clickTarget = e.target;
            // read data-color attribute
            if (clickTarget.dataset.color) {
                // update the color picker
                sliderPicker.color.set(clickTarget.dataset.color);
            }
        });
        swatchGrid.addEventListener('touchstart', function (e) {
            var clickTarget = e.target;
            // read data-color attribute
            if (clickTarget.dataset.color) {
                // update the color picker
                sliderPicker.color.set(clickTarget.dataset.color);
            }
        });
        var sliderPicker = new iro.ColorPicker("#sliderPicker", {
            width: 250,
            color: "rgb(255, 0, 0)",
            borderWidth: -1,
            borderColor: "#222438",
            layoutDirection: 'horizontal',
            layout: [
                {
                    component: iro.ui.Wheel,
                },
                {
                    component: iro.ui.Slider,
                    options: {
                        sliderType: 'saturation',
                        sliderSize: 60,
                    }
                },
                {
                    component: iro.ui.Slider,
                    options: {
                        sliderType: 'value',
                        sliderSize: 60,
                    }
                }, {
                    component: iro.ui.Slider,
                    options: {
                        sliderType: 'kelvin',
                        sliderShape: 'circle'
                    }
                },
            ]
        });
        var lamp = document.getElementById("lamp");
        sliderPicker.on(["color:init", "color:change"], function (color) {
            lamp.style.background = color.hexString;
            datawarna = color.hexString;
        });
    </script>
</body>

</html>
)rawliteral";

void notifyClients() {
  ws.textAll(String(ledState));
}
int htd(char H){
  switch(H){
    case '0':return 0;
    case '1':return 1;
    case '2':return 2;
    case '3':return 3;
    case '4':return 4;
    case '5':return 5;
    case '6':return 6;
    case '7':return 7;
    case '8':return 8;
    case '9':return 9;
    case 'a':return 10;
    case 'b':return 11;
    case 'c':return 12;
    case 'd':return 13;
    case 'e':return 14;
    case 'f':return 15;
  }
}

int hexToDec(String strHex){
  char H=strHex[0];
  int H1;
  switch(H){
    case '0':H1 = 0; break;
    case '1':H1 = 1; break;
    case '2':H1 = 2; break;
    case '3':H1 = 3; break;
    case '4':H1 = 4; break;
    case '5':H1 = 5; break;
    case '6':H1 = 6; break;
    case '7':H1 = 7; break;
    case '8':H1 = 8; break;
    case '9':H1 = 9; break;
    case 'a':H1 = 10; break;
    case 'b':H1 = 11; break;
    case 'c':H1 = 12; break;
    case 'd':H1 = 13; break;
    case 'e':H1 = 14; break;
    case 'f':H1 = 15; break;
  }
  int G1=H1;
  H=strHex[1];
  switch(H){
    case '0':H1 = 0; break;
    case '1':H1 = 1; break;
    case '2':H1 = 2; break;
    case '3':H1 = 3; break;
    case '4':H1 = 4; break;
    case '5':H1 = 5; break;
    case '6':H1 = 6; break;
    case '7':H1 = 7; break;
    case '8':H1 = 8; break;
    case '9':H1 = 9; break;
    case 'a':H1 = 10; break;
    case 'b':H1 = 11; break;
    case 'c':H1 = 12; break;
    case 'd':H1 = 13; break;
    case 'e':H1 = 14; break;
    case 'f':H1 = 15; break;
  }
  return ((G1*16)+H1);
//  return htd((strHex[0])*16)+htd(strHex[1]);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    datawarna=(char*)data;
    c[0]=hexToDec(datawarna.substring(1,3));
    c[1]=hexToDec(datawarna.substring(3,5));
    c[2]=hexToDec(datawarna.substring(5,7));
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
      case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
      case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
      case WS_EVT_PONG:
      case WS_EVT_ERROR:
        break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  
}

void setup(){
  pixels.begin();
//  myservo.attach(D2);
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Start server
  server.begin();
}
int color_r(int w){
  return (int)(w*0.90);
}

int color_g(int w){
  return (int)(w*0.4);
}

int color_b(int w){
  return (int)(w*1);
}


int warna(int w){
  return (int)(w*(kecerahan/100));
}

void statis(){
  int clocal[3];
  clocal[0]=c[0];
  clocal[1]=c[1];
  clocal[2]=c[2];
  for(i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(color_r(clocal[0]), color_g(clocal[1]), color_b(clocal[2])));
  }
  pixels.show();
}



void loop() {
  ws.cleanupClients();
  statis();
  if(false){
    Serial.print("R: ");
    Serial.print(datawarna.substring(1,3));
    Serial.print("  ");
    Serial.println(c[0]);
    Serial.print("G: ");
    Serial.print(datawarna.substring(3,5));
    Serial.print("  ");
    Serial.println(c[1]);
    Serial.print("B: ");
    Serial.print(datawarna.substring(5,7));
    Serial.print("  ");
    Serial.println(c[2]);
  }
  delay(30);
}
