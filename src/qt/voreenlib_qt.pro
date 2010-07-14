####################################################
# Project file for the Voreen-Qt library
####################################################
TEMPLATE = lib
TARGET = voreen_qt
VERSION = 1.0

CONFIG += qt
QT += opengl

QT += network
CONFIG += static thread
CONFIG -= dll

# Check Qt version:
# Use a regex that matches all invalid version numbers, namely X.*.*
# with X <= 3 and 4.Y.* with Y <= 3.
VERSION_CHECK = $$find(QT_VERSION, "^([123]|4\.[123])\..*$")
!isEmpty(VERSION_CHECK) {
   error("Your Qt version is $$QT_VERSION but at least 4.4.0 is required!")
}

# Include local configuration
!include(../../config.txt) {
  warning("config.txt not found! Using config-default.txt instead.")
  warning("For custom behavior, copy config-default.txt to config.txt and edit!")
  include(../../config-default.txt)
}

# Include common configuration
include(../../commonconf.pri)

# Destination directory for the library
macx: DESTDIR = ../..
unix: DESTDIR = ../..
win32: {
  CONFIG(debug, debug|release) {
    DESTDIR = ../../Debug
  }
  else {
    DESTDIR = ../../Release
  }
}

# Include modules which are selected in local configuration. The entry
# 'foo' in VRN_MODULES must correspond to a subdir 'modules/foo' and a
# file 'foo_qt.pri' there.
for(i, VRN_MODULES) : exists($${VRN_HOME}/src/modules/$${i}/$${i}_qt.pri) {
  include($${VRN_HOME}/src/modules/$${i}/$${i}_qt.pri)
}

# include resource files
RESOURCES = $${VRN_HOME}/resource/qt/vrn_qt.qrc

# please insert new files in alphabetical order!
##############
# Sources
##############
SOURCES += \
    aboutbox.cpp \
    helpbrowser.cpp \
    ioprogressdialog.cpp \
    pyvoreenqt.cpp \
    versionqt.cpp \
    voreenapplicationqt.cpp
SOURCES += \
    widgets/choicelistcombobox.cpp \
    widgets/clickablelabel.cpp \
    widgets/codeedit.cpp \
    widgets/consoleplugin.cpp \
    widgets/customlabel.cpp \
    widgets/eventpropertywidget.cpp \
    widgets/expandableheaderbutton.cpp \
    widgets/glslhighlighter.cpp \
    widgets/inputmappingdialog.cpp \
    widgets/keydetectorwidget.cpp \
    widgets/labelingwidgetqt.cpp \
    widgets/lightwidget.cpp \
    widgets/lineeditresetwidget.cpp \
    widgets/linkingscriptmanager.cpp \
    widgets/pythonhighlighter.cpp \
    widgets/rawvolumewidget.cpp \
    widgets/rendertargetviewer.cpp \
    widgets/shaderplugin.cpp \
    widgets/sliderspinboxwidget.cpp \
    widgets/snapshotplugin.cpp \
    widgets/syntaxhighlighter.cpp \
    widgets/volumecontainerwidget.cpp \
    widgets/volumeloadbutton.cpp \
    widgets/volumeviewhelper.cpp \
    widgets/voreentoolwindow.cpp
SOURCES += \
    widgets/animation/animationeditor.cpp \
    widgets/animation/animationexportwidget.cpp \
    widgets/animation/currentframegraphicsitem.cpp \
    widgets/animation/keyframegraphicsitem.cpp \
    widgets/animation/nodechainwidget.cpp \
    widgets/animation/overviewwidget.cpp \
    widgets/animation/processortimelinewidget.cpp \
    widgets/animation/propertytimelineview.cpp \
    widgets/animation/propertytimelinewidget.cpp \
    widgets/animation/templatepropertytimelinewidget.cpp \
    widgets/animation/timelinewidget.cpp  
SOURCES += \
    widgets/property/boolpropertywidget.cpp \
    widgets/property/buttonpropertywidget.cpp \
    widgets/property/camerapropertywidget.cpp \
    widgets/property/colorpropertywidget.cpp \
    widgets/property/filedialogpropertywidget.cpp \
    widgets/property/floatmat2propertywidget.cpp \
    widgets/property/floatmat3propertywidget.cpp \
    widgets/property/floatmat4propertywidget.cpp \
    widgets/property/floatpropertywidget.cpp \
    widgets/property/floatvec2propertywidget.cpp \
    widgets/property/floatvec3propertywidget.cpp \
    widgets/property/floatvec4propertywidget.cpp \
    widgets/property/intpropertywidget.cpp \
    widgets/property/intvec2propertywidget.cpp \
    widgets/property/intvec3propertywidget.cpp \
    widgets/property/intvec4propertywidget.cpp \
    widgets/property/lightpropertywidget.cpp \
    widgets/property/matrixpropertywidget.cpp \
    widgets/property/optionpropertywidget.cpp \
    widgets/property/processorpropertieswidget.cpp \
    widgets/property/propertyvectorwidget.cpp \
    widgets/property/qpropertywidget.cpp \
    widgets/property/qpropertywidgetfactory.cpp \
    widgets/property/qpropertywidgetwitheditorwindow.cpp \
    widgets/property/shaderpropertywidget.cpp \
    widgets/property/stringpropertywidget.cpp \
    widgets/property/transfuncpropertywidget.cpp \
    widgets/property/vecpropertywidget.cpp \
    widgets/property/volumecollectionpropertywidget.cpp \
    widgets/property/volumehandlepropertywidget.cpp 

SOURCES += \
    widgets/processorlistwidget.cpp \
    widgets/propertylistwidget.cpp
SOURCES += \
    widgets/processor/canvasrendererwidget.cpp \
    widgets/processor/qprocessorwidget.cpp \
    widgets/processor/qprocessorwidgetfactory.cpp
SOURCES += \
    widgets/property/camerawidget.cpp 

SOURCES += \
    widgets/transfunc/colorluminancepicker.cpp \
    widgets/transfunc/colorpicker.cpp \
    widgets/transfunc/doubleslider.cpp \
    widgets/transfunc/histogrampainter.cpp \
    widgets/transfunc/transfunceditor.cpp \
    widgets/transfunc/transfunceditorintensity.cpp \
    widgets/transfunc/transfunceditorintensitygradient.cpp \
    widgets/transfunc/transfunceditorintensitypet.cpp \
    widgets/transfunc/transfunceditorintensityramp.cpp \
    widgets/transfunc/transfuncintensitygradientpainter.cpp \
    widgets/transfunc/transfuncmappingcanvas.cpp \
    widgets/transfunc/transfuncmappingcanvasramp.cpp \
    widgets/transfunc/transfuncplugin.cpp \
    widgets/transfunc/transfunctexturepainter.cpp
    
SOURCES += \
    ../../ext/tgt/qt/qtcanvas.cpp \
    ../../ext/tgt/qt/qttimer.cpp

#SHADER_SOURCES += \

SHADER_SOURCES_RENDERTARGETVIEWER += \
    glsl/rendertargetviewer/color.frag \
    glsl/rendertargetviewer/inversecolor.frag

visual_studio: SOURCES += $$SHADER_SOURCES $$SHADER_SOURCES_RENDERTARGETVIEWER

##############
# Headers
##############
    
HEADERS += \
    ../../include/voreen/qt/aboutbox.h \
    ../../include/voreen/qt/helpbrowser.h \
    ../../include/voreen/qt/ioprogressdialog.h \
    ../../include/voreen/qt/pyvoreenqt.h \
    ../../include/voreen/qt/versionqt.h \
    ../../include/voreen/qt/voreenapplicationqt.h
HEADERS += \
    ../../include/voreen/qt/widgets/animation/animationexportwidget.h \
    ../../include/voreen/qt/widgets/choicelistcombobox.h \
    ../../include/voreen/qt/widgets/clickablelabel.h \
    ../../include/voreen/qt/widgets/codeedit.h \
    ../../include/voreen/qt/widgets/consoleplugin.h \
    ../../include/voreen/qt/widgets/customlabel.h \
    ../../include/voreen/qt/widgets/eventpropertywidget.h \
    ../../include/voreen/qt/widgets/expandableheaderbutton.h \
    ../../include/voreen/qt/widgets/glslhighlighter.h \
    ../../include/voreen/qt/widgets/inputmappingdialog.h \
    ../../include/voreen/qt/widgets/keydetectorwidget.h \
    ../../include/voreen/qt/widgets/labelingwidgetqt.h \
    ../../include/voreen/qt/widgets/lightwidget.h \
    ../../include/voreen/qt/widgets/lineeditresetwidget.h \
    ../../include/voreen/qt/widgets/linkingscriptmanager.h \
    ../../include/voreen/qt/widgets/pythonhighlighter.h \
    ../../include/voreen/qt/widgets/rawvolumewidget.h \
    ../../include/voreen/qt/widgets/rendertargetviewer.h \
    ../../include/voreen/qt/widgets/shaderplugin.h \
    ../../include/voreen/qt/widgets/sliderspinboxwidget.h \
    ../../include/voreen/qt/widgets/snapshotplugin.h \
    ../../include/voreen/qt/widgets/syntaxhighlighter.h \
    ../../include/voreen/qt/widgets/volumecontainerwidget.h \
    ../../include/voreen/qt/widgets/volumeloadbutton.h \
    ../../include/voreen/qt/widgets/volumeviewhelper.h \
    ../../include/voreen/qt/widgets/voreentoolwindow.h
HEADERS +=  \
    ../../include/voreen/qt/widgets/animation/animationeditor.h \
    ../../include/voreen/qt/widgets/animation/currentframegraphicsitem.h \
    ../../include/voreen/qt/widgets/animation/keyframegraphicsitem.h \
    ../../include/voreen/qt/widgets/animation/nodechainwidget.h \
    ../../include/voreen/qt/widgets/animation/overviewwidget.h \
    ../../include/voreen/qt/widgets/animation/processortimelinewidget.h \
    ../../include/voreen/qt/widgets/animation/propertytimelineview.h \
    ../../include/voreen/qt/widgets/animation/propertytimelinewidget.h \
    ../../include/voreen/qt/widgets/animation/templatepropertytimelinewidget.h \
    ../../include/voreen/qt/widgets/animation/timelinewidget.h 
HEADERS += \
    ../../include/voreen/qt/widgets/property/boolpropertywidget.h \
    ../../include/voreen/qt/widgets/property/buttonpropertywidget.h \
    ../../include/voreen/qt/widgets/property/camerapropertywidget.h \
    ../../include/voreen/qt/widgets/property/colorpropertywidget.h \
    ../../include/voreen/qt/widgets/property/filedialogpropertywidget.h \
    ../../include/voreen/qt/widgets/property/floatmat2propertywidget.h \
    ../../include/voreen/qt/widgets/property/floatmat3propertywidget.h \
    ../../include/voreen/qt/widgets/property/floatmat4propertywidget.h \
    ../../include/voreen/qt/widgets/property/floatpropertywidget.h \
    ../../include/voreen/qt/widgets/property/floatvec2propertywidget.h \
    ../../include/voreen/qt/widgets/property/floatvec3propertywidget.h \
    ../../include/voreen/qt/widgets/property/floatvec4propertywidget.h \
    ../../include/voreen/qt/widgets/property/intpropertywidget.h \
    ../../include/voreen/qt/widgets/property/intvec2propertywidget.h \
    ../../include/voreen/qt/widgets/property/intvec3propertywidget.h \
    ../../include/voreen/qt/widgets/property/intvec4propertywidget.h \
    ../../include/voreen/qt/widgets/property/lightpropertywidget.h \
    ../../include/voreen/qt/widgets/property/matrixpropertywidget.h \
    ../../include/voreen/qt/widgets/property/optionpropertywidget.h \
    ../../include/voreen/qt/widgets/property/processorpropertieswidget.h \
    ../../include/voreen/qt/widgets/property/propertyvectorwidget.h \
    ../../include/voreen/qt/widgets/property/qpropertywidget.h \
    ../../include/voreen/qt/widgets/property/qpropertywidgetfactory.h \
    ../../include/voreen/qt/widgets/property/qpropertywidgetwitheditorwindow.h \
    ../../include/voreen/qt/widgets/property/shaderpropertywidget.h \
    ../../include/voreen/qt/widgets/property/stringpropertywidget.h \
    ../../include/voreen/qt/widgets/property/transfuncpropertywidget.h \
    ../../include/voreen/qt/widgets/property/vecpropertywidget.h \
    ../../include/voreen/qt/widgets/property/volumecollectionpropertywidget.h \
    ../../include/voreen/qt/widgets/property/volumehandlepropertywidget.h 

HEADERS += \
    ../../include/voreen/qt/widgets/processorlistwidget.h \
    ../../include/voreen/qt/widgets/propertylistwidget.h
HEADERS += \
    ../../include/voreen/qt/widgets/processor/canvasrendererwidget.h \
    ../../include/voreen/qt/widgets/processor/qprocessorwidget.h \
    ../../include/voreen/qt/widgets/processor/qprocessorwidgetfactory.h
HEADERS += \
    ../../include/voreen/qt/widgets/property/camerawidget.h 

HEADERS += \
    ../../include/voreen/qt/widgets/transfunc/colorluminancepicker.h \
    ../../include/voreen/qt/widgets/transfunc/colorpicker.h \
    ../../include/voreen/qt/widgets/transfunc/doubleslider.h \
    ../../include/voreen/qt/widgets/transfunc/histogrampainter.h \
    ../../include/voreen/qt/widgets/transfunc/transfunceditor.h \
    ../../include/voreen/qt/widgets/transfunc/transfunceditorintensity.h \
    ../../include/voreen/qt/widgets/transfunc/transfunceditorintensitygradient.h \
    ../../include/voreen/qt/widgets/transfunc/transfunceditorintensitypet.h \
    ../../include/voreen/qt/widgets/transfunc/transfunceditorintensityramp.h \
    ../../include/voreen/qt/widgets/transfunc/transfuncintensitygradientpainter.h \
    ../../include/voreen/qt/widgets/transfunc/transfuncmappingcanvas.h \
    ../../include/voreen/qt/widgets/transfunc/transfuncmappingcanvasramp.h \
    ../../include/voreen/qt/widgets/transfunc/transfuncplugin.h \
    ../../include/voreen/qt/widgets/transfunc/transfunctexturepainter.h

HEADERS += \
    ../../ext/tgt/qt/qtcanvas.h \
    ../../ext/tgt/qt/qttimer.h

MSVC_IDE: SOURCES += \
    ../core/glsl/rendertargetdebugwidget/color.frag \
    ../core/glsl/rendertargetdebugwidget/inversecolor.frag

contains(DEFINES, VRN_WITH_DCMTK) {
  HEADERS += ../../include/voreen/qt/dicomdialog.h
  SOURCES += dicomdialog.cpp
}

FORMS += aboutbox.ui


# this must come after all SOURCES, HEADERS and FORMS have been added
contains(DEFINES, VRN_WITH_SVNVERSION) : revtarget.depends = $$SOURCES $$HEADERS $$FORMS

# installation
unix {
  !isEmpty(INSTALL_PREFIX) {
    target.path = $$INSTALLPATH_LIB

    shaders.path = $$INSTALLPATH_SHARE/shaders
    shaders.files += $$SHADER_SOURCES
    shaders_rtv.path = $$INSTALLPATH_SHARE/shaders/rendertargetviewer
    shaders_rtv.files += $$SHADER_SOURCES_RENDERTARGETVIEWER

    INSTALLS += target shaders shaders_rtv
  }
}

### Local Variables:
### mode:conf-unix
### End:
