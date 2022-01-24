import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15


Window {
    id: root;

    width: 500
    height: 600
    visible: true
    title: qsTr("Game of Fifteen")

    Background{
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            spacing: 2

            Header {
                Layout.alignment: Qt.AlignHCenter
                moveText: "0"
                scoreText: "0"
                onNewGame: {
                    board.openNewGame()
                }
            }

            Board {
                id: board
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
}



