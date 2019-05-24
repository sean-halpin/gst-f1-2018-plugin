# Note: plugindir is set in configure

##############################################################################
# TODO: change libformulaoneparse.la to something else, e.g. libmysomething.la     #
##############################################################################
plugin_LTLIBRARIES = libformulaoneparse.la

##############################################################################
# TODO: for the next set of variables, name the prefix if you named the .la, #
#  e.g. libmysomething.la => libmysomething_la_SOURCES                       #
#                            libmysomething_la_CFLAGS                        #
#                            libmysomething_la_LIBADD                        #
#                            libmysomething_la_LDFLAGS                       #
##############################################################################

## Plugin 1

# sources used to compile this plug-in
libformulaoneparse_la_SOURCES = gstformulaoneparse.c gstformulaoneparse.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libformulaoneparse_la_CFLAGS = $(GST_CFLAGS)
libformulaoneparse_la_LIBADD = $(GST_LIBS)
libformulaoneparse_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libformulaoneparse_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gstplugin.h
