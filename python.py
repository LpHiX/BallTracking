import socket
from vpython import *

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

tracker = box(pos=vec(0,0,0), size=vec(10,10,10), color=vec(0, 255, 0), make_trail=True, interval=1, retain=1000)
box(pos=vec(0, -40, 0), size=vec(200, 4, 200))

server_address = '127.0.0.1'
server_port = 54000

server = (server_address, server_port)
sock.bind(server)
print("Listening on " + server_address + ":" + str(server_port))

while True:
    k = keysdown()
    if 'backspace' in k: tracker.clear_trail()

    payload, client_address = sock.recvfrom(1024)
    pos_string = payload.decode('utf-8')
    #print(pos_string)
    strings = pos_string.split(" ", 4)
    x = float(strings[0])
    y = float(strings[1])
    z = float(strings[2])
    tracker.pos = vec(x, y, z)

