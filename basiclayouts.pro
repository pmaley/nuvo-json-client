QT += widgets network script

HEADERS     = dialog.h \
    nuvoapiclient.h \
    bonjourbrowser.h \
    bonjourrecord.h \
    bonjourresolver.h
SOURCES     = dialog.cpp \
              main.cpp \
    nuvoapiclient.cpp \
    bonjourbrowser.cpp \
    bonjourresolver.cpp

# install
#target.path = $$[QT_INSTALL_EXAMPLES]/widgets/layouts/basiclayouts
target.path = ./

INSTALLS += target

simulator: warning(This example might not fully work on Simulator platform)

OTHER_FILES += \
    img/aom.jpg

RESOURCES += \
    img/resources.qrc
