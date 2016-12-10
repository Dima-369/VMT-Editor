QT += core gui opengl widgets

TEMPLATE = app


win32-msvc*:{
    QMAKE_CXXFLAGS_RELEASE -= "-O2"
    QMAKE_CXXFLAGS_RELEASE += "-Od"

    LIBS += \
        Comdlg32.lib \
        Advapi32.lib \
        Shell32.lib
}

win32-g++: {
    LIBS += -lgdi32 -lcomdlg32 -lopengl32 -lglu32
}

win32:CONFIG(release, debug|release): DEFINES += _RELEASE=1
else:win32:CONFIG(debug, debug|release): DEFINES += _DEBUG=1

!win32-msvc*:{
    QMAKE_CXXFLAGS_RELEASE += "-O0"
}

linux {
    LIBS += -lGL -lGLU
}


INCLUDEPATH += src/

SOURCES += \
    src/main.cpp\
    src/mainwindow.cpp \
    src/vmtparser.cpp \
    src/glwidget.cpp \
    src/texturethread.cpp \
    src/utilities.cpp \
    src/highlighter.cpp \
    src/subgrouptextedit.cpp \
    src/aboutdialog.cpp \
    src/iconlabel.cpp \
    src/dialogwithouthelpbutton.cpp \
    src/optionsdialog.cpp \
    src/editshaderdialog.cpp \
    src/colordialog.cpp \
    src/newshadergroupdialog.cpp \
    src/faderwidget.cpp \
    src/texturepreviewdockwidget.cpp \
    src/shader.cpp \
    src/conversionthread.cpp \
    src/conversiondialog.cpp \
    src/messagebox.cpp \
    src/batchdialog.cpp \
    src/editgamesdialog.cpp \
    src/editgamedialog.cpp \
    src/tintslider.cpp \
    src/glwidget_diffuse1.cpp \
    src/glwidget_diffuse2.cpp \
    src/glwidget_spec.cpp \
    src/doublesliderspinbox.cpp \
    src/exponentialspinbox.cpp \
    src/user-interface/detail-texture.cpp \
    src/user-interface/view-helper.cpp \
    src/user-interface/shaders.cpp \
    src/user-interface/normal-blend.cpp \
    src/vmt/vmt-helper.cpp \
    src/user-interface/phong.cpp \
    src/logging/logging.cpp \
    src/opengl/helpers.cpp \
    src/utilities/window.cpp \
    src/user-interface/shading-reflection.cpp

HEADERS += \
    src/mainwindow.h \
    src/vmtparser.h \
    src/utilities.h \
    src/glwidget.h \
    src/texturethread.h \
    src/highlighter.h \
    src/subgrouptextedit.h \
    src/aboutdialog.h \
    src/iconlabel.h \
    src/dialogwithouthelpbutton.h \
    src/optionsdialog.h \
    src/editshaderdialog.h \
    src/colordialog.h \
    src/newshadergroupdialog.h \
    src/faderwidget.h \
    src/texturepreviewdockwidget.h \
    src/shader.h \
    src/conversionthread.h \
    src/conversiondialog.h \
    src/messagebox.h \
    src/batchdialog.h \
    src/editgamesdialog.h \
    src/editgamedialog.h \
    src/tintslider.h \
    src/glwidget_diffuse1.h \
    src/glwidget_diffuse2.h \
    src/glwidget_spec.h \
    src/doublesliderspinbox.h \
    src/exponentialspinbox.h \
    src/user-interface/constants.h \
    src/user-interface/detail-texture.h \
    src/user-interface/view-helper.h \
    src/user-interface/normal-blend.h \
    src/user-interface/shaders.h \
    src/vmt/vmt-helper.h \
    src/user-interface/errors.h \
    src/logging/logging.h \
    src/user-interface/phong.h \
    src/utilities/strings.h \
    src/opengl/helpers.h \
    src/utilities/window.h \
    src/user-interface/shading-reflection.h

FORMS += \
    ui/mainwindow.ui \
    ui/aboutdialog.ui \
    ui/steamnamedialog.ui \
    ui/optionsdialog.ui \
    ui/editshaderdialog.ui \
    ui/colordialog.ui \
    ui/newshadergroupdialog.ui \
    ui/conversiondialog.ui \
    ui/batchdialog.ui \
    ui/editgamedialog.ui \
    ui/editgamesdialog.ui

RESOURCES += resources.qrc

RC_FILE = VMT.rc
