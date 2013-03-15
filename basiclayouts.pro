QT += widgets network script

HEADERS     = dialog.h \
    nuvoapiclient.h \
    bonjourbrowser.h \
    bonjourrecord.h \
    bonjourresolver.h \
    overlay.h
SOURCES     = dialog.cpp \
              main.cpp \
    nuvoapiclient.cpp \
    bonjourbrowser.cpp \
    bonjourresolver.cpp \
    overlay.cpp

# install
#target.path = $$[QT_INSTALL_EXAMPLES]/widgets/layouts/basiclayouts
target.path = ./

INSTALLS += target

simulator: warning(This example might not fully work on Simulator platform)

ICON = img/app_icon.icns

OTHER_FILES += \
    img/aom.jpg
    img/Icon@2x.png
    img/app_icon.icns

RESOURCES += \
    img/resources.qrc
