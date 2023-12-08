CONFIG +=  qpa largefile precompile_header use_gold_linker enable_new_dtags sse2 sse3 ssse3 sse4_1 sse4_2 avx avx2
QT_BUILD_PARTS += libs tools
QT_SKIP_MODULES +=  qtmultimedia qtdeclarative qtconnectivity qtmacextras qtandroidextras qtsensors qtserialport qtwebengine qtwebsockets qtwebchannel qtwebview qtserialbus
QT_NO_DEFINES =  IMAGEFORMAT_JPEG LIBPROXY OPENVG TABLET TSLIB XINPUT ZLIB
QT_QCONFIG_PATH = 
host_build {
    QT_CPU_FEATURES.x86_64 =  mmx sse sse2
} else {
    QT_CPU_FEATURES.x86_64 =  mmx sse sse2
}
QT_COORD_TYPE = double
QT_CFLAGS_MYSQL   = -I/usr/include/mysql
QT_LFLAGS_MYSQL   = -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lssl -lcrypto -ldl
QT_LFLAGS_ODBC   = -lodbc
QMAKE_LINK = g++
QMAKE_CC = gcc
QMAKE_CXX = g++
styles += mac fusion windows
DEFINES += QT_NO_MTDEV
CONFIG += use_libmysqlclient_r
QT_LIBS_DBUS = -L/lib64 -ldbus-1  
QT_CFLAGS_DBUS = -I/usr/include/dbus-1.0 -I/usr/lib64/dbus-1.0/include  
QT_HOST_CFLAGS_DBUS = -I/usr/include/dbus-1.0 -I/usr/lib64/dbus-1.0/include  
QT_CFLAGS_GLIB = -pthread -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include  
QT_LIBS_GLIB = -pthread -lgthread-2.0 -lglib-2.0  
QT_CFLAGS_QGTKSTYLE = -pthread -I/usr/include/gtk-2.0 -I/usr/lib64/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/pango-1.0 -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng15 -I/usr/include/libdrm -I/usr/include/harfbuzz  
QT_LIBS_QGTKSTYLE = -lgobject-2.0 -lglib-2.0  
QT_CFLAGS_QGTK2 = -pthread -I/usr/include/gtk-2.0 -I/usr/lib64/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/pango-1.0 -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng15 -I/usr/include/libdrm -I/usr/include/harfbuzz  
QT_LIBS_QGTK2 = -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgio-2.0 -lpangoft2-1.0 -lpangocairo-1.0 -lgdk_pixbuf-2.0 -lcairo -lpango-1.0 -lfontconfig -lgobject-2.0 -lglib-2.0 -lfreetype  
QT_CFLAGS_PULSEAUDIO = -D_REENTRANT -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include  
QT_LIBS_PULSEAUDIO = -lpulse-mainloop-glib -lpulse -lglib-2.0  
QMAKE_INCDIR_OPENGL =  "/usr/include/libdrm"
QMAKE_LIBDIR_OPENGL = 
QMAKE_LIBS_OPENGL =  "-lGL"
QMAKE_CFLAGS_OPENGL = 
QMAKE_CFLAGS_FONTCONFIG = -I/usr/include/freetype2  
QMAKE_LIBS_FONTCONFIG = -lfontconfig -lfreetype  
QMAKE_INCDIR_LIBUDEV =  
QMAKE_LIBS_LIBUDEV = -ludev  
DEFINES += QT_NO_TSLIB
DEFINES += QT_NO_LIBINPUT
QMAKE_LIBXI_VERSION_MAJOR = 1
QMAKE_LIBXI_VERSION_MINOR = 7
QMAKE_LIBXI_VERSION_PATCH = 4
QMAKE_X11_PREFIX = /usr
QMAKE_XKB_CONFIG_ROOT = /usr/share/X11/xkb
QMAKE_INCDIR_EGL = /usr/include/libdrm  
QMAKE_LIBS_EGL = -lEGL  
QMAKE_CFLAGS_EGL =  
QMAKE_CFLAGS_XCB =  
QMAKE_LIBS_XCB = -lxcb  
sql-drivers = 
sql-plugins =  mysql odbc psql sqlite