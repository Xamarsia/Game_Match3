#include "board.h"
#include <QRandomGenerator>
#include <QtMath>
#include <QDebug>

Board::Board(QObject *parent) : QAbstractListModel(parent) {
    m_roleNames[ColorRole] = "color";
    m_roleNames[VisibleRole]  = "visible";

    insertRows(0, rowsCount * columnsCount, QModelIndex());
    newGame();
}

QHash<int, QByteArray> Board::roleNames() const {
    return m_roleNames;
}

bool Board::removeRows(int row, int count, const QModelIndex &parent) {
    Q_UNUSED(parent);
    if (count < 1 || row < 0 || (row + count - 1) >= m_cells.count())
        return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    m_cells.remove(row, count);
    endRemoveRows();
    return true;
}

void Board::remove(int row){
    removeRow(row);
}

int Board::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
       return m_cells.size();
}

QVariant Board::data(const QModelIndex &index, int role) const {
    int row = index.row();
    if(row < 0 || row >= m_cells.count()) {
        return QVariant();
    }

    const Cell &cell = m_cells[row];
    switch(role) {
        case ColorRole:
            return cell.color;
        case VisibleRole:
            return cell.visible;
    }
    return QVariant();
}

bool Board::setData(const QModelIndex &index, const QVariant &value, int role) {
    int row = index.row();
    if(row < 0 || row >= m_cells.count()) {
        return false;
    }

    Cell &cell = m_cells[row];

    switch(role){
        case ColorRole:
            cell.color = value.toString();
            emit dataChanged(index, index, { role });
            return true;
        case VisibleRole:
            cell.visible = value.toBool();
            emit dataChanged(index, index, { role });
            return true;
    }
    return false;
}

bool Board::moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
              const QModelIndex &destinationParent, int destinationChild) {

    if(destinationChild >= sourceRow && destinationChild <= (sourceRow + count)) {
        return false;
    }

    if (!beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild)) {
        // TODO Error processing
        return false;
    }

    QVector<Cell> buffer {};

    for(int i = 0; i < count; i++) {
        buffer.push_back(m_cells[sourceRow]);
        m_cells.remove(sourceRow);
    }

    if(destinationChild > sourceRow) {
        destinationChild = destinationChild - buffer.count();
    }

    while(!buffer.isEmpty()) {
        m_cells.insert(destinationChild, buffer.last());
        buffer.remove(buffer.count() - 1);
    }
    endMoveRows();
    return true;
}

bool Board::insertRows(int row, int count, const QModelIndex &parent) {
    if (count < 1 || row < 0 || row > m_cells.count()) {
        return false;
    }

    beginInsertRows(parent, row, row + count - 1);
    m_cells.insert(row, count, Cell{});
    endInsertRows();
    return true;
}

void  Board::newGame() {
    for(int i = 0; i < rowsCount * columnsCount; i++) {
        int colorIndex = QRandomGenerator::global()->generate() % m_colors.size();
        setData(index(i, 0), m_colors[colorIndex], ColorRole);
    }
    if(!checkStepsAvailable()) {
        emit noStepsAvailable();
    }
}

void Board::moveEmptyItemDown(int firstIndex) {
    if(firstIndex + columnsCount < rowsCount * columnsCount){
        moveRow(QModelIndex(), firstIndex, QModelIndex(), firstIndex + (columnsCount));
        moveRow(QModelIndex(), firstIndex + columnsCount, QModelIndex(), firstIndex);
        emit move();
        if(!checkStepsAvailable()) {
            emit noStepsAvailable();
        }
    }
}

void Board::moveEmptyItemLeft(int firstIndex) {
    if((firstIndex) % columnsCount != 0) {
        moveRow(QModelIndex(), firstIndex - 1, QModelIndex(), firstIndex + 1);
        moveRow(QModelIndex(), firstIndex - 1, QModelIndex(), firstIndex + 1);
        moveRow(QModelIndex(), firstIndex - 1, QModelIndex(), firstIndex + 1);
        emit move();
        if(!checkStepsAvailable()) {
            emit noStepsAvailable();
        }
    }
}

void Board::moveEmptyItemRight(int firstIndex) {
    if((firstIndex+1) % columnsCount != 0) {
        moveRow(QModelIndex(), firstIndex + 1, QModelIndex(), firstIndex);
        moveRow(QModelIndex(), firstIndex + 1, QModelIndex(), firstIndex);
        moveRow(QModelIndex(), firstIndex + 1, QModelIndex(), firstIndex);
        emit move();
        if(!checkStepsAvailable()) {
            emit noStepsAvailable();
        }
    }
}

void Board::moveEmptyItemUp(int firstIndex) {
    if(firstIndex - columnsCount >= 0) {
        moveRow(QModelIndex(), firstIndex, QModelIndex(), firstIndex - (columnsCount - 1));
        moveRow(QModelIndex(), firstIndex - columnsCount, QModelIndex(), firstIndex + 1);
        emit move();
        if(!checkStepsAvailable()) {
            emit noStepsAvailable();
        }
    }
}

int Board::getColumn(const int index, const int rowsCount, const int columnsCount) const{
    if(index > ((rowsCount*columnsCount)-1) || index < 0) {
        return -1;
    }
    return index % columnsCount;
}

int Board::getRow(const int index, const int rowsCount, const int columnsCount) const {
    if(index > ((rowsCount*columnsCount)-1) || index < 0) {
        return -1;
    }
    return (index / columnsCount) | 0;
}

bool Board::threeInRowBeforeHorizontalMove(const int firstIndex, const int secondIndex) const {
    bool firstClick = threeInRowForFirstIndexBeforeHorizontalMove(firstIndex, secondIndex);
    bool secondClick = threeInRowForFirstIndexBeforeHorizontalMove(secondIndex, firstIndex);

    return firstClick || secondClick;
}

bool Board::threeInRowForFirstIndexBeforeHorizontalMove(const int firstIndex, const int secondIndex) const {
    int firstColors = 1;
    QColor color = m_cells[firstIndex].color;

    for(int i = secondIndex; i + columnsCount < m_cells.size(); i += columnsCount) {
        if(m_cells[i + columnsCount].color == color) {
            ++firstColors;
        } else {
            break;
        }
    }

    for(int j = secondIndex; j - columnsCount >= 0; j -= columnsCount) {
        if(m_cells[j - columnsCount].color == color) {
            ++firstColors;
        } else {
            break;
        }
    }

    return firstColors > 2;
}

bool Board::threeInColumnBeforeHorizontalMove(const int firstIndex, const int secondIndex) const {
    bool firstClick = threeInColumnForFirstIndexBeforeHorizontalMove(firstIndex, secondIndex);
    bool secondClick = threeInColumnForFirstIndexBeforeHorizontalMove(secondIndex, firstIndex);

    return firstClick || secondClick;
}

bool Board::threeInColumnForFirstIndexBeforeHorizontalMove(const int firstIndex, const int secondIndex) const {
    int column = getColumn(firstIndex, rowsCount, columnsCount);

    QColor color = QColor(m_cells[firstIndex].color);
    int colors = 1;
    if(firstIndex < secondIndex) {
        for(int i = column + 2, j = secondIndex; i < columnsCount && j + 1 < m_cells.size(); ++i, ++j) {
            if(m_cells[j + 1].color == color) {
                ++colors;
            } else {
                break;
            }
        }
        for(int i = column, j = firstIndex; i > 0; --i, --j) {
            if(i == column){
                if(m_cells[secondIndex].color == color)
                {
                    ++colors;
                } else {
                    break;
                }

            } else if(m_cells[j - 1].color == color) {
                ++colors;
            } else {
                break;
            }
        }
    } else {
        for(int i = column, j = firstIndex; i < columnsCount && j + 1 < m_cells.size(); ++i, ++j) {
            if(i == column) {
                if(m_cells[secondIndex].color == color)
                {
                    ++colors;
                } else {
                    break;
                }
            } else if(m_cells[j + 1].color == color) {
                ++colors;
            } else {
                break;
            }
        }
        for(int i = column - 1, j = secondIndex; i > 0; --i, --j) {
            if(m_cells[j - 1].color == color) {
                ++colors;
            } else {
                break;
            }
        }
    }
    return colors > 2;
}

bool Board::threeInColumnBeforeVerticalMove(const int firstIndex, const int secondIndex) const {
    bool firstClick = threeInColumnForFirstIndexBeforeVerticalMove(firstIndex, secondIndex);
    bool secondClick = threeInColumnForFirstIndexBeforeVerticalMove(secondIndex, firstIndex);

    return firstClick || secondClick;
}

bool Board::threeInColumnForFirstIndexBeforeVerticalMove(const int firstIndex, const int secondIndex) const {
    int column = getColumn(firstIndex, rowsCount, columnsCount);

    QColor color = QColor(m_cells[firstIndex].color);
    int colors = 1;
    for(int i = column, j = secondIndex; i < columnsCount && j + 1 < m_cells.size(); ++i, ++j) {
        if(m_cells[j + 1].color == color) {
            ++colors;
        } else {
            break;
        }
    }
    for(int i = column, j = secondIndex; i > 0; --i, --j) {
        if(m_cells[j - 1].color == color) {
                ++colors;
        } else {
            break;
        }
    }
    return colors > 2;
}

bool Board::threeInRowBeforeVerticalMove(const int firstIndex, const int secondIndex) const {
    bool firstClick = threeInRowForFirstIndexBeforeVerticalMove(firstIndex, secondIndex);
    bool secondClick = threeInRowForFirstIndexBeforeVerticalMove(secondIndex, firstIndex);

    return firstClick || secondClick;
}
bool Board::threeInRowForFirstIndexBeforeVerticalMove(const int firstIndex, const int secondIndex) const {
    int colors = 1;
    QColor color = m_cells[firstIndex].color;

    if(firstIndex < secondIndex) {
        for(int i = firstIndex - columnsCount; i >= 0; i -= columnsCount) {
            if(i == firstIndex - columnsCount) {
                if(m_cells[secondIndex].color == color) {
                    ++colors;
                } else {
                    break;
                }
            } else if(m_cells[i].color == color) {
                ++colors;
            } else {
                break;
            }
        }
        for(int i = secondIndex + columnsCount; i < m_cells.size(); i += columnsCount) {
            if(m_cells[i].color == color) {
                ++colors;
            } else {
                break;
            }
        }
    } else if (secondIndex < firstIndex){
        for(int i = secondIndex - columnsCount; i >= 0; i -= columnsCount) {
            if(m_cells[i].color == color) {
                ++colors;
            } else {
                break;
            }
        }
        for(int i = firstIndex + columnsCount; i < m_cells.size(); i += columnsCount) {
            if(i == firstIndex + columnsCount) {
                if(m_cells[secondIndex].color == color) {
                    ++colors;
                } else {
                    break;
                }
            } else if(m_cells[i].color == color) {
                ++colors;
            } else {
                break;
            }
        }
    }
    return colors > 2;
}

bool Board::checkStepsAvailable() const {
    for(int i = 0; i < rowsCount; ++i) {
        for(int j = 0; j + 1 < columnsCount; ++j) {
            int firstIndex = (i * columnsCount) + j;
            if(threeBeforeHorizontalMove(firstIndex, firstIndex + 1)) {
                return true;
            }
        }
    }
    for(int i = 0; i + columnsCount < m_cells.size(); i++) {
        if(threeBeforeVerticalMove(i, i + columnsCount)) {
            return true;
        }
    }
    return false;
}

bool Board::threeBeforeVerticalMove(const int firstIndex, const int secondIndex) const {
    bool threeIncolumn =  threeInColumnBeforeVerticalMove(firstIndex, secondIndex);
    bool threeInRow = threeInRowBeforeVerticalMove(firstIndex, secondIndex);
    return threeIncolumn || threeInRow;
}

bool Board::threeBeforeHorizontalMove(const int firstIndex, const int secondIndex) const {
    bool threeInRow = threeInRowBeforeHorizontalMove(firstIndex, secondIndex);
    bool threeIncolumn = threeInColumnBeforeHorizontalMove(firstIndex, secondIndex);
    return (threeInRow || threeIncolumn);
}

bool Board::takeStep(int firstIndex, int secondIndex){
    int rowDistance = getRow(secondIndex, rowsCount, columnsCount) - getRow(firstIndex, rowsCount, columnsCount);
    int columnDistance = getColumn(secondIndex, rowsCount, columnsCount) - getColumn(firstIndex, rowsCount, columnsCount);

    if( qFabs(rowDistance) == 1 && columnDistance == 0) {
        if(!threeBeforeVerticalMove(firstIndex, secondIndex)) {
            return 0;
        } else if(getRow(secondIndex, rowsCount, columnsCount) < getRow(firstIndex, rowsCount, columnsCount)) {
            moveEmptyItemUp(firstIndex);
        } else {
            moveEmptyItemDown(firstIndex);
        }
        return 1;
    } else if( qFabs(columnDistance) == 1 && rowDistance == 0) {
        if(!threeBeforeHorizontalMove(firstIndex, secondIndex)) {
            return 0;
        } else if(getColumn(secondIndex, rowsCount, columnsCount) < getColumn(firstIndex, rowsCount, columnsCount)) {
            moveEmptyItemLeft(firstIndex);
        } else {
            moveEmptyItemRight(firstIndex);
        }
        return 1;
    }
    return 0;
}
