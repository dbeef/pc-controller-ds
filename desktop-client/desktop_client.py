import socket
import threading
import time

import pyautogui

nintendo_ip = "10.42.0.251"
nintendo_port = 8080

nds_client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
nds_client.connect((nintendo_ip, nintendo_port))
keys_pressed = []


def handle_keys():
    print("Pressed: " + keys_pressed)  

    # NDS - dpad
    if keys_pressed[0] == '1':
        pyautogui.press('right')

    if keys_pressed[1] == '1':
        pyautogui.press('left')

    if keys_pressed[2] == '1':
        pyautogui.press('down')

    if keys_pressed[3] == '1':
        pyautogui.press('up')

    # NDS - keypad

    if keys_pressed[4] == '1':
        if keys_pressed[8] == '1':
            pyautogui.moveRel(xOffset=None, yOffset=-4, duration=0.12)
        else:
            pyautogui.moveRel(xOffset=None, yOffset=-36, duration=0.1)

    if keys_pressed[5] == '1':
        if keys_pressed[8] == '1':
            pyautogui.moveRel(xOffset=None, yOffset=4, duration=0.12)
        else:
            pyautogui.moveRel(xOffset=None, yOffset=36, duration=0.1)

    if keys_pressed[6] == '1':
        if keys_pressed[8] == '1':
            pyautogui.moveRel(xOffset=-4, yOffset=None, duration=0.12)
        else:
            pyautogui.moveRel(xOffset=-36, yOffset=None, duration=0.1)

    if keys_pressed[7] == '1':
        if keys_pressed[8] == '1':
            pyautogui.moveRel(xOffset=4, yOffset=None, duration=0.12)
        else:
            pyautogui.moveRel(xOffset=36, yOffset=None, duration=0.1)

    # NDS - R bumper

    if keys_pressed[9] == '1':
        pyautogui.press('space')  # ctrl-c to copy

    # NDS SELECT / START

    if keys_pressed[10] == '1':
        pyautogui.click()  # ctrl-c to copy

    if keys_pressed[11] == '1':
        pyautogui.rightClick()  # ctrl-c to copy


key_thread = threading.Thread(target=handle_keys, args=[])
while True:
    timestamp_total = time.time()
    nds_client.send('A'.encode('ascii'))
    keys_pressed = (nds_client.recv(12)).decode("ascii")

    if key_thread.is_alive():
        key_thread.join()

    key_thread = threading.Thread(target=handle_keys, args=[])
    key_thread.start()
    total_time = time.time() - timestamp_total
    print("Total time: " + str(total_time))
