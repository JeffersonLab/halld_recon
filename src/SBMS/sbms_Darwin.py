
#
# Mac OS X specific settings
#

import os

def InitENV(env):

	# This is needed to allow plugins to have their
	# global variables linked to those in the running
	# executable.
	env.AppendUnique(LINKFLAGS='-flat_namespace')

	# For plugins that don't have everything when they are linked
	env.AppendUnique(SHLINKFLAGS=['-undefined', 'suppress'])

	# gfortran is installed via "brew cask install gfortran" in /usr/local/gfortran
	if os.path.exists('/usr/local/gfortran/lib'):
		env.AppendUnique(LIBPATH='/usr/local/gfortran/lib')
