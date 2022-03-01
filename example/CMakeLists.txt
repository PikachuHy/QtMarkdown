find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Quick QuickControls2 Qml Widgets Concurrent REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Quick QuickControls2 Qml Widgets Concurrent REQUIRED)
if (QT_VERSION_MAJOR EQUAL 6)
    qt_add_executable(QtMarkdownParserExample
            MANUAL_FINALIZATION
            QtMarkdownParserExample.cpp
            ../assets/md.qrc
            )
    qt_add_executable(QtWidgetMarkdownEditorExample
            MANUAL_FINALIZATION
            QtWidgetMarkdownEditorExample.cpp
            )
    #    qt_add_executable(QtQuickMarkdownEditorExample
    #            MANUAL_FINALIZATION
    #            Settings.cpp Settings.h
    #            Controller.cpp Controller.h
    #            RecentFile.cpp RecentFile.h
    #            QtQuickMarkdownEditorExample.cpp
    #            qml.qrc
    #            )
    #    qt_add_executable(ExportToDocxExample MANUAL_FINALIZATION
    #            ExportToDocx.cpp
    #            )

else ()
    add_executable(QtMarkdownParserExample QtMarkdownParserExample.cpp ../assets/md.qrc)
    add_executable(QtWidgetMarkdownEditorExample QtWidgetMarkdownEditorExample.cpp)
    #    add_executable(QtQuickMarkdownEditorMain
    #            QtQuickMarkdownEditorExample.cpp
    #            ../assets/md.qrc qml.qrc
    #            )
endif ()
#add_subdirectory(quazip)
target_link_libraries(QtMarkdownParserExample PRIVATE QtMarkdownParser)
set_target_properties(QtMarkdownParserExample PROPERTIES AUTORCC ON)
set_target_properties(QtWidgetMarkdownEditorExample PROPERTIES AUTORCC ON)
target_link_libraries(QtWidgetMarkdownEditorExample PRIVATE QtWidgetMarkdownEditor)
#set_target_properties(QtQuickMarkdownEditorExample PROPERTIES AUTORCC ON)
#set_target_properties(QtQuickMarkdownEditorExample PROPERTIES AUTOMOC ON)
#target_link_libraries(QtQuickMarkdownEditorExample PRIVATE QtQuickMarkdownEditor Qt${QT_VERSION_MAJOR}::QuickControls2)
#target_link_libraries(ExportToDocxExample PRIVATE QtMarkdownRender
#        QuaZip
#        )
if (QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(QtMarkdownParserExample)
    qt_finalize_executable(QtWidgetMarkdownEditorExample)
    #    qt_finalize_executable(QtQuickMarkdownEditorExample)
    #    qt_finalize_executable(ExportToDocxExample)
endif ()
