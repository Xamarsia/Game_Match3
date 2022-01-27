import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15


Window {
    id: root;

    minimumWidth: 500
    minimumHeight: 600
    visible: true
    title: qsTr("Game of Fifteen")

    Background{
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            spacing: 2

            Header {
                id: header

                Layout.alignment: Qt.AlignHCenter
                moveText: "0"
                scoreText: "0"
                onNewGame: {
                    board.openNewGame()
                    moveText = "0"
                    scoreText = "0"
                }
            }

            Board {
                id: board

                Layout.alignment: Qt.AlignHCenter
                onDoneMoving: {
                    ++header.moveText
                }
            }
        }
    }
}



