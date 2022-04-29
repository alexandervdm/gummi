project(
    'gummi',
    'c',
    version: '0.8.3',
    license: 'MIT',
    meson_version: '>=0.50.0'
)

i18n = import('i18n')


#dependency('setconf')

prefix = get_option('prefix')
bindir = get_option('bindir')
datadir = get_option('datadir')
libdir = get_option('libdir')
localesdir = get_option('localedir')
appdir = join_paths(datadir, 'applications')
policydir = join_paths(datadir, 'polkit-1/actions')
icondir = join_paths(datadir, 'icons/hicolor/')
schemadir = join_paths(datadir, 'glib-2.0/schemas')
mandir = join_paths(datadir, 'man/man1/')
gummidir = join_paths(datadir, meson.project_name())

project_dependencies = [
    dependency('gtk+-3.0'),
    dependency('glib-2.0'),
    dependency('gtksourceview-3.0'),
    dependency('gtkspell3-3.0'),
    dependency('poppler-glib'),
    dependency('synctex'),
]

build_args = [
  '-Wl,--export-dynamic',
  '-DGUMMI_DATA=' + '"' + join_paths(prefix, gummidir) + '"',
  '-DGUMMI_LIBS=' + '"' + join_paths(prefix, libdir, meson.project_name()) + '"',
  '-DGUMMI_LOCALES=' + '"' + join_paths(prefix, localesdir) + '"',
]

add_project_arguments([
        '-DGETTEXT_PACKAGE=\"@0@\"'.format(meson.project_name())
    ],
    language: 'c',
)

i18n = import('i18n')

conf_data = configuration_data()
conf_data.set('PACKAGE_NAME', 'Gummi')
conf_data.set('PACKAGE', meson.project_name())
conf_data.set_quoted('GETTEXT_PACKAGE', meson.project_name())
conf_data.set_quoted('LOCALEDIR', join_paths(get_option('prefix'), get_option('localedir')))

install_data(['COPYING',], install_dir: join_paths(datadir,'licences/gummi/'))

subdir('src')
subdir('data')
subdir('lib')
subdir('po')

meson.add_install_script('meson-post-install.py')

