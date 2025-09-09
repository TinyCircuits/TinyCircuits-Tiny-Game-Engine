import engine_main
import engine
import engine_io
import engine_audio
import engine_time
import engine_draw
from engine_resources import ToneSoundResource, FontResource
from engine_nodes import CameraNode, Text2DNode, Rectangle2DNode
from engine_math import Vector2
import machine

# Always have a camera
cam = CameraNode()

complete_text = None

# Creates nodes that cover entire screen indicating the test has been completed
def display_test_complete():
    global complete_text
    complete_text = Text2DNode(text="// TEST COMPLETE //", color=engine_draw.green, letter_spacing=1, position=Vector2(0, 54))
    print("COMPLETE")

def is_test_completed():
    try:
        f = open("completed.txt", "r")
        f.close()
        return True
    except:
        return False


# Check for test hasn't been completed yet, if it has, do not do the test
# If this excepts, the test has not been completed and pass, otherwise
# show that it has been completed
if is_test_completed():
    display_test_complete()

# Print ID for the test to track
print("ID:", list(machine.unique_id()))

engine_audio.set_volume(0.2)

desc = Text2DNode(text="Check:\n  Buttons\n  RTC\n  Audio\n  Vibration\n  LED", position=Vector2(25, -36), letter_spacing=1, line_spacing=2, color=engine_draw.yellow)
rtc = Text2DNode(position=Vector2(-3, 40))
bat = Text2DNode(text="Battery:", position=Vector2(-34, 30))

tone = ToneSoundResource()
tone.frequency = 4000

# Test ends when this reaches a certain number
pressed_total_count = 0

# Buttons to test
class Button():
    def __init__(self, button, name, x, y):
        self.btn = button
        self.pressed_before = False
        self.name = name
        self.text_node = Text2DNode(text="[] " + name, position=Vector2(-50+x, y), letter_spacing=1)

buttons = [
    Button(engine_io.A, "A", 0, -59),
    Button(engine_io.B, "B", 0, -50),
    Button(engine_io.UP, "UP", 3, -41),
    Button(engine_io.DOWN, "DOWN", 9, -32),
    Button(engine_io.LEFT, "LEFT", 9, -23),
    Button(engine_io.RIGHT, "RIGHT", 11, -14),
    Button(engine_io.LB, "LB", 3, -5),
    Button(engine_io.RB, "RB", 3, 4),
    Button(engine_io.MENU, "MENU", 9, 13),
]


# Function where each button to test is
# checked to be pressed. If a button is
# pressed, a sound is made
def test_buttons():
    global pressed_total_count

    # Setup flag for one button press at a time restriction
    # and flag to play audio if any button is pressed
    pressed_now_count = 0
    any_pressed = False

    # Go through all buttons in list and check if
    # they are pressed and track that
    for button in buttons:
        if button.btn.is_pressed:
            any_pressed = True

            # If the button has not been pressed before, and not other
            # buttons were pressed yet either, track the press
            if (button.pressed_before is False and pressed_now_count == 0):
                button.text_node.text = "[X] " + button.name
                pressed_total_count += 1
                button.pressed_before = True
            
            pressed_now_count += 1
    
    if any_pressed:
        engine_audio.play(tone, 0, False)
        engine_io.rumble(0.35)
        engine_io.indicator(False)
    else:
        engine_audio.stop(0)
        engine_io.rumble(0.0)
        engine_io.indicator(True)


while True:
    if engine.tick():
        # Buttons
        test_buttons()
        
        # Battery
        bat.text = f"Battery: {round(engine_io.battery_voltage(), 1)}"

        # Date and time
        date = engine_time.datetime()
        
        if date == engine_time.RTC_I2C_ERROR:
            print("Got an RTC I2C Error?")
            rtc.text = "RTC: ERROR!!!"
            rtc.position.x = -29
        else:
            rtc.text = f"RTC: {date[1]}/{date[2]}/{date[0]} : {date[3]}:{date[4]}:{date[5]}"
            rtc.position.x = -3

        if pressed_total_count == 9 and complete_text is None:
            # Write the file indicating this unit has been tested
            f = open("completed.txt", "w")
            f.write("COMPLETED")
            f.close()
            display_test_complete()


