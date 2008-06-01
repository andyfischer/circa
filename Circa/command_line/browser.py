
from string import Template
from Circa.core import (builtins, ca_type, ca_function)
from Circa import debug


class Browser(object):
    def __init__(self, codeUnit=None):
        self.codeUnit = codeUnit
    def doCommand(self, cmd, args):
        if cmd == 'list':
            for term in self.codeUnit.allTerms:
                print self.describeTerm(term, "$id: $func_name($input_ids)")

        elif cmd == 'details':
            term = self.getTermFromIdentifier(args[0])
            if term is None:
                print "Couldn't find '%s'" % args[0]

            print self.describeTerm(term, "$id: $func_name($input_ids) = $value")

        else:
            print "Unrecognized command: " + cmd
    def getIdentifier(self, term):
        return self.codeUnit.getIdentifier(term)

    def getTermFromIdentifier(self, id):
        debug._assert(isinstance(id, str))

        # Handle ids of the form '#xxx'
        if id[0] == '#':
            index = int(id[1:])

            for term in self.codeUnit.allTerms:
                if term.globalID == index:
                    return term
            return None

        if id in self.codeUnit.mainNamespace:
            return self.codeUnit.mainNamespace[id]

        return None

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

        splitCommand = cmd.split(' ')
        browser.doCommand(splitCommand[0], splitCommand[1:])

if __name__ == "__main__":
    main()
