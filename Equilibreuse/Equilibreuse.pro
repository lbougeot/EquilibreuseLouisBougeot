QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    codeur.cpp \
    dialogconfigurerexperience.cpp \
    dialogparametrageequilibreuse.cpp \
    experience.cpp \
    jaugesdecontrainte.cpp \
    main.cpp \
    equilibreuse.cpp \
    mcculdaq.cpp \
    moteur.cpp \
    pointdecourbe.cpp \
    rapportmesures.cpp \
    vuegraphique.cpp

HEADERS += \
    codeur.h \
    constante.h \
    dialogconfigurerexperience.h \
    dialogparametrageequilibreuse.h \
    equilibreuse.h \
    experience.h \
    jaugesdecontrainte.h \
    mcculdaq.h \
    moteur.h \
    pointdecourbe.h \
    rapportmesures.h \
    vuegraphique.h

FORMS += \
    dialogconfigurerexperience.ui \
    dialogparametrageequilibreuse.ui \
    equilibreuse.ui

TRANSLATIONS += \
    Equilibreuse_fr_FR.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    images.qrc

LIBS += /usr/local/lib/libuldaq.a
LIBS += -lusb-1.0
