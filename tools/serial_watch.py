import serial
import serial.tools.list_ports
import time
import sys
import os

PORT = sys.argv[1] if len(sys.argv) > 1 else "COM8"
DUR = float(sys.argv[2]) if len(sys.argv) > 2 else 25.0
LOG = os.path.join(os.path.dirname(os.path.abspath(__file__)), "serial_log.txt")

print("watching %s for %.0fs (auto-reconnect)" % (PORT, DUR))
t0 = time.time()
buf = []
ser = None
opened = 0
while time.time() - t0 < DUR:
    if ser is None:
        ports = [p.device for p in serial.tools.list_ports.comports()]
        if PORT in ports:
            try:
                ser = serial.Serial(PORT, 115200, timeout=0.2)
                opened += 1
                print("[%5.1fs] connected (open #%d)" % (time.time() - t0, opened))
            except Exception:
                ser = None
        if ser is None:
            time.sleep(0.15)
            continue
    try:
        d = ser.read(4096)
    except Exception as e:
        print("[%5.1fs] lost: %s" % (time.time() - t0, e))
        try:
            ser.close()
        except Exception:
            pass
        ser = None
        continue
    if d:
        buf.append(d)

if ser is not None:
    try:
        ser.close()
    except Exception:
        pass

raw = b"".join(buf)
txt = raw.decode("utf-8", "replace")
with open(LOG, "w", encoding="utf-8") as f:
    f.write(txt)
print("--- captured %d bytes, %d connect(s) ---" % (len(raw), opened))
print(txt)
