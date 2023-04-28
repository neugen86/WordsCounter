import QtQuick
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Controls

import WordsCounter 1.0

Window {
    width: 600
    height: 610
    visible: true
    minimumWidth: 300
    minimumHeight: 300
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
            TextField {
                enabled: false
                text: controller.file
            }

            Button {
                text: "Select File"

                onClicked: {
                    dialog.open()
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Button {
                implicitWidth: 100

                text: controller.state !== Controller.Running ? "Start" : "Pause"
                enabled: controller.file.length > 0

                onClicked: {
                    controller.startPause()
                }
            }

            Button {
                implicitWidth: 100

                text: "Stop"
                enabled: controller.state !== Controller.Stopped

                onClicked: {
                    controller.stop()
                }
            }

            Text {
                Layout.fillWidth: true

                elide: Text.ElideMiddle
                text: controller.error
                color: "red"
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Slider {
                id: slider

                Layout.fillWidth: true

                to: 100
                stepSize: 1

                value: controller.model.maxSize
                enabled: to > 0

                onMoved: {
                    Qt.callLater(function() {
                        controller.model.maxSize = value
                    })
                }

                Component.onCompleted: {
                    from = controller.model.maxSize
                }
            }

            Text {
                rightPadding: 4
                text: `${controller.model.maxSize} / ${slider.to}`
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            border.width: 1
            border.color: "gray"

            ListView {
                id: list

                anchors {
                    fill: parent
                    margins: 1
                }

                clip: true

                model: controller.model

                readonly property int spacing: 4
                readonly property int padding: 4

                readonly property variant colors: [
                    "azure", "beige", "bisque", "khaki", "pink", "plum", "skyblue"
                ]

                readonly property int availableWidth: width - (scroll.visible ? scroll.width : 0)

                delegate: Item {
                    width: list.availableWidth * animatedPercent
                    height: stripe.height + list.spacing

                    property real animatedPercent: percent
                    Behavior on animatedPercent { PropertyAnimation {} }

                    Rectangle {
                        id: stripe

                        anchors {
                            bottom: parent.bottom
                            horizontalCenter: parent.horizontalCenter
                        }

                        width: parent.width - 2 * list.padding
                        height: label.height

                        border {
                            width: 1
                            color: "gray"
                        }

                        color: list.colors[Math.floor(list.colors.length * Math.random())]

                        Text {
                            id: label

                            anchors.verticalCenter: parent.verticalCenter

                            padding: 6
                            width: list.availableWidth - 2 * list.padding

                            elide: Text.ElideMiddle
                            text: `<b>${htmlWord}</b> (${count})`
                        }
                    }
                }

                add: Transition {
                    NumberAnimation { properties: "x"; from: -list.width/4 }
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
}
