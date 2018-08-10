import socket
import struct

import pyautogui

nintendo_ip = "10.42.0.251"
nintendo_port = 8080

nds_client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
nds_client.connect((nintendo_ip, nintendo_port))


def set_nth_bit(n):
    return 1 << n


mask_a_key = set_nth_bit(0)

while True:
    nds_client.send('A'.encode('ascii'))
    packet = nds_client.recv(2) #otrzymujemy pakiet z naszymi klawiszami
    keys_held_little_endian = struct.unpack('<H', packet) #'H' oznacza unsigned short, '<' oznacza little endian
    print("Received as bytes : " + str(bin(keys_held_little_endian[0])))
    print("Received as integer : " + str(keys_held_little_endian[0]))
    key_a_pressed = (keys_held_little_endian[0] & mask_a_key) != 0
    print("Masked 1st bit - A key - " + str((key_a_pressed)))

    if key_a_pressed:
        pyautogui.press('space')
