import QtQuick 2.0
import QtQml.Models 2.15
import QtQuick.Controls 2.12
import BoardController 1.0

Rectangle {
    id: root

    property int firstIndex: -1
    property int secondIndex: -1
    property int cellSize: 110

    signal doneMoving()
    signal openNewGame()

    onOpenNewGame: {
        boardModel.newGame();
    }

    anchors.margins: 10
    color: "lemonchiffon"

    height: boardModel.row() * (10 + cellSize)
    width: boardModel.column() * (10 + cellSize)

    BoardModel {
        id: boardModel
        onMove: {
            root.doneMoving()

        }
    }

    Rectangle {
        anchors.margins: 10
        anchors.fill: parent
        border.color: Qt.darker("orange", 2)

        GridView {
            id: grid

            interactive: false
            clip: true
            model: boardModel

            anchors.fill: parent
            anchors.margins: 5

            cellHeight: height / boardModel.row()
            cellWidth: width / boardModel.column()

            move: Transition {
                NumberAnimation { properties: "x"; duration: 500; property: "visible"}
                NumberAnimation { properties: "y"; duration: 500 ; property: "visible"}
            }

            delegate: Cell {
                id: cell
                width:  cellSize
                height: width

                circleColor: model.color

                onItemClicked: {
                    if(firstIndex == -1) {
                        firstIndex = index
                        cell.state = "PRESSED"
                    } else if (secondIndex == -1) {
                        cell.state = "PRESSED"
                        secondIndex = index

                        if(boardModel.takeStep(firstIndex, secondIndex)) {
                            var delegateInstance = grid.itemAtIndex(firstIndex);

                            delegateInstance.state = "RELEASED"
                            cell.state = "RELEASED"
                        } else {
                            var delegateInstance = grid.itemAtIndex(firstIndex);
                            delegateInstance.state = "JUMP"
                            cell.state = "JUMP"
                        }

                        firstIndex = -1
                        secondIndex = -1
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
            }
        } 
    }
}
