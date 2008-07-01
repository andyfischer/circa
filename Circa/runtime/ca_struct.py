
class CircaStructDefinition(object):
    name = 'struct-definition'

    def __init__(self):
        self.members = []

    def appendMember(self, name, type):
        self.members.append((name,type))

