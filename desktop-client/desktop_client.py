import socket
import struct

import pyautogui

nintendo_ip = "10.42.0.147"
nintendo_port = 8080
nds_client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
nds_client.connect((nintendo_ip, nintendo_port))


def set_nth_bit(n):
    return 1 << n


mask_a_key = set_nth_bit(0)

while True:
    # wysyłamy dowolny znak w ramach synchronizacji
    nds_client.send('A'.encode('ascii'))
    # teraz otrzymujemy pakiet z naszymi klawiszami:
    packet = nds_client.recv(2)
    if len(packet) == 0:
        print("NDS zamknął połączenie")
        nds_client.close()
        break
    # odkodowanie otrzymanych bitów w taki sposób,
    # aby ich kolejność była taka sama jak na NDS.
    # 'H' oznacza unsigned short, '<' oznacza little endian
    keys_held_little_endian = struct.unpack('<H', packet)[0]
    print("Otrzymane bajty : " + str(bin(keys_held_little_endian)))
    print("Otrzymane bajty jako liczba : " + str(keys_held_little_endian))
    key_a_pressed = (keys_held_little_endian & mask_a_key) != 0
    print("Czy wciśnięto A - " + str((key_a_pressed)))
    if key_a_pressed:
        pyautogui.press('space')

print("Koniec")
