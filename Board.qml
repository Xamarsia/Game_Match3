import QtQuick 2.0
import QtQml.Models 2.15
import QtQuick.Controls 2.12
import BoardController 1.0

Rectangle {
    id: root

    property int firstIndex: -1
    property int secondIndex: -1

    signal openNewGame()

    onOpenNewGame: {
        boardModel.newGame();
    }

    anchors.margins: 20
    color: "lemonchiffon"

    BoardModel {
        id: boardModel
    }

    Rectangle {
        anchors.margins: 20
        anchors.fill: parent
        border.color: Qt.darker("orange", 2)

        GridView {
            id: grid

            interactive: false
            model: boardModel

            anchors.fill: parent
            anchors.margins: 5
            cellHeight: height / boardModel.row()
            cellWidth: width / boardModel.column()

            move: Transition {
                NumberAnimation { properties: "x"; duration: 500; property: "visible"}
                NumberAnimation { properties: "y"; duration: 500 ; property: "visible"}
            }

//            removeDisplaced : Transition {
//                NumberAnimation { properties: "x,y"; duration: 1000 }
//            }

            delegate: Cell {
                id: cell

                circleColor: model.color

                onItemClicked: {
                    if(firstIndex == -1) {
                        firstIndex = index
                    } else if (secondIndex == -1) {
                        secondIndex = index
                        boardModel.takeStep(firstIndex, secondIndex)
                        firstIndex = -1
                        secondIndex = -1
                    }
                }
            }
        } 
    }
}
