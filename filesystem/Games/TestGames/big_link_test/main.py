import engine_main
import engine_io
import engine_link
import engine
from engine_nodes import CameraNode, Text2DNode, Rectangle2DNode
from engine_math import Vector2
import time

unique_buffer512 = bytearray("MTQVhtFdfq3CHYUPuUC1nqfghZyqOMKlWs21MnQYnqC9q1tER1xPG2YUog9y70yAkmp8p0XdVkdmao5iMk2azo9aWeqxkYB6VIpnDtY095IWpqXrtmeaCDo2cFfYo2MF8iClpBO1fdZAc6iILGoQlGPIUR9iESCerrK9BcDmX8uvDK3NZjn194QJyqxQYVGgFxdu68kzReP038hZDfOyfWmkBrG0cmNaJUrv9tHRMpgfZsFiXju5NM8QEqq8eRJPsyLKlAvO1IamAPuCxekV5u9kom7CPDQKAh1oNxP2zQg0uPqkUChHMSWLT8Td2jBsf1hMp4iIUfmm3D7kup0P2HaucKx3Zu1GNLZzFYwGT5SqhzkyIpQMMbgeDeBBd6y9cxrlCHUuiMOkzRmDp4tyii5PQ2TKTUJIxUmfRlo8a6I9ficIdng5cpZQtzF4nAAvnctKAfxmVNHwyIurQjIu4D6IJrpCXdSgEJeMXOiiuDOU7ReAQuJcP6Bs64lP8d3w".encode())
unique_buffer513 = bytearray("jMTQVhtFdfq3CHYUPuUC1nqfghZyqOMKlWs21MnQYnqC9q1tER1xPG2YUog9y70yAkmp8p0XdVkdmao5iMk2azo9aWeqxkYB6VIpnDtY095IWpqXrtmeaCDo2cFfYo2MF8iClpBO1fdZAc6iILGoQlGPIUR9iESCerrK9BcDmX8uvDK3NZjn194QJyqxQYVGgFxdu68kzReP038hZDfOyfWmkBrG0cmNaJUrv9tHRMpgfZsFiXju5NM8QEqq8eRJPsyLKlAvO1IamAPuCxekV5u9kom7CPDQKAh1oNxP2zQg0uPqkUChHMSWLT8Td2jBsf1hMp4iIUfmm3D7kup0P2HaucKx3Zu1GNLZzFYwGT5SqhzkyIpQMMbgeDeBBd6y9cxrlCHUuiMOkzRmDp4tyii5PQ2TKTUJIxUmfRlo8a6I9ficIdng5cpZQtzF4nAAvnctKAfxmVNHwyIurQjIu4D6IJrpCXdSgEJeMXOiiuDOU7ReAQuJcP6Bs64lP8d3w".encode())

camera = CameraNode()

text0_help = Text2DNode(position=Vector2(0, -50), text="Press B to start")
text1_help = Text2DNode(position=Vector2(0, -50+10), text="LB to send 512")
text2_help = Text2DNode(position=Vector2(0, -50+20), text="RB to send 512+1")
text3_help = Text2DNode(position=Vector2(0, -50+30), text="Menu to read all/test")

text4_test = Text2DNode(position=Vector2(0, -50+40), text="test: reset")

text5_info = Text2DNode(position=Vector2(0, -50+65), text="available: ~\nconnected: ~\nis_host: ~\nis_started: ~", line_spacing=3)


def connected():
    engine_link.clear_send()
    engine_link.clear_read()

engine_link.set_connected_cb(connected)

while True:
    if engine.tick() is False:
        continue

    if engine_io.B.is_just_pressed:
        engine_link.start()
        text4_test.text="test: reset"
    elif engine_io.LB.is_just_pressed:
        engine_link.send(unique_buffer512)
    elif engine_io.RB.is_just_pressed:
        engine_link.send(unique_buffer513)
    elif engine_io.MENU.is_just_pressed:
        read = engine_link.read()

        if read == unique_buffer512:
            text4_test.text="test: matched"
        else:
            text4_test.text="test: not matched"



    text5_info.text = "available: " + str(engine_link.available()) + "\nconnected: " + str(engine_link.connected()) + "\nis_host: " + str(engine_link.is_host()) + "\nis_started: " + str(engine_link.is_started())