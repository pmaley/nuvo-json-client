QT += widgets

HEADERS     = dialog.h
SOURCES     = dialog.cpp \
              main.cpp

# install
#target.path = $$[QT_INSTALL_EXAMPLES]/widgets/layouts/basiclayouts
target.path = ./

INSTALLS += target

simulator: warning(This example might not fully work on Simulator platform)

OTHER_FILES += \
    img/aom.jpg
