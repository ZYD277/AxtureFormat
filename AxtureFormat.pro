#-------------------------------------------------
#
# Project created by QtCreator 2020-01-12T19:58:36
#
#-------------------------------------------------

QT       += core gui
QT       += xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AxtureFormat
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    html/htmlparser.cpp \
    util/fileutils.cpp \
    util/rsingleton.cpp \
    util/rutil.cpp \
    html/htmlparsemethod.cpp \
    html/htmlstruct.cpp \
    qui/qtparsemethod.cpp \
    qui/qtstruct.cpp \
    qui/qtoutput.cpp \
    head.cpp \
    css/cssparsemethod.cpp \
    css/cssparser.cpp \
    css/cssstruct.cpp \
    qui/qrc/qrcoutput.cpp \
    qui/qrc/qrcparsemethod.cpp \
    qui/qss/qssoutput.cpp \
    qui/qss/qssparsemethod.cpp

HEADERS  += widget.h \
    html/htmlparser.h \
    util/fileutils.h \
    util/rsingleton.h \
    util/rutil.h \
    html/htmlparsemethod.h \
    html/htmlstruct.h \
    qui/qtparsemethod.h \
    qui/qtstruct.h \
    qui/qtoutput.h \
    head.h \
    css/cssparsemethod.h \
    css/cssparser.h \
    css/cssstruct.h \
    qui/qrc/qrcoutput.h \
    qui/qrc/qrcparsemethod.h \
    qui/qss/qssoutput.h \
    qui/qss/qssparsemethod.h

FORMS    += widget.ui
