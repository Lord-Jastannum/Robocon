import numpy as np
import serial
import websockets
#from datetime import datetime
import json
import asyncio

#URL = "http://192.168.222.133:5000/receive"

# serial configuaration 
#SERIAL_PORT = "/dev/ttyUSB0"  # Or "/dev/ttyACM0" on linux
#SERIAL_PORT, BAUD_RATE = "COM3", 115200
#ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

#websockets variables
xf, yf = 0, 0 #fianl coordinates
xc ,yc = 0, 0 #current coordinates
client = set() #connected websocket clients

#handle incoming message
async def handle_connection(websocket, path):
    global xf, yf, clients
    clients.add(websocket) #store active clients
    try: 
        async for message in websocket:
            data = json.loads(message) #receive json 
            print("Received from laptop:", data)

            if "x" in data and "y" in data:
                xf, yf = data["x"], data["y"]  # Update destination coordinates
                response = json.dumps({"message": "Destination updated", "x":xf, "y":yf})
                await websocket.send(response)
    except Exception as e:
        print("Error:", e) #throw error
    finally:
        clients.remove(websocket) #remove laptop client

# send bot position to laptop
async def send_to_laptop(angle):
    global xc, yc, clients
    data = json.dumps({"x": xc, "y":yc, "angle": angle})
    
    #broadcasting to clients
    for client in clients:
        await client.send(data)
    print(f"Sent to Laptop: {data}")


# when autonomous mode, calculate next way point and send to teensy using serial; 0,0 if at destination
#UNCHANGED
def divide_path_step_cm(source, destination, steps):
    global xf, xc, yf, yc
    global xn, yn
    source = np.array(source)
    destination = np.array(destination)

    # Compute total distance
    direction = destination - source
    total_distance = np.linalg.norm(direction)

    if total_distance == 0:  # If source and destination are the same
        xn = 0
        yn = 0
        angle = 0

        data = f"{xn},{yn},{angle}\n"
        #ser.write(data.encode())
        print(f"Sent: {data.strip()}")

        return

    # Normalize direction vector
    unit_direction = direction / total_distance

    # Generate points at step intervals
    points = [tuple(source)]
    for d in np.arange(steps, total_distance, steps):
        new_point = source + unit_direction * d
        points.append(tuple(new_point))

    # Append destination if remaining distance is less than 2 cm
    if total_distance % 2 != 0:
        points.append(tuple(destination))

    lxy = points[1]

    xn = lxy[0]
    yn = lxy[1]

    xn -= source[0]
    yn -= source[1]

    xn = int(xn*4)
    yn = int(yn*4)
    angle = 0

    data = f"{xn},{yn},{angle}\n"
    #ser.write(data.encode())
    print(f"Sent: {data.strip()}")

    return


# read odom data coming from teensy using serial

def read_serial_data():
    try:
        data = 5 #ser.readline().decode('utf-8').strip()
        if data:
            auto_flag, x, y, angle = map(float, data.split(","))
            return x, y, angle, auto_flag
    except:
        return None


# receive from teensy
# if autonomous mode, send next waypoint to teensy
# send current position to laptop

async def receive_send():
    global xf, yf, xc, yc
    c = 0
    while True:
        coords = read_serial_data()  # receive from teensy
        print(coords)
        if coords:
            xc, yc, angle, auto_flag = coords
            if auto_flag == 1:  # if autonomous mode, send next waypoint to teensy
                source = (xc, yc)
                destination = (xf, yf)
                steps = 10
                divide_path_step_cm(source, destination, steps)

            if c % 50 == 0:
                await send_to_laptop(angle)  # send current position to laptop
                c = 0
            c += 1
        await asyncio.sleep(0.1)

#starting server
async def start_server():
    server = await websockets.serve(handle_connection, "10.9.72.108", 5000) #ip and port
    print ("Websocket Server Running on ws://10.9.72.108:5000")

    # Run both WebSocket server and Serial processing
    await asyncio.gather(server.wait_closed(), receive_send())


if __name__ == '__main__':
    asyncio.run(start_server())
    