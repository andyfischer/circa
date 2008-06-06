#!/usr/bin/python

import os,sys,pdb
from string import Template
from Circa.core import (builtins, ca_type, ca_function)
from Circa import (debug, parser)


class Browser(object):
    def __init__(self, codeUnit=None):
        self.codeUnit = codeUnit

    def doInputLoop(self):
        while True:
            try:
                userInput = raw_input('> ')
            except KeyboardInterrupt:
                print ""
                return

            if userInput == 'exit':
                raise Exception()
                return

            self.doCommand(userInput)
        
    def doCommand(self, inputStr):
        (command, commandArgs) = parseCommand(inputStr)
        if command == 'list' or command == 'l':
            options = commandArgs.split(' ')
            template = "$id: $func_name($input_ids)"

            if '-v' in options:
                template += " = $value"
            for term in self.codeUnit.allTerms:
                print self.describeTerm(term, template)

        elif command == 'details':
            term = self.getTermFromIdentifier(commandArgs)
            if term is None:
                print "Couldn't find '%s'" % commandArgs

            print self.describeTerm(term, "$id: $func_name($input_ids) = $value")

        elif command == 'create' or command == 'c':
            ast = parser.parseExpression(commandArgs)
            print ast
            result = ast.eval(self.codeUnit)
            print str(result)
            result.execute()

        elif command == 'users':
            term = self.getTermFromIdentifier(commandArgs)
            print map(lambda term: self.describeTerm(term, "$id"), term.users)

        elif command == 'state':
            term = self.getTermFromIdentifier(commandArgs)
            print term.state

        else:
            # Interpret as a Circa expression
            ast = parser.parseExpression(inputStr)
            print ast
            result = ast.eval(self.codeUnit)
            print str(result)
            result.execute()

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

def parseCommand(string):
    """
    Parse a string into a command, followed by a space, followed by
    whatever else. Returns tuple of (command, whatever_else).
    """

    first_space = string.find(' ')
    if first_space == -1:
        return (string, "")
    else:
        return (string[:first_space], string[first_space+1:])

def findUsersFilename(filename):
    """
    Take as input a filename specified by the user, and returns the
    filename that they probably meant. Specifically, if the exact
    file is not found, we will try appending '.ca' (if it is missing).
    """
    if filename.endswith('.ca'):
        possibilities = [filename]
    else:
        possibilities = [filename, filename + '.ca']

    for possibility in possibilities:
        if os.path.exists(possibility):
            return possibility

    return None
    

def main():
    import Circa.core.bootstrap

    targetCodeUnit = None

    # Read command-line args
    if len(sys.argv) < 2:
        print "No file specified, examining Kernel"
        targetCodeUnit = builtins.KERNEL
    else:
        filename = findUsersFilename(sys.argv[1])
        if filename is None:
            print "File not found: " + filename
            return

        print "Reading file " + filename + "..."

        (errors, codeUnit) = parser.parseFile(filename)

        if errors:
            print len(errors), "parsing errors occured"
            for error in errors:
                print str(error)
            return

        targetCodeUnit = codeUnit

    browser = Browser(targetCodeUnit)

    browser.doInputLoop()


if __name__ == "__main__":
    main()
