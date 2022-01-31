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
    signal addPoints(int points)
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
        onNoStepsAvailable: {
            console.log("noStepsAvailable")
        }
        onTreeInRow: {
            root.addPoints(points)
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
                SequentialAnimation {
                    NumberAnimation { properties: "x, y"; duration: 500 }
                    PauseAnimation { duration: 1500 }
                }
            }

            delegate: Cell {
                id: cell

                width: cellSize
                height: width

                circleColor: model.color
                visible: model.visible

                onItemClicked: {
                    if(firstIndex == -1) {
                        firstIndex = index
                        cell.state = "PRESSED"
                    } else if (secondIndex == -1) {
                        cell.state = "PRESSED"
                        secondIndex = index
                        var delegateInstance = grid.itemAtIndex(firstIndex);

                        if(boardModel.takeStep(firstIndex, secondIndex)) { 
                            delegateInstance.state = "RELEASED"
                            cell.state = "RELEASED"

                        } else {
                            delegateInstance.state = "JUMP"
                            cell.state = "JUMP"
                        }

                        firstIndex = -1
                        secondIndex = -1
                    }
                }
            }
        } 
    }
}
