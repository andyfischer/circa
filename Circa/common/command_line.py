#!/usr/bin/python

import os,sys,pdb
from string import Template
import Circa
from Circa import parser
from Circa.common import (debug, errors, codeunit_utils)
from Circa.core import (builtins, ca_codeunit, ca_type, ca_function)
from Circa.parser import ast as ast_module

class CommandLine(object):
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

        elif command == 'details' or command == 'd':
            term = self.getTermFromIdentifier(commandArgs)
            if term is None:
                print "Couldn't find '%s'" % commandArgs
                return

            print self.describeTerm(term, "$id: $func_id($input_ids) = $value")

        elif command == 'eval' or command == 'e':
            ast = parser.parseStatement(commandArgs)
            print ast
            result = ast.createTerms(ast_module.CompilationContext(self.codeUnit))
            print str(result)
            result.execute()

        elif command == 'update' or command == 'up':
            term = self.getTermFromIdentifier(commandArgs)
            term.update()
            print str(term)

        elif command == 'exec' or command == 'ex':
            term = self.getTermFromIdentifier(commandArgs)
            term.execute()
            print str(term)

        elif command == 'run':
            self.codeUnit.execute()

        elif command == 'func' or command == 'f':
            term = self.getTermFromIdentifier(commandArgs)
            print self.describeTerm(term.functionTerm)

        elif command == 'inputs' or command == 'i':
            term = self.getTermFromIdentifier(commandArgs)
            print map(lambda term: self.describeTerm(term, "$id"), term.inputs)

        elif command == 'users':
            term = self.getTermFromIdentifier(commandArgs)
            print map(lambda term: self.describeTerm(term, "$id"), term.users)

        elif command == 'state' or command == 'st':
            term = self.getTermFromIdentifier(commandArgs)
            print term.state

        elif command == 'trace' or command == 'tr':
            term = self.getTermFromIdentifier(commandArgs)
            term.execute_trace()

        elif command == 'render' or command == "ren":
            print (codeunit_utils.codeToSource(self.codeUnit))

        elif command == 'switch' or command == 'sw':
            module_name = commandArgs.strip()
            if module_name not in Circa.LOADED_MODULES:
                print "Couldn't find module: " + module_name
                return

            self.codeUnit = Circa.LOADED_MODULES[module_name]
            print "Switched to: " + module_name

        elif command == 'create':
            module_name = commandArgs.strip()
            newModule = ca_codeunit.CodeUnit()
            Circa.LOADED_MODULES[module_name] = newModule
            self.codeUnit = newModule

        elif command == 'dir':
            for (name, module) in Circa.LOADED_MODULES.items():
                prefix = "* " if module is self.codeUnit else "  "
                print prefix + name

        elif command == "":
            return

        else:
            # Interpret as a Circa expression
            try:
                ast = parser.parseStatement(inputStr)
                result = ast.createTerms(ast_module.CompilationContext(self.codeUnit))
                if result is None:
                    print "(void)"
                else:
                    print str(result)
                    result.execute()
            except errors.CircaError, e:
                print e
            except Exception,e:
                raise

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

def removeFileSuffix(filename):
    if filename.endswith('.ca'):
        return filename[:-3]


def main():
    Circa.initialize()

    targetCodeUnit = None

    # Read command-line args
    if len(sys.argv) < 2:
        print "No file specified, examining main"
        targetCodeUnit = Circa.LOADED_MODULES['main']
    else:
        filename = findUsersFilename(sys.argv[1])
        if filename is None:
            print "File not found: " + sys.argv[1]
            return

        print "Reading file " + filename + "..."

        (errors, codeUnit) = parser.parseFile(filename)

        if errors:
            print len(errors), "parsing errors occured"
            for error in errors:
                print str(error)
            return

        Circa.LOADED_MODULES[removeFileSuffix(filename)] = codeUnit

        targetCodeUnit = codeUnit
        #targetCodeUnit.execute()
        #targetCodeUnit.updateAll()

    command_line = CommandLine(targetCodeUnit)
    command_line.doInputLoop()

if __name__ == "__main__":
    main()
