#include "board.h"
#include "common.h"

#include <QRandomGenerator>
#include <QtMath>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

Board::Board(QObject *parent) : QAbstractListModel(parent) {
    m_roleNames[ColorRole] = "color";
    m_roleNames[VisibleRole]  = "visible";
    read(getJsonObject(sourceFile));
    if(m_colors.isEmpty()) {
        m_colors = {QColor("cyan"), QColor("magenta"), QColor("red"), QColor("green"), QColor("yellow"), QColor("blue")};
    }
    if(rowsCount < 2) {
        rowsCount = 4;
    }
    if(columnsCount < 2) {
        columnsCount = 4;
    }
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

void Board::remove(int row) {
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
            if(cell.color != value.toString()) {
                cell.color = value.toString();
                emit dataChanged(index, index, { role });
                return true;
            }
        case VisibleRole:
            if(cell.visible != value.toBool()) {
                cell.visible = value.toBool();
                emit dataChanged(index, index, { role });
                return true;
            }
    }
    return false;
}

bool Board::moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
              const QModelIndex &destinationParent, int destinationChild) {

    if(destinationChild >= sourceRow && destinationChild <= (sourceRow + count)) {
        return false;
    }

    if (!beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild)) {
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
    QColor color = QRandomGenerator::global()->generate() % m_colors.size();
    m_cells.insert(row, count, Cell{{color}, {}});
    endInsertRows();
    return true;
}

void Board::setRandomColor(const int cellIndex) {
    int colorIndex = QRandomGenerator::global()->generate() % m_colors.size();
    setData(index(cellIndex, 0), m_colors[colorIndex], ColorRole);
}

void Board::setVisible(const int cellIndex, bool visible) {
    setData(index(cellIndex, 0), visible, VisibleRole);
}

void  Board::newGame() {
    for(int i = 0; i < rowsCount * columnsCount; i++) {
        int colorIndex = QRandomGenerator::global()->generate() % m_colors.size();
        setData(index(i, 0), m_colors[colorIndex], ColorRole);
        setData(index(i, 0), true, VisibleRole);
    }
    if(!checkStepsAvailable() || isAvailableStep(m_cells)) {
        newGame();
    }
    points = 0;
}

void Board::moveInvisibleItemTop(const int index) {
    int column = getColumn(index);
    int row = getRow(index);

    for(int i = row; i > 0; --i) {
        moveItemUp((i * columnsCount) + column);
    }
}

void Board::moveItemDown(const int index) {
    if(index + columnsCount < rowsCount * columnsCount) {
        moveRow(QModelIndex(), index, QModelIndex(), index + (columnsCount));
        moveRow(QModelIndex(), index + columnsCount, QModelIndex(), index);
    }
}

void Board::moveItemLeft(const int index) {
    if((index) % columnsCount != 0) {
        moveRow(QModelIndex(), index - 1, QModelIndex(), index + 1);
        moveRow(QModelIndex(), index - 1, QModelIndex(), index + 1);
        moveRow(QModelIndex(), index - 1, QModelIndex(), index + 1);
    }
}

void Board::moveItemRight(const int index) {
    if((index+1) % columnsCount != 0) {
        moveRow(QModelIndex(), index + 1, QModelIndex(), index);
        moveRow(QModelIndex(), index + 1, QModelIndex(), index);
        moveRow(QModelIndex(), index + 1, QModelIndex(), index);
    }
}

void Board::moveItemUp(const int index) {
    if(index - columnsCount >= 0) {
        moveRow(QModelIndex(), index, QModelIndex(), index - (columnsCount - 1));
        moveRow(QModelIndex(), index - columnsCount, QModelIndex(), index + 1);
    }
}

void Board::read(const QJsonObject &json) {
    if (json.contains("colors") && json["colors"].isArray()) {
        QJsonArray npcArray = json["colors"].toArray();
        m_colors.clear();
        for (int npcIndex = 0; npcIndex < npcArray.size(); ++npcIndex) {
            QString color = npcArray[npcIndex].toString();
            m_colors.append(color);
        }
    }
    if (json.contains("columns") && json["columns"].isDouble())
        columnsCount = json["columns"].toInt();

    if (json.contains("rows") && json["rows"].isDouble())
        rowsCount = json["rows"].toInt();
}

QJsonObject Board::getJsonObject(const QString& sourceFile) {
    QFile loadFile;
    loadFile.setFileName(sourceFile);
    if(!loadFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        qWarning("Couldn't open ':/config.json' file.");
    }

    QString val;
    val = loadFile.readAll();
    loadFile.close();

    QJsonDocument document = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject jsonObject = document.object();

    return jsonObject;
}

void Board::doAllCellsVisible() {
    for(int i = 0; i < columnsCount * rowsCount; ++i) {
        if(!m_cells[i].visible) {
            setVisible(i, true);
        }
    }
}

void Board::solveAllMatches() {
    setInvisibleCellAfterMove(m_cells);
}

bool Board::checkStepsAvailable() {
    for(int row = 0; row < rowsCount; ++row) {
        for(int column = 0; column + 1 < columnsCount; ++column) {
            QVector<Cell> cells = m_cells;
            int firstIndex = (columnsCount * row) + column;
            int secondIndex = firstIndex + 1;
            cells.swapItemsAt(firstIndex, secondIndex);
            if(isAvailableStep(cells)) {
                return true;
            }
        }
    }

    for(int i = 0; i + columnsCount < m_cells.size(); i++) {
        QVector<Cell> cells = m_cells;
        cells.swapItemsAt(i, i + columnsCount);
        if(isAvailableStep(cells)) {
            return true;
        }
    }
    return false;
}

int Board::getRow(const int index) const {
    return Common::getRow(index, rowsCount, columnsCount);
}

int Board::getColumn(const int index) const {
    return Common::getColumn(index, rowsCount, columnsCount);
}

QVector<QVector<int>> Board::threeInColumnAfterMove(const QVector<Cell>& cells) const {
    QVector<QVector<int>> threeInRow;
    QColor color;
    QVector<int> indexes;
    for(int row = 0; row < rowsCount; ++row) {
        int firstIndexInCurrentRow = row * columnsCount;
        int firstIndexInNextRow = (row + 1) * columnsCount;
        int firstIndexInPreviousRow = (row - 1) * columnsCount;

        color = cells[firstIndexInCurrentRow].color;
        indexes = {firstIndexInCurrentRow};

        for(int i = firstIndexInCurrentRow + 1; i < firstIndexInNextRow; i++) {
            if(cells[i].color == color){
                indexes.append(i);
            } else {
                if(indexes.size() > 1) {
                    for(int j = 0; j + 1 < indexes.size(); j++) {
                        if(firstIndexInNextRow < cells.size()) {
                            if(cells[indexes[j] + columnsCount].color == color && cells[indexes[j] + 1 + columnsCount].color == color) {
                                indexes.append(indexes[j] + columnsCount);
                                indexes.append(indexes[j] + 1  + columnsCount);
                            }
                        }
                        if(firstIndexInPreviousRow >= 0) {
                            if(cells[indexes[j] - columnsCount].color == color && cells[indexes[j] + 1 - columnsCount].color == color) {
                                indexes.append(indexes[j] - columnsCount);
                                indexes.append(indexes[j] + 1  - columnsCount);
                            }
                        }
                        if(indexes.size() > 2) {
                            threeInRow.append(indexes);
                            indexes.clear();
                        }
                    }
                }
                if(indexes.size() > 2) {
                    threeInRow.append(indexes);
                }
                color = cells[i].color;
                indexes.clear();
                indexes.append(i);
            }
        }
        if(indexes.size() > 2) {
            threeInRow.append(indexes);
        }
    }
    return  threeInRow;
}

QVector<QVector<int>> Board::threeInRowAfterMove(const QVector<Cell>& cells) const {
    QVector<QVector<int>> threeInRow;
    QColor color;
    QVector<int> indexes;

    for(int column = 0; column < columnsCount; ++column) {
        color = cells[column].color;
        indexes = {column};

        for(int i = 1; i < rowsCount; ++i) {
            if(cells[(i * columnsCount) + column].color == color) {
                indexes.append((i * columnsCount) + column);
            } else {
                if(indexes.size() > 2) {
                    threeInRow.append(indexes);
                }
                indexes.clear();
                indexes.append((i * columnsCount) + column);
                color = cells[(i * columnsCount) + column].color;
            }
        }
        if(indexes.size() > 2) {
            threeInRow.append(indexes);
        }
    }

    return  threeInRow;
}

QVector<QVector<int>> Board::findAllMatches(const QVector<Cell>& cells) const{
    QVector<QVector<int>> threeInRow = threeInColumnAfterMove(cells);
    threeInRow.append(threeInRowAfterMove(cells));

    return  threeInRow;
}

bool Board::isAvailableStep(const QVector<Cell>& cells) const {
    return !(findAllMatches(cells).isEmpty());
}

void Board::setInvisibleCellAfterMove(const QVector<Cell>& cells) {
    QVector<QVector<int>> threeInRowVector = findAllMatches(cells);
    for(auto& cellsVector : threeInRowVector) {
        for(auto& cell : cellsVector) {
            if(cells[cell].visible == false) {
                return;
            }
            setData(index(cell, 0), false, VisibleRole);
            ++points;
        }
    }
    emit treeInRow(points);
}

bool Board::takeStep(int firstIndex, int secondIndex){
    int rowDistance = getRow(secondIndex) - getRow(firstIndex);
    int columnDistance = getColumn(secondIndex) - getColumn(firstIndex);

    QVector<Cell> cells = m_cells;
    if( qFabs(rowDistance) == 1 && columnDistance == 0) {
        cells.swapItemsAt(firstIndex, secondIndex);
        if(!isAvailableStep(cells)) {
            return 0;
        }else if(getRow(secondIndex) < getRow(firstIndex)) {
            moveItemUp(firstIndex);
        } else {
            moveItemDown(firstIndex);
        }
        setInvisibleCellAfterMove(m_cells);
        emit move();
        return 1;
    } else if( qFabs(columnDistance) == 1 && rowDistance == 0) {
        cells.swapItemsAt(firstIndex, secondIndex);
        if(!isAvailableStep(cells)) {
            return 0;
        } else if(getColumn(secondIndex) < getColumn(firstIndex)) {
            moveItemLeft(firstIndex);
        } else {
            moveItemRight(firstIndex);
        }
        setInvisibleCellAfterMove(m_cells);
        emit move();
        return 1;
    }
    return 0;
}
