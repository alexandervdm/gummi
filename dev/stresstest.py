# Gummi: stresstest.py
# Development tool to test stability and memory leakage by simulating a user. 

import os
import tempfile
import virtkey
import sys
import random
import time
import thread

# TODO: write command line argument to enable/disable gummi debug mode
# TODO: Options for stress test with limited time, typing speed
# TODO: Start with question whether to proceed, else exit program

# aliases for to be used X.org keysyms, see complete list at:
# http://wiki.linuxquestions.org/wiki/List_of_Keysyms_Recognised_by_Xmodmap
ENTER = 0xFF0D
DOWN = 0xFF54
UP = 0xFF52

# set up keyboard simulator
v = virtkey.virtkey()

# create a new document with default text
_, filenm = tempfile.mkstemp()
tmp = open(filenm, "w")
tmp.write("""\\documentclass{article}
\\begin{document}
\n!\n
\\end{document}""")
tmp.close()

# start a gummi process in a different thread
thread.start_new_thread(lambda: os.system('gummi %s' % filenm) ,())
time.sleep(5)

# set the cursor in the right position:
v.press_keysym(DOWN)
v.release_keysym(DOWN)
v.press_keysym(DOWN)
v.release_keysym(DOWN)


# start the stress test
while True:

	for i in range(1,random.randint(2,20)):
		character = random.randint(97,122)
		v.press_unicode(character)
		v.release_unicode(character)
		time.sleep(0.1)
	if random.randint(1,4) == 1:
		v.press_unicode(ord('\\'))
		v.release_unicode(ord('\\'))
		v.press_unicode(ord('\\'))
		v.release_unicode(ord('\\'))
	v.press_keysym(ENTER)
	v.release_keysym(ENTER)
	time.sleep(random.randint(1,10)/10)








