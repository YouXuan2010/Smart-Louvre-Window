#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
#include <Arduino.h>
#include <Hash.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "Uniz";
const char* password = "zinu2027";

const int PDLC = 2;
const int ACUP = 4;
const int ACDOWN = 5;

#define DHTPIN 0     // Digital pin connected to the DHT sensor

// Uncomment the type of sensor in use:
#define DHTTYPE    DHT11     // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

// current temperature & humidity, updated in loop()
float t = 0.0;
float h = 0.0;

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;    // will store last time DHT was updated

// Updates DHT readings every 10 seconds
const long interval = 10000; 

// HTML web page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
  <head>
    <title>AutoTint</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body { font-family: Arial; text-align: center; margin:0px auto; padding-top: 30px;
      background-color: lightblue;}
      .logo img{
        float: left;
        width: 150px;
        height: auto;
      }
      .button {
        padding: 10px 20px;
        font-size: 24px;
        text-align: center;
        outline: none;
        color: #fff;
        background-color: #2f4468;
        border: none;
        border-radius: 5px;
        box-shadow: 0 6px #999;
        cursor: pointer;
        -webkit-touch-callout: none;
        -webkit-user-select: none;
        -khtml-user-select: none;
        -moz-user-select: none;
        -ms-user-select: none;
        user-select: none;
        -webkit-tap-highlight-color: rgba(0,0,0,0);
      }  
      .button:hover {background-color: #1f2e45}
      .button:active {
        background-color: #1f2e45;
        box-shadow: 0 4px #666;
        transform: translateY(2px);
      }
      title { font-size: 3.0rem; }
      h1 { font-size: 3.0rem; }
      h2 { font-size: 2.5rem; }
      p { font-size: 2.0rem; }
        .units { font-size: 1.2rem; }
        .dht-labels{
           font-size: 1.5rem;
           vertical-align:middle;
           padding-bottom: 15px;
         }
     
        }
    </style>
  </head>
  <body>
    <h1>AutoTint</h1>
    <button class="button" onclick="toggleCheckbox('PDLCon');"  onclick="changeColor();">PDLC On</button>
    <button class="button" onclick="toggleCheckbox('PDLCoff');">PDLC Off</button>
    <br/>
    <br/>
    <br/>
    <button class="button" onmousedown="toggleCheckbox('UP');" ontouchstart="toggleCheckbox('UP');" onmouseup="toggleCheckbox('STOP');" ontouchend="toggleCheckbox('STOP');">Open Window</button>
    <br/>
    <br/>
    <br/>
    <button class="button" onmousedown="toggleCheckbox('DOWN');" ontouchstart="toggleCheckbox('DOWN');" onmouseup="toggleCheckbox('STOP');" ontouchend="toggleCheckbox('STOP');">Close Window</button>
    <br/>
    <br/>
    <br/>
    <h2>Weather Conditions</h2>
    <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
    </p>
    <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">%</sup>
    </p>
    
   <script>
   function toggleCheckbox(x) {
     var xhr = new XMLHttpRequest();
     xhr.open("GET", "/" + x, true);
     xhr.send();
   }

   setInterval(function ( ) {
     var xhttp = new XMLHttpRequest();
     xhttp.onreadystatechange = function() {
       if (this.readyState == 4 && this.status == 200) {
         document.getElementById("temperature").innerHTML = this.responseText;
       }
      };
     xhttp.open("GET", "/temperature", true);
     xhttp.send();
     }, 10000 ) ;

   setInterval(function ( ) {
     var xhttp = new XMLHttpRequest();
     xhttp.onreadystatechange = function() {
       if (this.readyState == 4 && this.status == 200) {
         document.getElementById("humidity").innerHTML = this.responseText;
        }
     };
    xhttp.open("GET", "/humidity", true);
    xhttp.send();
    }, 10000 ) ;
  </script>
  </body>

</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(t);
  }
  else if(var == "HUMIDITY"){
    return String(h);
  }
  return String();
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  dht.begin();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
  }
// Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  /*
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("ESP IP Address: http://");
  Serial.println(WiFi.localIP());*/
  
  pinMode(PDLC, OUTPUT);
  pinMode(ACUP, OUTPUT);
  pinMode(ACDOWN, OUTPUT);
  digitalWrite(PDLC, LOW);
  digitalWrite(ACUP, LOW);
  digitalWrite(ACDOWN, LOW);
  
  // Send web page to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Receive an HTTP GET request
  server.on("/PDLCon", HTTP_GET, [] (AsyncWebServerRequest *request) {
    digitalWrite(PDLC, LOW);
    request->send(200, "text/plain", "ok");
  });

  // Receive an HTTP GET request
  server.on("/PDLCoff", HTTP_GET, [] (AsyncWebServerRequest *request) {
    digitalWrite(PDLC, HIGH);
    request->send(200, "text/plain", "ok");
  });

  // Receive an HTTP GET request
  server.on("/UP", HTTP_GET, [] (AsyncWebServerRequest *request) {
    digitalWrite(ACDOWN, HIGH);
    request->send(200, "text/plain", "ok");
  });

// Receive an HTTP GET request
  server.on("/DOWN", HTTP_GET, [] (AsyncWebServerRequest *request) {
    digitalWrite(ACUP, HIGH);
    request->send(200, "text/plain", "ok");
  });

// Receive an HTTP GET request
  server.on("/STOP", HTTP_GET, [] (AsyncWebServerRequest *request) {
    digitalWrite(ACUP, LOW);
    digitalWrite(ACDOWN, LOW);
    request->send(200, "text/plain", "ok");
  });

// TEMPERATURE
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(h).c_str());
  });
  
  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;
    // Read temperature as Celsius (the default)
    float newT = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    //float newT = dht.readTemperature(true);
    // if temperature read failed, don't change t value
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      t = newT;
      Serial.println(t);
    }
    // Read Humidity
    float newH = dht.readHumidity();
    // if humidity read failed, don't change h value 
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      h = newH;
      Serial.println(h);
    }
  }
  //if (h > 70.0){
  //  digitalWrite(ACDOWN, HIGH);
  //}
}
