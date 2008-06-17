
"""
import Circa

demo_ca = Circa.loadFile("demo.ca")

class Draggable(Circa.PythonFunction):
   pureFunction=False
"""

import Circa.core.bootstrap
from Circa.core import builtins
from Circa.common import function_builder

import pyglet, pyglet.window, pyglet.font

class Window(function_builder.BaseFunction):
    pureFunction = False
    inputTypes = builtins.VOID_TYPE
    outputType = builtins.VOID_TYPE

    @staticmethod
    def initialize(cxt):
        cxt.caller().state = pyglet.window.Window()

    @staticmethod
    def evaluate(cxt):
        pass

class HelloWorldWindow(pyglet.window.Window):
    def __init__(self):
        super(HelloWorldWindow, self).__init__()

        ft = pyglet.font.load('Arial', 36)
        self.text = pyglet.font.Text(ft, 'Hello, World!')

    def draw(self):
        self.text.draw()

    def run(self):
        while not self.has_exit:
            self.dispatch_events()

            self.clear()
            self.draw()
            self.flip()

if __name__ == '__main__':
    HelloWorldWindow().run()
