# set the include path for X, qt and KDE
INCLUDES = $(all_includes) 

# these are the headers for your project
noinst_HEADERS = k2sendconsole.h k2send.h k2sendplayer.h  k2sendplaylistitem.h  k2sendwidgetbase.ui.h k2sendwidget.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/k2send.pot

KDE_ICON = k2send

#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = k2send

# the application source, library search path, and link libraries
k2send_SOURCES = main.cpp k2send.cpp k2sendwidget.cpp k2sendwidgetbase.ui k2sendplayer.cpp k2sendplaylistitem.cpp k2sendplaylistitem.h k2sendconsole.cpp k2sendconsole.h k2sendplaylist.cpp k2sendplaylist.h pref.cpp 
k2send_LDFLAGS = $(KDE_RPATH) $(all_libraries)
k2send_LDADD = $(LIB_KDEUI) $(LIB_KFILE) $(LIB_KDEPRINT)  

# this is where the desktop file will go 
shelldesktopdir   = $(kde_appsdir)/Utilities
shelldesktop_DATA = k2send.desktop

# this is where the shell's XML-GUI resource file goes
shellrcdir   = $(kde_datadir)/k2send
shellrc_DATA = k2sendui.rc

