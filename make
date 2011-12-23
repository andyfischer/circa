
# A simple make script, good for one-time building of a release-mode tool.
#
# For repeated builds, you can omit the "prebuild.py" step.
#
# To build a debug variant, read docs/building.md

python tools/prebuild.py
scons
