import QtQuick 2.0
import QtQuick.Controls 2.15

Rectangle {
    id: root

    signal cliced();

    width: 230
    height: 84
    color: "lemonchiffon"

    Button {
        id: mixButton
        width: 140
        height: 64
        text: qsTr("New game")
        font.pixelSize: 24
        anchors.fill: parent
        anchors.margins: 10

        background: Rectangle {
            radius: 45
            anchors.fill: parent
            border.color: Qt.darker("lemonchiffon", 2)
            color: parent.hovered ? "khaki" : "moccasin"
        }
        onClicked: root.cliced();
    }
}
