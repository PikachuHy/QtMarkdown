import QtQuick 2.15
import QtQuick.Window 2.15
import QtMarkdown 1.0
import QtQuick.Controls 2.15
import Qt.labs.platform
import Controller

Window {
    id: root
    width: 1100
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
            Menu {
                     id: recentFilesSubMenu
                     title: qsTr("Recent Files")
                     enabled: recentFilesInstantiator.count > 0

                     Instantiator {
                         id: recentFilesInstantiator
                         model: controller.recentOpenFiles()
                         delegate: MenuItem {
                             text: modelData.path + '  "' + modelData.title + '"'
                             onTriggered: {
                                 md.source = "file://" + modelData.path
                                 root.title = md.title
                             }
                         }
                         // 这两句是必须的，不然菜单就没有内容
                         onObjectAdded: (index, object) => recentFilesSubMenu.insertItem(index, object)
                         onObjectRemoved: (index, object) => recentFilesSubMenu.removeItem(object)
                         function menuItemText(s) {
                             var arr = s.split(":")
                             console.log(arr)
                             if (arr.length === 0) return ""
                             if (arr.length === 1) return arr[0]
                             return arr[0] + '  "' + arr[1] + '"'
                         }
                     }

                     MenuSeparator {}

                     MenuItem {
                         text: qsTr("Clear Recent Files")
                         onTriggered: {
                             controller.clearRecentFiles()
                             recentFilesInstantiator.model = controller.recentOpenFiles()
                         }
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
                        var prefix = "file://"
                        console.log(s)
                        var path = s.substring(prefix.length)
                        controller.addRecentOpenFile(path, md.title)
                        recentFilesInstantiator.model = controller.recentOpenFiles()
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
                        if (md.title.length === 0) {
                            var title = s.substring(s.lastIndexOf('/') + 1)
                            if (!title.endsWith('.md')) {
                                title += ".md"
                            }
                            root.title = title
                        } else {
                            root.title = md.title
                        }

                    }
    }

    Flickable {
        id: sv
        width: parent.width - 300
        height: parent.height
        contentWidth: parent.width - 300
        // contentHeight: md.height < parent.height ? parent.height : md.height
        QtQuickMarkdownEditor {
            id: md
            // 必须focus才能接收键盘输入
            focus: true
            width: parent.width
            implicitHeight: parent.height
            onDocSave: (isNew) => {
                           if (isNew) {
                               saveFileDialog.open()
                           } else {
                               root.title = md.title
                           }

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
            onImageClicked: function (path) {
                console.log('click image:', path)
                if (path.endsWith(".gif")) {
                    previewGif.source = 'file://' + path
                    previewGifPopup.visible = true
                } else {
                    previewImage.source = 'file://' + path
                    previewImagePopup.visible = true
                }

            }
            onContentChanged: {
                root.title = md.title + "*"
            }

            Component.onCompleted: {
                root.title = md.title
            }
        }
        Component.onCompleted: {
            md.source = ":/test.md"
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
    Popup {
        id: previewImagePopup
        visible: false
        anchors.centerIn: parent
        ScrollView {
            anchors.fill: parent

            Image {
                id: previewImage
            }
        }
    }
    Popup {
        id: previewGifPopup
        visible: false
        anchors.centerIn: parent
        ScrollView {
            anchors.fill: parent

            AnimatedImage {
                id: previewGif
            }
        }
    }
    Controller {
        id: controller
    }

    Component.onCompleted: function () {
        console.log(md.width, md.height)
        console.log(sv.width, sv.height)
    }
}
