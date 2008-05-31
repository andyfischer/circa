
import pretty_print

class Browser(object):
    def __init__(self, codeUnit=None):
        self.codeUnit = codeUnit

    def doCommand(cmd):
        if cmd == 'list':
            for term in self.codeUnit.allTerms:
                print pretty_print.term(self.codeUnit,term)


def main():
    browser = Browser(builtins.KERNEL)
    while True:
        cmd = get_input('> ')

        if cmd == 'exit':
            return

        browser.doCommand(cmd)

if __name__ == "__main__":
    main()
