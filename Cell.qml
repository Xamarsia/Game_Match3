import QtQuick 2.0
import QtGraphicalEffects 1.15

Rectangle {
    id: root

    property alias circleColor: circle.color
    property alias circleMargins: circle.anchors.margins

    signal itemClicked();

    color: "#00ffffff"

    Behavior on visible{
        SequentialAnimation {
            PauseAnimation { duration: 500 }
            NumberAnimation { properties: "visible"; duration: 700 }
        }
    }

    SequentialAnimation {
        id: droppingOut
        running: false
        NumberAnimation {
            target: cell; property: "y";
            from: - grid.cellHeight;
            to: (boardModel.getRow(index) * grid.cellHeight);
            duration: (boardModel.getRow(index)) ? (500 / boardModel.getRow(index)) : 600
        }
        PauseAnimation { duration: 500 }
    }

    onVisibleChanged: {
        if(visible) {
            droppingOut.start()
            boardModel.doAllCellsVisible();
            boardModel.clearAllMatches()
        } else {
            boardModel.moveInvisibleItemTop(index)
            boardModel.setRandomColor(index);
            boardModel.setVisible(index, true)
        }
    }

    states: [
        State {
            name: "PRESSED"
            PropertyChanges { target: cell; circleMargins: 15;}
        },
        State {
            name: "RELEASED"
            PropertyChanges { target: cell; circleMargins: 3;}
        },
        State {
            name: "JUMP"
            PropertyChanges { target: cell; circleMargins: 3;}
        }
    ]

    transitions: [
        Transition {
            from: "PRESSED"
            to: "RELEASED"
            ColorAnimation { target: cell; duration: 1000}
        },
        Transition {
            from: "RELEASED"
            to: "PRESSED"
            ColorAnimation { target: cell; duration: 1000}
        },
        Transition {
            from: "PRESSED"
            to: "JUMP"
            SequentialAnimation {
                NumberAnimation { target: cell; property: "circleMargins"; from: 3; to: 15; duration:150}
                NumberAnimation { target: cell; property: "circleMargins"; from: 15; to: 3 ; duration: 150}
                NumberAnimation { target: cell; property: "circleMargins"; from: 3; to: 15; duration:150}
                NumberAnimation { target: cell; property: "circleMargins"; from: 15; to: 3 ; duration: 150}
            }
        }
    ]

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

