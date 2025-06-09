#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <ESP8266WebServer.h>

enum FanMode { AUTO, MANUAL };

extern ESP8266WebServer server;
extern FanMode currentMode;
extern int manualPWMPercent;

extern float currentTemp;
extern int currentPWM;
extern const int pwmPin;

extern int calculateFanPWM(float temp);
extern void updateFanSpeed(float temp);

// --- Temperature History (last 20 readings) ---
const int MAX_HISTORY = 20;
float tempHistory[MAX_HISTORY];
int tempIndex = 0;
int tempCount = 0;

void addTempToHistory(float temp) {
  tempHistory[tempIndex] = temp;
  tempIndex = (tempIndex + 1) % MAX_HISTORY;
  if (tempCount < MAX_HISTORY) tempCount++;
}

String getTemperatureHistoryJSON() {
  String result = "";
  int index = (tempIndex - tempCount + MAX_HISTORY) % MAX_HISTORY;
  for (int i = 0; i < tempCount; i++) {
    if (i > 0) result += ",";
    result += String(tempHistory[(index + i) % MAX_HISTORY], 1);
  }
  return result;
}

// --- Web Interface Routes ---
void setupWebInterfaceRoutes() {
  server.on("/", []() {
    String html = R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head>
        <meta charset="UTF-8">
        <title>Fan Control</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>
          body {
            font-family: Arial, sans-serif;
            background: #f4f6f8;
            margin: 0;
            padding: 0;
            color: #333;
          }
          header {
            background: #2c3e50;
            color: white;
            padding: 20px;
            text-align: center;
            font-size: 1.5em;
          }
          main {
            padding: 30px;
            text-align: center;
          }
          footer {
            background: #2c3e50;
            color: white;
            text-align: center;
            padding: 15px;
            margin-top: 30px;
          }
          button {
            padding: 10px 20px;
            margin: 10px;
            font-size: 1em;
            background-color: #3498db;
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
          }
          button:hover {
            background-color: #2980b9;
          }
          input[type=range] {
            width: 80%;
          }
          #manual {
            margin-top: 20px;
          }
          #temp {
            font-size: 1.5em;
            font-weight: bold;
            color: #e74c3c;
          }
          canvas {
            width: 1000px;
            height: 500px;
            margin: 30px auto;
            display: block;
            background: white;
            border: 1px solid #ccc;
            border-radius: 10px;
          }
        </style>
        <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
      </head>
      <body>
        <header>
          Coursework by Zaplatynskyi Ihor (IR-23)
        </header>
        <main>
          <h2>Fan Control</h2>
          <div>Current Temperature: <span id="temp">--</span>&deg;C</div>
          <button onclick="setMode('auto')">Auto</button>
          <button onclick="setMode('manual')">Manual</button>
          <div id="manual" style="display:none;">
            <input type="range" min="0" max="100" id="slider" oninput="update(this.value)">
            <div>Fan Speed: <span id="val">0</span>%</div>
          </div>
          <canvas id="tempChart" width="300" height="150"></canvas>
        </main>
        <footer>
          Lviv Polytechnic National University<br/>
          <small>2025</small>
        </footer>
        <script>
          let mode = "auto";
          let chart;

          function setMode(m) {
            mode = m;
            fetch("/mode?set=" + m);
            document.getElementById("manual").style.display = m === "manual" ? "block" : "none";
          }

          function update(v) {
            document.getElementById("val").innerText = v;
            fetch("/set_pwm?val=" + v);
          }

          function initChart() {
            const ctx = document.getElementById("tempChart").getContext("2d");
            chart = new Chart(ctx, {
              type: "line",
              data: {
                labels: Array.from({length: 20}, (_, i) => i + 1),
                datasets: [{
                  label: "CPU Temperature",
                  data: [],
                  borderColor: "#e67e22",
                  backgroundColor: "rgba(230, 126, 34, 0.1)",
                  fill: true,
                  tension: 0.2,
                  borderWidth: 2,
                  pointRadius: 4,
                  pointHoverRadius: 6
                }]
              },
              options: {
                plugins: {
                  legend: {
                    display: false
                  }
                },
                scales: {
                  y: {
                    beginAtZero: false,
                    title: {
                      display: true,
                      text: "°C",
                      font: {
                        size: 16
                      }
                    },
                    ticks: {
                      font: {
                        size: 14
                      }
                    }
                  },
                  x: {
                    display: false
                  }
                }
              }
            });
          }

          function poll() {
            fetch("/data")
              .then(r => r.json())
              .then(d => {
                document.getElementById("temp").innerText = d.temp;
                setMode(d.mode);

                if (chart && d.history) {
                  chart.data.datasets[0].data = d.history;
                  chart.update();
                }

                // Оновлення не кожну секунду, а кожні 10 сек (синхронно з ESP)
                setTimeout(poll, 10000);
              })
              .catch(err => {
                console.error("Poll error:", err);
                setTimeout(poll, 10000);
              });
          }

          window.onload = () => {
            initChart();
            poll();
          };
        </script>
      </body>
      </html>
    )rawliteral";
    server.send(200, "text/html", html);
  });

  server.on("/mode", []() {
    if (server.hasArg("set"))
      currentMode = (server.arg("set") == "manual") ? MANUAL : AUTO;
    server.send(200, "text/plain", "OK");
  });

  server.on("/set_pwm", []() {
    if (server.hasArg("val")) {
      manualPWMPercent = server.arg("val").toInt();
      if (currentMode == MANUAL) {
        int pwm = (manualPWMPercent == 0) ? 0 : map(manualPWMPercent, 0, 100, 716, 1023);
        analogWrite(pwmPin, pwm);
        currentPWM = pwm;
      }
    }
    server.send(200, "text/plain", "OK");
  });

  server.on("/data", []() {
    addTempToHistory(currentTemp);
    String json = "{\"temp\":" + String(currentTemp, 1) +
                  ",\"mode\":\"" + (currentMode == MANUAL ? "manual" : "auto") +
                  "\",\"history\":[" + getTemperatureHistoryJSON() + "]}";
    server.send(200, "application/json", json);
  });
}

void updateWebInterface() {
  if (currentMode == AUTO) {
    int targetPWM = calculateFanPWM(currentTemp);
    if (targetPWM != currentPWM) updateFanSpeed(currentTemp);
  }
}

#endif
