# setup.py
import sys
import glob
from distutils.core import setup
import py2exe

sys.path.insert(0, sys.path[0] + '\..')

_data_files = [
	 ('gui', glob.glob('gui/*')),
	 ('doc', glob.glob('docs/*'))
	]

setup(
	name = "Gummi",
	description = "Gummi - LaTex Editor",
	version = "svn",
	windows = [
		{
			"script": "gummi.py",
		# In order to show icon in Vista/7 the first icon resources must
		# be (1, "256x256_ico")
			"icon_resources": [(1, "gummi_256x256.ico"),
					(0, "gummi_64x64.ico")]
		}
	],
	data_files=_data_files
)
