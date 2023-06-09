import QtQuick
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Controls

import WordsCounter 1.0

Window {
    visible: true
    minimumWidth: 500
    minimumHeight: 630
    title: qsTr("WordsCounter")

    property bool sortedByModel: false
    readonly property int defaultSortOrder: Qt.DescendingOrder

    Controller {
        id: controller

        Component.onCompleted: {
            model.sortOrder = defaultSortOrder
        }
    }

    FileDialog {
        id: dialog

        title: "Choose file"
        options: FileDialog.ReadOnly
        fileMode: FileDialog.OpenFile

        property bool resumeAfterClose: false

        onVisibleChanged: {
            if (visible) {
                if (controller.state === Controller.Running) {
                    resumeAfterClose = true
                    controller.startStop()
                }
            }
            else if (resumeAfterClose) {
                resumeAfterClose = false
                controller.startStop()
            }
        }

        onAccepted: {
            controller.file = selectedFile
        }
    }

    ColumnLayout {
        id: mainLayout

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
                elide: Text.ElideRight

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
                        return "Stop"
                    case Controller.Stopped:
                        return "Continue"
                    default:
                        return "Start"
                    }
                }

                onClicked: {
                    controller.startStop()
                }
            }

            Button {
                implicitWidth: 100

                text: "Cancel"
                enabled: controller.state !== Controller.Idle

                onClicked: {
                    controller.cancel()
                }
            }

            Item {
                implicitWidth: 30
            }

            RowLayout {
                spacing: 20

                Column {
                    spacing: mainLayout.spacing

                    CheckBox {
                        text: "Sort by model"
                        checked: sortedByModel

                        onCheckedChanged: {
                            sortedByModel = checked
                            controller.model.sortOrder = defaultSortOrder
                            list.verticalLayoutDirection = ListView.TopToBottom
                        }
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
                            controller.model.sortOrder === Qt.AscendingOrder ? 0 : 1
                        }

                        onCurrentValueChanged: {
                            if (sortedByModel) {
                                controller.model.sortOrder = currentValue
                            }
                            else {
                                const sameAsModel = currentValue === controller.model.sortOrder

                                list.verticalLayoutDirection = sameAsModel ? ListView.TopToBottom
                                                                           : ListView.BottomToTop
                            }

                            scroll.position = 0
                        }
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
        }

        RowLayout {
            Slider {
                id: slider

                implicitWidth: 400

                value: controller.model.maxSize
                enabled: from < to

                onMoved: {
                    controller.model.maxSize = value
                }

                Component.onCompleted: {
                    from = controller.model.maxSize
                    to = Math.max(100, from)
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

                readonly property int indent: 2 * parent.border.width

                anchors.centerIn: parent

                spacing: 4
                leftMargin: spacing
                rightMargin: spacing
                bottomMargin: spacing
                topMargin: spacing

                clip: true
                model: controller.model
                rotation: flipped ? 270 : 0

                boundsMovement: Flickable.StopAtBounds

                contentWidth: width - scroll.visibleWidth - 2 * spacing

                width: (flipped ? parent.height : parent.width) - indent
                height: (flipped ? parent.width : parent.height) - indent

                delegate: RowDelegate {
                    width: list.contentWidth

                    text: `<b>${roleHtmlWord}</b> (${roleCount})`
                    proportion: roleProportion
                }

                add: Transition {
                    NumberAnimation { properties: "x"; from: -list.width / 4 }
                }

                remove: Transition { OpacityAnimator { to: 0 } }

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
                    leftMargin: 8
                    verticalCenter: parent.verticalCenter
                }

                text: `${controller.wordsCount} unique`
                visible: controller.wordsCount > 0
            }

            Text {
                anchors {
                    right: parent.right
                    rightMargin: 8
                    verticalCenter: parent.verticalCenter
                }

                text: `${controller.wordsPerSec} words/sec.`
                visible: controller.wordsPerSec > 0
            }
        }
    }

    component RowDelegate: Item {
        id: delegate

        property string text
        property real proportion

        readonly property variant colors: [
            "azure", "beige", "bisque", "khaki", "pink", "plum", "skyblue"
        ]

        implicitHeight: label.height

        Rectangle {
            Behavior on width { PropertyAnimation {} }

            width: Math.max(parent.width * proportion,
                            3 + 2 * border.width)
            height: parent.height

            border {
                width: 1
                color: "gray"
            }

            color: colors[Math.floor(colors.length * Math.random())]

            Text {
                id: label

                anchors.verticalCenter: parent.verticalCenter

                padding: 6
                width: delegate.width

                text: delegate.text
                elide: Text.ElideMiddle
            }
        }
    }
}
