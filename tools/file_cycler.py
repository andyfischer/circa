
import time

def write(contents):
    f = open('y.ca', 'w')
    f.write(contents)
    f.close()

while True:
    write("""type T { int i }

state List l

l.append([1] -> T)
    """)

    time.sleep(1)

    write("""type T { int i }

state List r

r.append([1] -> T)
""")

    time.sleep(1)
