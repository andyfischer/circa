
import Circa
import pyglet, pyglet.window, pyglet.font

Circa.initialize()

class Window(object):
    name = 'pyglet_window'
    pure = False
    inputs = []
    output = 'void'

    def initialize(self):
        print "initialze called"
        self.window = pyglet.window.Window()

    def evaluate(self):
        self.window.dispatch_events()
        self.window.clear()
        #self.window.draw()
        self.window.flip()

Circa.importFunction(Window)

"""
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
"""

Circa.startReplLoop()
