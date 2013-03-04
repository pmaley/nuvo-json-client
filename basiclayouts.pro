QT += widgets network script

HEADERS     = dialog.h \
    nuvoactionitem.h \
    nuvoapiclient.h \
    nuvocontaineritem.h \
    treemodel.h \
    treeitem.h
SOURCES     = dialog.cpp \
              main.cpp \
    nuvoactionitem.cpp \
    nuvoapiclient.cpp \
    nuvocontaineritem.cpp \
    treemodel.cpp \
    treeitem.cpp

# install
#target.path = $$[QT_INSTALL_EXAMPLES]/widgets/layouts/basiclayouts
target.path = ./

INSTALLS += target

simulator: warning(This example might not fully work on Simulator platform)

OTHER_FILES += \
    img/aom.jpg \
    default.txt

RESOURCES += \
    img/resources.qrc
