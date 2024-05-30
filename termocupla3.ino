#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "max6675.h" //https://github.com/adafruit/MAX6675-library

float t, tF;
const char* ssid = "apple";  // Enter SSID here
const char* password = "gibran1234";  // Enter Password here
ESP8266WebServer server(80);        
int thermoDO = D6; // SO of  MAX6675 module to D6
int thermoCS = D8; // CS of MAX6675 module to D8
int thermoCLK = D5; // SCK of MAX6675 module to D5

const int led1Pin = D2; // Replace with your chosen pin for LED 1
const int led2Pin = D4; // Replace with your chosen pin for LED 2

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

// Arrays to store temperature data
float tempC[60]; // Store last 60 readings (1 minute if updated every second)
float tempF[60]; // Store last 60 readings (1 minute if updated every second)
unsigned long previousMillis = 0;
const long interval = 1000; // Interval to read temperature (1 second)

void setup() {
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
  Serial.begin(115200);
  delay(100);
 
  Serial.println("Connecting to ");
  Serial.println(ssid);
  // Connect to your local Wi-Fi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  
  Serial.println(WiFi.localIP());
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    readTemperature();
  }
}

void readTemperature() {
  // Shift all the data to the left to make room for the new reading
  for (int i = 0; i < 59; i++) {
    tempC[i] = tempC[i + 1];
    tempF[i] = tempF[i + 1];
  }
  // Read new temperature
  tempC[59] = thermocouple.readCelsius();
  tempF[59] = thermocouple.readFahrenheit();
  Serial.print("Deg C = ");
  Serial.println(tempC[59]);
  Serial.print("\t Deg F = ");
  Serial.println(tempF[59]);
  Serial.println(); 

  // LED control based on temperature
  if (tempC[59] >= 30) {
    digitalWrite(led1Pin, HIGH); // Turn on LED 1
    digitalWrite(led2Pin, HIGH); // Turn on LED 2
  } else {
    digitalWrite(led1Pin, LOW); // Turn off LED 1
    digitalWrite(led2Pin, LOW); // Turn off LED 2
  }
}

void handle_OnConnect() {
  t = thermocouple.readCelsius();
  tF = thermocouple.readFahrenheit();
  server.send(200, "text/html", SendHTML(t, tF)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float t, float tF) {
  String ptr = "<!DOCTYPE html>";
  ptr += "<html>";
  ptr += "<head>";
  ptr += "<title>Reballing tecnico</title>";
  ptr += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  ptr += "<link href='https://fonts.googleapis.com/css?family=Open+Sans:300,400,600' rel='stylesheet'>";
  ptr += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>";
  ptr += "<style>";
  ptr += "html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #444444;}";
  ptr += "body { margin: 0px; } ";
  ptr += "h1 { margin: 50px auto 30px; } ";
  ptr += ".side-by-side { display: table-cell; vertical-align: middle; position: relative; }";
  ptr += ".text { font-weight: 600; font-size: 19px; width: 200px; }";
  ptr += ".reading { font-weight: 300; font-size: 50px; padding-right: 25px; }";
  ptr += ".t .reading { color: #F29C1F; }";
  ptr += ".tF .reading { color: #3B97D3; }";
  ptr += ".superscript { font-size: 17px; font-weight: 600; position: absolute; top: 10px; }";
  ptr += ".data { padding: 10px; }";
  ptr += ".container { display: table; margin: 0 auto; }";
  ptr += ".icon { width:65px }";
  ptr += "</style>";
  ptr += "<script>\n";
  ptr += "setInterval(loadDoc, 1000);\n";
  ptr += "function loadDoc() {\n";
  ptr += "  var xhttp = new XMLHttpRequest();\n";
  ptr += "  xhttp.onreadystatechange = function() {\n";
  ptr += "    if (this.readyState == 4 && this.status == 200) {\n";
  ptr += "      document.body.innerHTML = this.responseText;\n";
  ptr += "    }\n";
  ptr += "  };\n";
  ptr += "  xhttp.open('GET', '/', true);\n";
  ptr += "  xhttp.send();\n";
  ptr += "}\n";
  ptr += "window.onload = function() {\n";
  ptr += "  var ctx = document.getElementById('tempChart').getContext('2d');\n";
  ptr += "  window.tempChart = new Chart(ctx, {\n";
  ptr += "    type: 'line',\n";
  ptr += "    data: {\n";
  ptr += "      labels: Array.from({ length: 60 }, (v, k) => k + 1),\n";
  ptr += "      datasets: [{\n";
  ptr += "        label: 'Temperature (°C)',\n";
  ptr += "        data: [" + joinArray(tempC, 60) + "],\n";
  ptr += "        borderColor: 'rgba(242, 156, 31, 1)',\n";
  ptr += "        fill: false\n";
  ptr += "      }, {\n";
  ptr += "        label: 'Temperature (°F)',\n";
  ptr += "        data: [" + joinArray(tempF, 60) + "],\n";
  ptr += "        borderColor: 'rgba(59, 151, 211, 1)',\n";
  ptr += "        fill: false\n";
  ptr += "      }]\n";
  ptr += "    },\n";
  ptr += "    options: {\n";
  ptr += "      scales: {\n";
  ptr += "        x: {\n";
  ptr += "          title: {\n";
  ptr += "            display: true,\n";
  ptr += "            text: 'Time (s)'\n";
  ptr += "          }\n";
  ptr += "        },\n";
  ptr += "        y: {\n";
  ptr += "          title: {\n";
  ptr += "            display: true,\n";
  ptr += "            text: 'Temperature'\n";
  ptr += "          }\n";
  ptr += "        }\n";
  ptr += "      }\n";
  ptr += "    }\n";
  ptr += "  });\n";
  ptr += "};\n";
  ptr += "function updateChart() {\n";
  ptr += "  window.tempChart.data.datasets[0].data = [" + joinArray(tempC, 60) + "];\n";
  ptr += "  window.tempChart.data.datasets[1].data = [" + joinArray(tempF, 60) + "];\n";
  ptr += "  window.tempChart.update();\n";
  ptr += "}\n";
  ptr += "</script>\n";
  ptr += "</head>";
  ptr += "<body>";
  ptr += "<h1>ESP8266 Weather Station</h1>";
  ptr += "<div class='container'>";
  ptr += "<div class='data t'><div class='side-by-side text'>Temperature Celsius</div><div class='side-by-side reading'>";
  ptr += (int)t;
  ptr += "<span class='superscript'>&deg;C</span></div></div>";
  ptr += "<div class='data tF'><div class='side-by-side text'>Temperature Fahrenheit</div><div class='side-by-side reading'>";
  ptr += (int)tF;
  ptr += "<span class='superscript'>&deg;F</span></div></div>";
  ptr += "</div>";
  ptr += "<canvas id='tempChart' width='400' height='200'></canvas>";
  ptr += "</body>";
  ptr += "</html>";
  return ptr;
}

String joinArray(float arr[], int length) {
  String result = "";
  for (int i = 0; i < length; i++) {
    result += String(arr[i]);
    if (i < length - 1) {
      result += ", ";
    }
  }
  return result;
}
