import QtQuick 2.0
import QtGraphicalEffects 1.15

Rectangle {
    id: root

    property alias circleColor: circle.color
    signal itemClicked();

    color: "#00ffffff"
    width:  110
    height: 110

    MouseArea {
        anchors.fill: parent
        onClicked: {
            root.itemClicked()
        }
    }

    Rectangle {
        id: circle

        color: Qt.rgba(Math.random(),Math.random(),Math.random(),1);
        radius: Math.min(width, height)
        anchors.margins: 3
        anchors.fill: parent
        border.color: Qt.darker(color, 2)
    }
}

