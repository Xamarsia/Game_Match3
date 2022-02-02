#pragma once
#include <QAbstractListModel>
#include <QColor>
#include <QJsonObject>

class Board : public QAbstractListModel {
     Q_OBJECT

public:
    Board(QObject *parent = 0);

    enum BoardRoles {
        ColorRole = Qt::UserRole + 1,
        VisibleRole
    };

    struct Cell {
        QColor color{};
        bool visible{true};
    };

    Q_INVOKABLE int row() const { return rowsCount; }
    Q_INVOKABLE int column() const { return columnsCount; }
    Q_INVOKABLE int getRow(const int index) const;
    Q_INVOKABLE bool checkStepsAvailable();

    Q_INVOKABLE void newGame();
    Q_INVOKABLE bool takeStep(const int firstIndex,const int secondIndex);
    Q_INVOKABLE void remove(int row);
    Q_INVOKABLE void moveInvisibleItemTop(int index);
    Q_INVOKABLE void setVisible(const int cellIndex, bool visible);
    Q_INVOKABLE void setRandomColor(const int cellIndex);
    Q_INVOKABLE void solveAllMatches();
    Q_INVOKABLE void doAllCellsVisible();


    int getColumn(const int index) const;
    QHash<int, QByteArray> roleNames() const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                          const QModelIndex &destinationParent, int destinationChild) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;


signals:
    void win();
    void move();
    void treeInRow(int points);

private:
    int rowsCount {};
    int columnsCount {};
    int points;
    QString sourceFile = ":/config.json";

    QVector<QVector<int>> threeInColumnAfterMove(const QVector<Cell>& cells) const;
    QVector<QVector<int>> threeInRowAfterMove(const QVector<Cell>& cells) const;
    QVector<QVector<int>> findAllMatches(const QVector<Cell>& cells) const;

    bool isAvailableStep(const QVector<Cell>& cells) const;

    void setInvisibleCellAfterMove(const QVector<Cell>& cells);



    void moveItemDown(const int index);
    void moveItemLeft(const int index);
    void moveItemRight(const int index);
    void moveItemUp(const int index);

    QJsonObject getJsonObject(const QString& sourceFile);
    void read(const QJsonObject &json);

    QHash<int, QByteArray> m_roleNames;
    QVector<Cell> m_cells;
    QVector<QColor> m_colors;
};



