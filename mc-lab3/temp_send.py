import clr
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import sqlite3
from urllib.parse import urlparse, parse_qs
import threading
import time
from queue import Queue
import os
import traceback
from datetime import datetime  # <-- нове

# --- LibreHardwareMonitor ---
clr.AddReference(r"C:\\Uni\\Coursework\\LibreHardwareMonitor-net472\\LibreHardwareMonitorLib.dll")
from LibreHardwareMonitor.Hardware import Computer, SensorType

# --- Ініціалізація моніторингу ---
computer = Computer()
computer.IsCpuEnabled = True
computer.Open()

latest_temp = "0.0"
log_queue = Queue()

# --- Фонове оновлення температури ---
def update_temp_loop():
    global latest_temp
    while True:
        for hw in computer.Hardware:
            hw.Update()
            for sensor in hw.Sensors:
                if sensor.SensorType == SensorType.Temperature and sensor.Name == "CPU Package":
                    latest_temp = str(sensor.Value)
        time.sleep(10)  # Частота запитів з ESP — кожні 10 сек

# --- Логування даних у SQLite ---
def log_worker():
    db_path = os.path.abspath("logs.db")
    print(f"[DB Path] {db_path}")
    while True:
        data = log_queue.get()
        if data:
            try:
                temp, mode, pwm, pwm_percent = data
                timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')  # локальний час
                print(f"[Queue] Logging: temp={temp}, mode={mode}, pwm={pwm}, pwm%={pwm_percent}, time={timestamp}")
                with sqlite3.connect(db_path) as conn:
                    cursor = conn.cursor()
                    cursor.execute("""
                        CREATE TABLE IF NOT EXISTS temperature_logs (
                            id INTEGER PRIMARY KEY AUTOINCREMENT,
                            timestamp DATETIME,
                            temperature REAL,
                            mode TEXT,
                            pwm INTEGER,
                            pwm_percent INTEGER
                        )
                    """)
                    cursor.execute("""
                        INSERT INTO temperature_logs (timestamp, temperature, mode, pwm, pwm_percent)
                        VALUES (?, ?, ?, ?, ?)
                    """, (timestamp, temp, mode, pwm, pwm_percent))
                    cursor.execute("""
                        DELETE FROM temperature_logs
                        WHERE id NOT IN (
                            SELECT id FROM temperature_logs
                            ORDER BY timestamp DESC
                            LIMIT 1000
                        )
                    """)
                    conn.commit()
            except Exception as e:
                print(f"[Log Error] {e}")
                traceback.print_exc()
        log_queue.task_done()

# --- HTTP-обробник ---
class TempHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        parsed = urlparse(self.path)
        path = parsed.path
        query = parse_qs(parsed.query)

        self.send_header('Connection', 'close')
        self.send_header('Access-Control-Allow-Origin', '*')

        if path == '/temp':
            self.send_response(200)
            self.end_headers()
            self.wfile.write(latest_temp.encode())

        elif path == '/log':
            try:
                mode = query.get('mode', [''])[0]
                pwm = int(query.get('pwm', [0])[0])
                temp = float(query.get('temp', [0.0])[0])
                pwm_percent = round((pwm - 716) / 3.07) if pwm > 0 else 0

                log_queue.put((temp, mode, pwm, pwm_percent))

                self.send_response(200)
                self.end_headers()
                self.wfile.write(b"Queued")
            except Exception as e:
                self.send_response(500)
                self.end_headers()
                self.wfile.write(f"Error: {str(e)}".encode())
                traceback.print_exc()
        else:
            self.send_response(204)
            self.end_headers()

# --- Запуск потоків ---
threading.Thread(target=update_temp_loop, daemon=True).start()
threading.Thread(target=log_worker, daemon=True).start()

# --- Запуск сервера ---
if __name__ == '__main__':
    server = ThreadingHTTPServer(('0.0.0.0', 5000), TempHandler)
    print("Ultra-lightweight server with queue started on port 5000")
    server.serve_forever()
