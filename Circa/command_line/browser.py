
from string import Template
from Circa.core import (builtins, ca_type, ca_function)


class Browser(object):
    def __init__(self, codeUnit=None):
        self.codeUnit = codeUnit
    def doCommand(self, cmd):
        if cmd == 'list':
            for term in self.codeUnit.allTerms:
                print self.describeTerm(term, "$id: $func_name($input_ids) = $value")
        else:
            print "Unrecognized command"
    def getIdentifier(self, term):
        return self.codeUnit.getIdentifier(term)

    def describeTerm(self, term, template):
        """
        Valid tokens in 'template' are:
          $id : the term's identifier
          $func_id : ID of the term's function
          $input_ids : ID of the term's inputs, comma seperated
          $value : the term's current value
        """
        return Template(template).safe_substitute(
                  { 'id': self.getIdentifier(term),
                    'func_id': self.getIdentifier(term.functionTerm),
                    'func_name': ca_function.name(term.functionTerm),
                    'input_ids': ','.join(map(self.getIdentifier,term.inputs)),
                    'value': str(term),
                    'type': ca_type.name(term.getType()) })


def main():
    import Circa.core.bootstrap

    browser = Browser(builtins.KERNEL)
    while True:
        try:
            cmd = raw_input('> ')
        except KeyboardInterrupt:
            print ""
            return

        if cmd == 'exit':
            return

        browser.doCommand(cmd)

if __name__ == "__main__":
    main()
