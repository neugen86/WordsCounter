import QtQuick
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Controls

import WordsCounter 1.0

Window {
    width: 500
    height: 610
    visible: true
    minimumWidth: 500
    minimumHeight: 400
    title: qsTr("WordsCounter")

    Controller {
        id: controller
    }

    FileDialog {
        id: dialog

        title: "Choose file"
        options: FileDialog.ReadOnly
        fileMode: FileDialog.OpenFile

        onAccepted: {
            controller.file = selectedFile
        }
    }

    ColumnLayout {
        anchors {
            fill: parent
            margins: 4
        }

        RowLayout {
            Layout.fillWidth: true

            TextField {
                enabled: false
                text: controller.file
            }

            Button {
                text: dialog.title

                onClicked: {
                    dialog.open()
                }
            }

            Text {
                Layout.fillWidth: true

                text: controller.error
                elide: Text.ElideMiddle

                color: "red"
            }
        }

        RowLayout {
            Button {
                implicitWidth: 100

                enabled: controller.file.length > 0

                text: {
                    switch (controller.state) {
                    case Controller.Running:
                        return "Pause"
                    case Controller.Paused:
                        return "Continue"
                    default:
                        return "Start"
                    }
                }

                onClicked: {
                    controller.startPause()
                }
            }

            Button {
                implicitWidth: 100

                text: "Cancel"
                enabled: controller.state !== Controller.Stopped

                onClicked: {
                    controller.cancel()
                }
            }

            Item {
                implicitWidth: 30
            }

            ComboBox {
                model: ListModel {
                    ListElement {
                        text: "Ascending"
                        value: Qt.AscendingOrder
                    }
                    ListElement {
                        text: "Descending"
                        value: Qt.DescendingOrder
                    }
                }

                textRole: "text"
                valueRole: "value"

                currentIndex: {
                    controller.model.viewOrder === Qt.AscendingOrder ? 0 : 1
                }

                onCurrentValueChanged: {
                    controller.model.viewOrder = currentValue
                }
            }

            CheckBox {
                text: "Flipped"
                checked: list.flipped

                onCheckedChanged: {
                    list.flipped = checked
                }
            }
        }

        RowLayout {
            Slider {
                id: slider

                implicitWidth: 400

                to: 100
                stepSize: 1

                value: controller.model.maxSize
                enabled: to > 0

                onMoved: {
                    controller.model.maxSize = value
                }

                Component.onCompleted: {
                    from = controller.model.maxSize
                }
            }

            Text {
                text: `${controller.model.maxSize} / ${slider.to}`
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            border {
                width: 1
                color: "gray"
            }

            ListView {
                id: list

                property bool flipped: false

                anchors.centerIn: parent

                clip: true
                model: controller.model

                rotation: flipped ? 270 : 0

                boundsBehavior: Flickable.StopAtBounds
                boundsMovement: Flickable.StopAtBounds

                topMargin: 2
                bottomMargin: topMargin

                contentWidth: width - scroll.visibleWidth

                width: (flipped ? parent.height : parent.width) - 2
                height: (flipped ? parent.width : parent.height) - 2

                delegate: RowDelegate {
                    maxWidth: list.contentWidth

                    word: roleHtmlWord
                    count: roleCount
                    percent: rolePercent
                }

                add: Transition {
                    NumberAnimation { properties: "x"; from: -list.width / 4 }
                }

                remove: Transition {
                    NumberAnimation { properties: "opacity"; to: 0 }
                }

                move: Transition {
                    NumberAnimation { properties: "y" }
                }

                displaced: Transition {
                    NumberAnimation { properties: "y" }
                }

                ScrollBar.vertical: ScrollBar {
                    id: scroll

                    readonly property int visibleWidth:
                        visible ? width : 0
                }
            }
        }

        ProgressBar {
            id: progress

            Layout.fillWidth: true

            value: controller.progress

            Text {
                anchors.centerIn: parent

                text: `${Math.round(progress.position * 100)}%`
                visible: progress.position > 0
            }

            Text {
                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                }

                leftPadding: 8

                text: `${controller.wordsCount} unique`
                visible: controller.wordsCount > 0
            }

            Text {
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }

                rightPadding: 8

                text: `${controller.wordsPerSec} words/sec.`
                visible: controller.wordsPerSec > 0
            }
        }
    }

    component RowDelegate: Item {
        id: delegate

        property int maxWidth

        property string word
        property int count
        property real percent

        readonly property int spacing: 4
        readonly property int padding: 4

        readonly property variant colors: [
            "azure", "beige", "bisque", "khaki", "pink", "plum", "skyblue"
        ]

        Behavior on percent { PropertyAnimation {} }

        width: maxWidth * percent
        height: content.height + spacing

        Rectangle {
            id: content

            anchors.centerIn: parent

            width: Math.max(parent.width - 2 * padding,
                            2 * border.width + 3)

            height: label.height

            border {
                width: 1
                color: "gray"
            }

            color: colors[Math.floor(colors.length * Math.random())]

            Text {
                id: label

                anchors.verticalCenter: parent.verticalCenter

                padding: 6
                width: maxWidth - 2 * delegate.padding

                elide: Text.ElideMiddle
                text: `<b>${word}</b> (${count})`
            }
        }
    }
}
