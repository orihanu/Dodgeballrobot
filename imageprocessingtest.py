import cv2
import numpy as np
import serial
import time
import math

# ---------- SERIAL ----------
arduino = serial.Serial('COM6', 9600)
time.sleep(2)

# ---------- CAMERA ----------
cap = cv2.VideoCapture(0, cv2.CAP_DSHOW)

if not cap.isOpened():
    print("❌ Camera not working")
    exit()

last_command = ""
previous_center = None
previous_time = None
trajectory = []

while True:
    ret, frame = cap.read()
    if not ret:
        print("❌ Frame not received")
        break

    current_time = time.time()

    # ---------- PREPROCESS ----------
    frame = cv2.GaussianBlur(frame, (11, 11), 0)
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    lower_orange = np.array([5, 100, 100])
    upper_orange = np.array([20, 255, 255])

    mask = cv2.inRange(hsv, lower_orange, upper_orange)
    mask = cv2.erode(mask, None, iterations=2)
    mask = cv2.dilate(mask, None, iterations=2)

    contours, _ = cv2.findContours(
        mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE
    )

    if contours:
        largest = max(contours, key=cv2.contourArea)
        area = cv2.contourArea(largest)

        if area > 800:
            x, y, w, h = cv2.boundingRect(largest)

            cx = int(x + w / 2)
            cy = int(y + h / 2)
            current_center = (cx, cy)

            # ---------- TRAJECTORY ----------
            trajectory.append(current_center)
            if len(trajectory) > 20:
                trajectory.pop(0)

            # ---------- SPEED ----------
            speed = 0
            direction = "UNKNOWN"

            if previous_center is not None and previous_time is not None:
                px, py = previous_center

                dx = cx - px
                dy = cy - py

                distance = math.sqrt(dx**2 + dy**2)
                dt = current_time - previous_time

                if dt > 0:
                    speed = distance / dt  # pixel per second

                # ---------- DIRECTION ----------
                if abs(dx) > abs(dy):
                    if dx > 0:
                        direction = "RIGHT"
                    elif dx < 0:
                        direction = "LEFT"
                else:
                    if dy > 0:
                        direction = "DOWN (closer)"
                    elif dy < 0:
                        direction = "UP (far)"

                # ---------- TERMINAL OUTPUT ----------
                print("===================================")
                print(f"🎯 Position: ({cx}, {cy})")
                print(f"➡️ dx={dx}, dy={dy}")
                print(f"🛣️ Trajectory: {trajectory}")
                print(f"🧭 Direction: {direction}")
                print(f"🚀 Speed: {speed:.2f} px/sec")
                print(f"📏 Area: {area}")

            previous_center = current_center
            previous_time = current_time

            # ---------- DRAW ----------
            cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
            cv2.circle(frame, (cx, cy), 5, (0, 0, 255), -1)

            # draw trajectory line
            for i in range(1, len(trajectory)):
                cv2.line(frame, trajectory[i - 1],
                         trajectory[i], (255, 0, 0), 2)

            # ---------- LOGIC ----------
            command = ""

            if area > 4000:
                command = "DODGE"
            elif cx < 200:
                command = "LEFT"
            elif cx > 400:
                command = "RIGHT"

            # ---------- SEND ----------
            if command != "" and command != last_command:
                arduino.write((command + "\n").encode())
                print(f"🚨 Sent: {command}")
                last_command = command

    cv2.imshow("Webcam", frame)
    cv2.imshow("Mask", mask)

    if cv2.waitKey(1) & 0xFF == 27:
        break

cap.release()
arduino.close()
cv2.destroyAllWindows()
