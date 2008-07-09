
import pdb
from Circa.core import builtins

class CircaVirtualMachine(object):
    def __init__(self):
        pass

    def run(self, codeUnit):
        self.run_branch(codeUnit.mainBranch)
        
    def run_branch(self, branch):
        for term in branch:
            for input in term.inputs:
                input.requestUpdate()
            term.functionTerm.requestUpdate()

            term.execute()

            # Handle terms with branches
            if term.functionTerm is builtins.IF_STATEMENT:
                if term.getInput(0).value():
                    self.run_branch(term.state.branches[0])
                else:
                    if len(term.state.branches) > 1:
                        self.run_branch(term.state.branches[1])
            elif term.functionTerm is builtins.WHILE_STATEMENT:
                while True:
                    term.getInput(0).requestUpdate()
                    
                    if not term.getInput(0).value():
                        break

                    self.run_branch(term.state.branch)
