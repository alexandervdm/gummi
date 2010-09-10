# Internationalization support

import gettext
import locale

locale.setlocale(locale.LC_ALL, "")

gettext.bindtextdomain('gummi')
gettext.textdomain('gummi')
gettext.install('gummi')
