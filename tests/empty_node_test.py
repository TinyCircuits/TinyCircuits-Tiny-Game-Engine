import gc
import engine
import engine_debug
from engine_nodes import EmptyNode
from common import run_tests

engine_debug.debug_enable_all()
engine.init()


test_list = [
    
]


run_tests(test_list)