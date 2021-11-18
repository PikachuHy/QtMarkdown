import QtQuick 2.15
import QtQuick.Window 2.15
import QtMarkdown 1.0
import QtQuick.Controls 2.15
import Qt.labs.platform

Window {
    id: root
    width: 1000
    height: 600
    visible: true
    title: qsTr("Markdown Editor Qt Quick Demo")
    MenuBar {
        id: menuBar
        Menu {
            id: fileMenu
            title: qsTr("File")
            MenuItem {
                text: qsTr("New")
                shortcut: StandardKey.New
                onTriggered: {
                    md.newDoc()
                    root.title = "New Markdown Document"
                }
            }
            MenuItem {
                text: qsTr("Open...")
                onTriggered: {
                    fileDialog.open()
                }
            }
        }
    }
    FileDialog {
        id: fileDialog
        nameFilters: ["Markdown files (*.md *.markdown)"]
        onAccepted: () => {
                        var fileUrl = fileDialog.file
                        console.log(fileUrl)
                        md.source = fileUrl
                        var s = fileUrl.toString()
                        root.title = s.substring(s.lastIndexOf('/') + 1)
                    }
    }
    FileDialog {
        id: saveFileDialog
        fileMode: FileDialog.SaveFile
        onAccepted: () => {
                        var fileUrl = saveFileDialog.file
                        console.log('save document to', fileUrl)
                        md.saveToFile(fileUrl)
                        var s = fileUrl.toString()
                        var title = s.substring(s.lastIndexOf('/') + 1)
                        if (!title.endsWith('.md')) {
                            title += ".md"
                        }
                        root.title = title
                    }
    }

    Flickable {
        id: sv
        width: parent.width - 200
        height: parent.height
        contentWidth: parent.width - 200
        // contentHeight: md.height < parent.height ? parent.height : md.height
        QtQuickMarkdownEditor {
            id: md
            // 必须focus才能接收键盘输入
            focus: true
            width: parent.width
            implicitHeight: parent.height
            source: ":/test.md"
            onDocSave: {
                saveFileDialog.open()
            }
            onCursorCoordChanged: (coord) => {
                                      cursorText.text = coord
                                  }
            onImplicitHeightChanged: {
                if (md.implicitHeight === 0) return
                if (sv.contentHeight === md.implicitHeight) return
                sv.contentHeight = md.implicitHeight < parent.height ? parent.height : height
            }
            onSourceChanged: {
                console.log('source change:',source)
            }
            Component.onCompleted: {
                root.title = source.substring(source.lastIndexOf('/') + 1)
            }
        }
    }
    Column {
        x: sv.width
        Text {
            id: cursorText
            text: qsTr("text")
        }
        Text {
            text: md.width
        }
        Text {
            text: md.height
        }
    }

    Component.onCompleted: function () {
        console.log(md.width, md.height)
        console.log(sv.width, sv.height)
    }
}
