import engine_main
import engine
import engine_io
import engine_audio
import engine_time
import engine_draw
from engine_resources import ToneSoundResource, FontResource
from engine_nodes import CameraNode, Text2DNode, Rectangle2DNode
from engine_math import Vector2

engine_audio.set_volume(0.2)

desc = Text2DNode(text="Check:\n  Buttons\n  Battery\n  RTC\n  Audio\n  Vibration\n  LED", position=Vector2(-10, -64), letter_spacing=1, line_spacing=1, color=engine_draw.blue)

cam = CameraNode()
rtc = Text2DNode(position=Vector2(-63, 50))
bat = Text2DNode(text="Battery:", position=Vector2(-63, 30))

tone = ToneSoundResource()
tone.frequency = 4000

pressed_total_count = 0


class Button():
    def __init__(self, button, name, y):
        self.btn = button
        self.pressed_before = False
        self.name = name
        self.text_node = Text2DNode(text="[] " + name, position=Vector2(-64, y), letter_spacing=1)


buttons = [
    Button(engine_io.A, "A", -64),
    Button(engine_io.B, "B", -56),
    Button(engine_io.UP, "UP", -48),
    Button(engine_io.DOWN, "DOWN", -40),
    Button(engine_io.LEFT, "LEFT", -32),
    Button(engine_io.RIGHT, "RIGHT", -24),
    Button(engine_io.LB, "LB", -16),
    Button(engine_io.RB, "RB", -8),
    Button(engine_io.MENU, "MENU", -0),
]


counter = 0
def pulse():
    global counter

    if counter < 60:
        engine_io.rumble(0.0)
        engine_io.indicator(True)
    elif counter >= 60 and counter < 120:
        engine_io.rumble(0.35)
        engine_io.indicator(False)
    else:
        counter = 0
    
    counter += 1


def test_buttons():
    global pressed_total_count

    pressed_now_count = 0
    any_pressed = False

    for button in buttons:
        if button.btn.is_pressed:
            any_pressed = True

            if (button.pressed_before is False and pressed_now_count == 0):
                button.text_node.text = "[X] " + button.name
                pressed_total_count += 1
                button.pressed_before = True
            
            pressed_now_count += 1
    
    if any_pressed:
       engine_audio.play(tone, 0, False)
    else:
        engine_audio.stop(0)


while True:
    if engine.tick():
        # Buttons
        test_buttons()

        # Rumble and LED
        pulse()
        
        # Battery
        bat.text = f"Battery: {round(engine_io.battery_voltage(), 1)}"

        # Date and time
        date = engine_time.datetime()
        
        if date == engine_time.RTC_I2C_ERROR:
            print("Got an RTC I2C Error?")
        else:
            rtc.text = f"{date[1]}/{date[2]}/{date[0]} : {date[3]}:{date[4]}:{date[5]}"

        if pressed_total_count == 9:
            full_rect = Rectangle2DNode(width=128, height=128, color=engine_draw.black)
            text = Text2DNode(text="/// TEST COMPLETE ///", color=engine_draw.green)
            text.position.x = -56

            print("COMPLETE")
            engine_io.rumble(0.0)
            engine_io.indicator(False)
            engine_audio.stop(0)

            engine.tick()

            break