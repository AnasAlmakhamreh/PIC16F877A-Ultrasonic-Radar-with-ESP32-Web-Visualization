
#include <WiFi.h>
#include <WebServer.h>

/* ---------- WiFi ---------- */
const char* ssid = "PIC_Radar_System";
const char* password = "12345678";

/* ---------- Web Server ---------- */
WebServer server(80);

/* ---------- Data ---------- */
volatile int distance_cm = 0;

#define SAMPLE_COUNT 60
int history[SAMPLE_COUNT];
int idx = 0;

/* ---------- Web Page ---------- */
void handleRoot() {
  server.send(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>PIC Radar</title>
<style>
body {
  background:#111;
  color:white;
  font-family:Arial;
  text-align:center;
}
canvas {
  background:#222;
  border:1px solid #555;
}
.stats {
  margin-top:10px;
}
</style>
</head>
<body>

<h2>PIC Ultrasonic Radar</h2>
<h1 id="val">---</h1>

<canvas id="graph" width="320" height="160"></canvas>

<div class="stats">
  <p>Min: <span id="min">---</span> cm</p>
  <p>Max: <span id="max">---</span> cm</p>
</div>

<script>
const canvas = document.getElementById("graph");
const ctx = canvas.getContext("2d");

function drawGraph(data, color) {
  ctx.clearRect(0,0,canvas.width,canvas.height);

  ctx.strokeStyle = color;
  ctx.lineWidth = 2;
  ctx.beginPath();

  for (let i = 0; i < data.length; i++) {
    let x = i * (canvas.width / (data.length - 1));
    let y = canvas.height - (data[i] / 200) * canvas.height;
    if (i === 0) ctx.moveTo(x,y);
    else ctx.lineTo(x,y);
  }
  ctx.stroke();
}

setInterval(() => {
  fetch("/data")
    .then(r => r.json())
    .then(obj => {
      document.getElementById("val").innerText = obj.current + " cm";
      document.getElementById("min").innerText = obj.min;
      document.getElementById("max").innerText = obj.max;

      let color = "green";
      if (obj.current < 15) color = "red";
      else if (obj.current < 30) color = "orange";

      drawGraph(obj.history, color);
    });
}, 200);
</script>

</body>
</html>
)rawliteral");
}

/* ---------- JSON Data ---------- */
void handleData() {
  int minVal = 1000;
  int maxVal = 0;

  for (int i = 0; i < SAMPLE_COUNT; i++) {
    if (history[i] < minVal) minVal = history[i];
    if (history[i] > maxVal) maxVal = history[i];
  }

  String json = "{";
  json += "\"current\":" + String(distance_cm) + ",";
  json += "\"min\":" + String(minVal) + ",";
  json += "\"max\":" + String(maxVal) + ",";
  json += "\"history\":[";

  for (int i = 0; i < SAMPLE_COUNT; i++) {
    int pos = (idx + i) % SAMPLE_COUNT;
    json += String(history[pos]);
    if (i < SAMPLE_COUNT - 1) json += ",";
  }

  json += "]}";

  server.send(200, "application/json", json);
}

/* ---------- Setup ---------- */
void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, -1); // RX only

  WiFi.softAP(ssid, password);

  for (int i = 0; i < SAMPLE_COUNT; i++) history[i] = 0;

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
}

/* ---------- Loop ---------- */
void loop() {
  server.handleClient();

  if (Serial2.available()) {
    String line = Serial2.readStringUntil('\n');
    line.trim();
    int d = line.toInt();

    distance_cm = d;
    history[idx] = d;
    idx = (idx + 1) % SAMPLE_COUNT;
  }
}
