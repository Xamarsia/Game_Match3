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
            if(cell.visible != value.toBool())
            {
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
    if(!checkStepsAvailable()) {
        newGame();
    }
    if(!isDefaultBoard()){
        newGame();
    }
    points = 0;
}

bool Board::isDefaultBoard() {
    QVector<QVector<int>> matches;
    bool matchesFound = false;
    for(int i = 0; i < columnsCount; ++i) {
        matches = threeInRowAfterMove(i);
        if (!matches.isEmpty()) {
            return false;
        }
    }
    for(int i = 0; i < rowsCount; ++i) {
        matches = threeInColumnAfterMove(i * columnsCount);
        if (!matches.isEmpty()) {
            return false;
        }
    }
    return !matchesFound;
}

void Board::moveInvisibleItemTop(const int index) {
    int column = getColumn(index);
    int row = getRow(index);

    for(int i = row; i > 0; --i) {
        moveItemUp((i * columnsCount) + column);
    }
}

void Board::moveItemDown(const int index) {
    if(index + columnsCount < rowsCount * columnsCount){
        moveRow(QModelIndex(), index, QModelIndex(), index + (columnsCount));
        moveRow(QModelIndex(), index + columnsCount, QModelIndex(), index);
        if(!checkStepsAvailable()) {
            emit noStepsAvailable();
        }
    }
}

void Board::moveItemLeft(const int index) {
    if((index) % columnsCount != 0) {
        moveRow(QModelIndex(), index - 1, QModelIndex(), index + 1);
        moveRow(QModelIndex(), index - 1, QModelIndex(), index + 1);
        moveRow(QModelIndex(), index - 1, QModelIndex(), index + 1);
        if(!checkStepsAvailable()) {
            emit noStepsAvailable();
        }
    }
}

void Board::moveItemRight(const int index) {
    if((index+1) % columnsCount != 0) {
        moveRow(QModelIndex(), index + 1, QModelIndex(), index);
        moveRow(QModelIndex(), index + 1, QModelIndex(), index);
        moveRow(QModelIndex(), index + 1, QModelIndex(), index);
        if(!checkStepsAvailable()) {
            emit noStepsAvailable();
        }
    }
}

void Board::moveItemUp(const int index) {
    if(index - columnsCount >= 0) {
        moveRow(QModelIndex(), index, QModelIndex(), index - (columnsCount - 1));
        moveRow(QModelIndex(), index - columnsCount, QModelIndex(), index + 1);
        if(!checkStepsAvailable()) {
            emit noStepsAvailable();
        }
    }
}
void Board::read(const QJsonObject &json)
{
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

QJsonObject Board::getJsonObject(const QString& sourceFile)
{
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
    int column = getColumn(firstIndex);

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
    int column = getColumn(firstIndex);

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

void Board::doAllCellsVisible() {
    for(int i = 0; i < columnsCount * rowsCount; ++i) {
        if(!m_cells[i].visible) {
            setVisible(i, true);
        }
    }
}

bool Board::clearAllMatches() {
    QVector<QVector<int>> matches;
    bool matchesFound = false;
    for(int i = 0; i < columnsCount; ++i) {
        matches = threeInRowAfterMove(i);
        if (!matches.isEmpty()) {
            if(i == 0){
                threeAfterHorizontalMove(i, i + 1);
            } else if (i < rowsCount) {
                threeAfterHorizontalMove(i, i - 1);
            }
            matchesFound = true;
        }
    }
    for(int i = 0; i < rowsCount; ++i) {
        matches = threeInColumnAfterMove(i * columnsCount);
        if (!matches.isEmpty()) {
            threeAfterVerticalMove(i, i * columnsCount);
            matchesFound = true;
        }
    }
    return matchesFound;
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

QVector<QVector<int>> Board::threeInRowAfterMove(const int index) const {
    int column = getColumn(index);

    QColor color = m_cells[column].color;

    QVector<QVector<int>> threeInRow;
    QVector<int> indexes = {column};

    for(int i = 1; i < rowsCount; ++i) {
        if(m_cells[(i * columnsCount) + column].color == color) {
            indexes.append((i * columnsCount) + column);
        } else {
            if(indexes.size() > 2) {
                threeInRow.append(indexes);
            }
            indexes.clear();
            indexes.append((i * columnsCount) + column);
            color = m_cells[(i * columnsCount) + column].color;
        }
    }
    if(indexes.size() > 2) {
        threeInRow.append(indexes);
    }
    return threeInRow;
}

int Board::getRow(const int index) const {
    return Common::getRow(index, rowsCount, columnsCount);
}
int Board::getColumn(const int index) const {
    return Common::getColumn(index, rowsCount, columnsCount);
}

QVector<QVector<int>> Board::threeInColumnAfterMove(const int index) const {
    int row = getRow(index);
    int firstIndexInCurrentRow = row * columnsCount;
    int firstIndexInNextRow = (row + 1) * columnsCount;

    QColor color = m_cells[firstIndexInCurrentRow].color;
    QVector<QVector<int>> threeInColumn;
    QVector<int> indexes = {firstIndexInCurrentRow};

    for(int i = firstIndexInCurrentRow + 1; i < firstIndexInNextRow; i++) {
        if(m_cells[i].color == color){
            indexes.append(i);
        } else {
            if(indexes.size() > 2) {
                threeInColumn.append(indexes);
            }
            color = m_cells[i].color;
            indexes.clear();
            indexes.append(i);
        }
    }
    if(indexes.size() > 2) {
        threeInColumn.append(indexes);
    }
    return threeInColumn;
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

void Board::threeAfterHorizontalMove(const int firstIndex, const int secondIndex) {
    QVector<QVector<int>> threeInRowVector = threeInColumnAfterMove(firstIndex);
    threeInRowVector.append(threeInRowAfterMove(firstIndex));
    threeInRowVector.append(threeInRowAfterMove(secondIndex));
    for(auto& cellsVector : threeInRowVector) {
        for(auto& cell : cellsVector) {
            setData(index(cell, 0), false, VisibleRole);
            ++points;
        }
    }
    emit treeInRow(points);
}

void Board::threeAfterVerticalMove(const int firstIndex, const int secondIndex) {
    QVector<QVector<int>> threeInRowVector = threeInColumnAfterMove(firstIndex);
    threeInRowVector.append(threeInColumnAfterMove(secondIndex));
    threeInRowVector.append(threeInRowAfterMove(firstIndex));

    for(auto& cellsVector : threeInRowVector) {
        for(auto& cell : cellsVector) {
            setData(index(cell, 0), false, VisibleRole);
            ++points ;
        }
    }
    emit treeInRow(points);
}

bool Board::takeStep(int firstIndex, int secondIndex){
    int rowDistance = getRow(secondIndex) - getRow(firstIndex);
    int columnDistance = getColumn(secondIndex) - getColumn(firstIndex);

    if( qFabs(rowDistance) == 1 && columnDistance == 0) {
        if(!threeBeforeVerticalMove(firstIndex, secondIndex)) {
            return 0;
        } else if(getRow(secondIndex) < getRow(firstIndex)) {
            moveItemUp(firstIndex);
        } else {
            moveItemDown(firstIndex);
        }
        threeAfterVerticalMove(firstIndex, secondIndex);
        emit move();
        return 1;
    } else if( qFabs(columnDistance) == 1 && rowDistance == 0) {
        if(!threeBeforeHorizontalMove(firstIndex, secondIndex)) {
            return 0;
        } else if(getColumn(secondIndex) < getColumn(firstIndex)) {
            moveItemLeft(firstIndex);
        } else {
            moveItemRight(firstIndex);
        }
        threeAfterHorizontalMove(firstIndex, secondIndex);
        emit move();
        return 1;
    }
    return 0;
}
