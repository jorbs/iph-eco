#-------------------------------------------------
#
# Project created by QtCreator 2015-01-16T00:31:49
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = iph-eco
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/domain/project.cpp \
    src/application/iph_application.cpp \
    src/dao/project_dao.cpp \
    src/exceptions/database_exception.cpp \
    src/services/project_service.cpp \
    src/ui/main_window.cpp \
    src/ui/new_project_dialog.cpp \
    src/ui/project_properties_dialog.cpp \
    src/utility/database_utility.cpp \
    src/exceptions/mesh_exception.cpp \
    src/domain/unstructured_mesh.cpp \
    src/domain/structured_mesh.cpp \
    src/domain/mesh.cpp \
    src/ui/unstructured_mesh_dialog.cpp \
    src/ui/unstructured_mesh_opengl_widget.cpp \
    src/ui/structured_mesh_opengl_widget.cpp \
    src/domain/mesh_polygon.cpp \
    src/ui/structured_mesh_dialog.cpp \
    src/ui/grid_data_dialog.cpp \
    src/domain/grid_data.cpp \
    src/ui/grid_data_opengl_widget.cpp \
    src/domain/grid_data_configuration.cpp \
    src/exceptions/grid_data_exception.cpp \
    src/ui/grid_information_dialog.cpp \
    src/ui/cell_update_dialog.cpp \
    src/domain/quad.cpp \
    src/domain/cell_info.cpp \
    src/domain/grid_information_type.cpp \
    src/domain/delaunay_triangulation_face.cpp \
    src/ui/unstructured_mesh_vtk_widget.cpp

HEADERS  += \
    include/domain/project.h \
    include/application/iph_application.h \
    include/dao/project_dao.h \
    include/exceptions/database_exception.h \
    include/services/project_service.h \
    include/ui/main_window.h \
    include/ui/new_project_dialog.h \
    include/ui/project_properties_dialog.h \
    include/utility/database_utility.h \
    include/exceptions/mesh_exception.h \
    include/domain/unstructured_mesh.h \
    include/domain/structured_mesh.h \
    include/domain/mesh.h \
    include/ui/unstructured_mesh_dialog.h \
    include/ui/unstructured_mesh_opengl_widget.h \
    include/ui/structured_mesh_opengl_widget.h \
    include/domain/mesh_polygon.h \
    include/ui/structured_mesh_dialog.h \
    include/ui/grid_data_dialog.h \
    include/domain/grid_data.h \
    include/ui/grid_data_opengl_widget.h \
    include/domain/grid_data_configuration.h \
    include/exceptions/grid_data_exception.h \
    include/domain/grid_data_point.h \
    include/ui/grid_information_dialog.h \
    include/domain/grid_data_polygon.h \
    include/domain/quad.h \
    include/domain/grid_information_type.h \
    include/ui/cell_update_dialog.h \
    include/domain/cell_info.h \
    include/domain/delaunay_triangulation_face.h \
    include/utility/cgal_definitions.h \
    include/utility/opengl_util.h \
    include/ui/unstructured_mesh_vtk_widget.h

FORMS    += \
    include/ui/main_window.ui \
    include/ui/new_project_dialog.ui \
    include/ui/project_properties_dialog.ui \
    include/ui/unstructured_mesh_dialog.ui \
    include/ui/structured_mesh_dialog.ui \
    include/ui/grid_data_dialog.ui \
    include/ui/grid_information_dialog.ui \
    include/ui/cell_update_dialog.ui

macx: QMAKE_CXXFLAGS += -Wno-redeclared-class-member -Wno-unused-parameter

macx: INCLUDEPATH += /usr/local/Cellar/boost/1.57.0/include
macx: DEPENDPATH += /usr/local/Cellar/boost/1.57.0/include
macx: LIBS += -L/usr/local/Cellar/boost/1.57.0/lib/ -lboost_system -lboost_thread-mt

macx: INCLUDEPATH += /usr/local/include/vtk-6.2
macx: LIBS += -L/usr/local/lib -lvtkCommonCore-6.2 \
        -lvtkCommonExecutionModel-6.2 \
        -lvtkViewsCore-6.2 \
        #-lvtkViewsQt-6.2 \
        -lvtkRenderingCore-6.2 \
        #-lvtkRenderingQt-6.2 \
        -lvtkGUISupportQt-6.2 \
        -lvtkFiltersSources-6.2 \
#        -lvtkRenderingVolume-6.2 \
        -lvtkCommonDataModel-6.2 \
        -lvtkRenderingOpenGL-6.2 \
        -lvtkRenderingVolumeOpenGL-6.2 \
        -lvtkInteractionStyle-6.2 \
        -lvtkIOLegacy-6.2 \
        -lvtkFiltersCore-6.2
        #-lvtkRenderingFreeType-6.2 \
        #-lvtkRenderingFreeTypeOpenGL-6.2

macx: LIBS += -L/usr/local/Cellar/geographiclib/1.40/lib/ -lGeographic
macx: INCLUDEPATH += /usr/local/Cellar/geographiclib/1.40/include
macx: DEPENDPATH += /usr/local/Cellar/geographiclib/1.40/include

macx: LIBS += -L/usr/local/Cellar/gmp/6.0.0a/lib/ -lgmp
macx: INCLUDEPATH += /usr/local/Cellar/gmp/6.0.0a/include
macx: DEPENDPATH += /usr/local/Cellar/gmp/6.0.0a/include

macx: LIBS += -L/usr/local/Cellar/mpfr/3.1.2-p10/lib/ -lmpfr
macx: INCLUDEPATH += /usr/local/Cellar/mpfr/3.1.2-p10/include
macx: DEPENDPATH += /usr/local/Cellar/mpfr/3.1.2-p10/include

macx: LIBS += -L/usr/local/Cellar/cgal/4.5.2/lib/ -lCGAL
macx: INCLUDEPATH += /usr/local/Cellar/cgal/4.5.2/include
macx: DEPENDPATH += /usr/local/Cellar/cgal/4.5.2/include

unix:!macx: QMAKE_CXXFLAGS += -frounding-math
unix:!macx: LIBS += -L/usr/lib/ -lGeographic -lCGAL -lgmp -lmpfr -lboost_system -lboost_thread
unix:!macx: INCLUDEPATH += /usr/include
unix:!macx: DEPENDPATH += /usr/include

win32: INCLUDEPATH =+ C:\Qt\Qt5.4.1\Tools\mingw491_32\include
win32 {
  standardboost {
    LIBS += -lboost_thread_win32-mt -lboost_system-mt -lgmp -lmpfr -lGeographic -lCGAL
  } else {
    LIBS += -lboost_thread-mgw49-mt-1_58 -lboost_system-mgw49-mt-1_58 -lgmp -lmpfr -lGeographic -lCGAL
  }
}

RESOURCES += icons.qrc