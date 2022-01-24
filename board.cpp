#include "board.h"
//#include <iostream>
#include <QRandomGenerator>
#include <QtMath>

Board::Board(QObject *parent) : QAbstractListModel(parent) {
    m_roleNames[ColorRole] = "color";

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
}

void Board::moveEmptyItemDown(int firstIndex) {
    if(firstIndex + columnsCount < (rowsCount * columnsCount)) {
        moveRow(QModelIndex(), firstIndex, QModelIndex(), firstIndex + (columnsCount));
        moveRow(QModelIndex(), firstIndex + columnsCount, QModelIndex(), firstIndex);
        //firstIndex = firstIndex + columnsCount;
    }
}

void Board::moveEmptyItemLeft(int firstIndex) {
    if((firstIndex) % columnsCount != 0) {
        moveRow(QModelIndex(), firstIndex - 1, QModelIndex(), firstIndex + 1);
        moveRow(QModelIndex(), firstIndex - 1, QModelIndex(), firstIndex + 1);
        moveRow(QModelIndex(), firstIndex - 1, QModelIndex(), firstIndex + 1);
        //firstIndex = firstIndex - 1;
    }
}

void Board::moveEmptyItemRight(int firstIndex) {
    if((firstIndex+1) % columnsCount != 0) {
        moveRow(QModelIndex(), firstIndex + 1, QModelIndex(), firstIndex);
        moveRow(QModelIndex(), firstIndex + 1, QModelIndex(), firstIndex);
        moveRow(QModelIndex(), firstIndex + 1, QModelIndex(), firstIndex);
        //firstIndex = firstIndex + 1;
    }
}

void Board::moveEmptyItemUp(int firstIndex) {
    if(firstIndex - columnsCount >= 0) {
        moveRow(QModelIndex(), firstIndex, QModelIndex(), firstIndex - (columnsCount - 1));
        moveRow(QModelIndex(), firstIndex - columnsCount, QModelIndex(), firstIndex + 1);
        //firstIndex = firstIndex - columnsCount;
    }
}

int Board::getColumn(int index, int rowsCount, int columnsCount) {
    if(index > ((rowsCount*columnsCount)-1) || index < 0) {
        return -1;
    }
    return index % columnsCount;
}

int Board::getRow(int index, int rowsCount, int columnsCount) {
    if(index > ((rowsCount*columnsCount)-1) || index < 0) {
        return -1;
    }
    return (index / columnsCount) | 0;
}

bool Board::takeStep(int firstIndex, int secondIndex){
    int rowDistance = getRow(secondIndex, rowsCount, columnsCount) - getRow(firstIndex, rowsCount, columnsCount);
    int columnDistance = getColumn(secondIndex, rowsCount, columnsCount) - getColumn(firstIndex, rowsCount, columnsCount);

    if( qFabs(rowDistance) == 1 && columnDistance == 0) {
        if(getRow(secondIndex, rowsCount, columnsCount) < getRow(firstIndex, rowsCount, columnsCount)) {
            moveEmptyItemUp(firstIndex);
        } else {
            moveEmptyItemDown(firstIndex);
        }
        return 1;
    } else if( qFabs(columnDistance) == 1 && rowDistance == 0 ) {
        if(getColumn(secondIndex, rowsCount, columnsCount) < getColumn(firstIndex, rowsCount, columnsCount)) {
            moveEmptyItemLeft(firstIndex);
        } else {
            moveEmptyItemRight(firstIndex);
        }
        return 1;
    }
    return 0;
}
