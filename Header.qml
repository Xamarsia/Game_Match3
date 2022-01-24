import QtQuick 2.0
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    property alias scoreText: scoreText.text
    property alias moveText: moveText.text

    signal newGame()

    height: 95
    width: 400
    color: "lemonchiffon"

    RowLayout {
        anchors.fill: parent
        spacing: 6
        anchors.margins: 20

        ColumnLayout {
            id: moveCounter

            Layout.alignment: Qt.AlignLeft
            spacing: 10

            Text {
                Layout.alignment: Qt.AlignCenter
                text: "move"
                font.pixelSize: 24
            }

            Text {
                id: moveText
                Layout.alignment: Qt.AlignCenter
                font.pixelSize: 24
            }
        }

        NewGameButton {
            id: mixButton
            Layout.alignment: Qt.AlignCenter
            onCliced: {
                root.newGame()
            }
        }

        ColumnLayout {
            id: scoreCounter

            Layout.alignment: Qt.AlignRight
            spacing: 10

            Text {
                Layout.alignment: Qt.AlignCenter
                text: "score"
                font.pixelSize: 24
            }

            Text {
                id: scoreText
                Layout.alignment: Qt.AlignCenter
                font.pixelSize: 24
            }
        }
    }
}



