#pragma once
#include <QAbstractListModel>
#include <QColor>

class Board : public QAbstractListModel {
     Q_OBJECT

public:
    Board(QObject *parent = 0);

    enum BoardRoles {
        ColorRole = Qt::UserRole + 1
    };

    struct Cell {
        QColor color{};
    };

    Q_INVOKABLE void newGame();
    Q_INVOKABLE bool takeStep(const int firstIndex,const int secondIndex);

    Q_INVOKABLE int row() { return rowsCount; }
    Q_INVOKABLE int column() { return columnsCount; }
    Q_INVOKABLE void remove(int row);

    QHash<int, QByteArray> roleNames() const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                          const QModelIndex &destinationParent, int destinationChild) override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

signals:
    void win();

private:

    int rowsCount {4};
    int columnsCount {4};

    int getRow(int index, int rowsCount, int columnsCount);
    int getColumn(int index, int rowsCount, int columnsCount);

    void moveEmptyItemDown(const int firstIndex);
    void moveEmptyItemLeft(const int firstIndex);
    void moveEmptyItemRight(const int firstIndex);
    void moveEmptyItemUp(const int firstIndex);

    QHash<int, QByteArray> m_roleNames;
    QVector<Cell> m_cells;
    QVector<QColor> m_colors = {QColor("cyan"), QColor("magenta"), QColor("red"),
                          QColor("darkRed"), QColor("darkCyan"), QColor("darkMagenta"),
                          QColor("green"), QColor("darkGreen"), QColor("yellow"),
                          QColor("blue")};
};



