#!/usr/bin/env python

import os, subprocess, sys, time

os.chdir(os.environ['CIRCA_HOME'])

binary = 'build/bin/plas'
args = [binary] + sys.argv[1:]

def repeatedly_relaunch():
    while True:
        proc = subprocess.Popen(args)

        mtime = os.path.getmtime(binary)

        while True:

            time.sleep(.05)

            if proc.poll() is not None:
                return

            try:
                new_mtime = os.path.getmtime(binary)
                if mtime != new_mtime:
                    break
            except OSError:
                pass

        # kill
        proc.terminate()

try:
    repeatedly_relaunch()
except KeyboardInterrupt:
    pass



