#pragma once
#include <QAbstractListModel>
#include <QColor>

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

    Q_INVOKABLE void newGame();
    Q_INVOKABLE bool takeStep(const int firstIndex,const int secondIndex);

    Q_INVOKABLE int row() const { return rowsCount; }
    Q_INVOKABLE int column() const { return columnsCount; }
    Q_INVOKABLE void remove(int row);
    Q_INVOKABLE void moveInvisibleItemTop(int index);
    Q_INVOKABLE void setRandomColor(const int cellIndex);
    Q_INVOKABLE void setVisible(const int cellIndex, bool visible);
    Q_INVOKABLE int getRow(const int index) const;
    Q_INVOKABLE bool clearAllMatches();

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
    void noStepsAvailable();
    void treeInRow(int points);

private:
    const int rowsCount {7};
    const int columnsCount {5};
    int points;

    QVector<QVector<int>> threeInColumnAfterMove(const int index) const;
    QVector<QVector<int>> threeInRowAfterMove(const int index) const;

    void threeAfterHorizontalMove(const int firstIndex, const int secondIndex);
    void threeAfterVerticalMove(const int firstIndex, const int secondIndex);

    bool checkStepsAvailable() const;
    bool threeBeforeVerticalMove(const int firstIndex, const int secondIndex) const ;
    bool threeInColumnBeforeVerticalMove(const int firstIndex, const int secondIndex) const;
    bool threeInColumnForFirstIndexBeforeVerticalMove(const int firstIndex, const int secondIndex) const;

    bool threeInRowBeforeVerticalMove(const int firstIndex, const int secondIndex) const;
    bool threeInRowForFirstIndexBeforeVerticalMove(const int firstIndex, const int secondIndex) const;

    bool threeBeforeHorizontalMove(const int firstIndex, const int secondIndex) const ;

    bool threeInRowBeforeHorizontalMove(const int firstIndex, const int secondIndex) const;
    bool threeInRowForFirstIndexBeforeHorizontalMove(const int firstIndex, const int secondIndex) const;

    bool threeInColumnBeforeHorizontalMove(const int firstIndex, const int secondIndex) const;
    bool threeInColumnForFirstIndexBeforeHorizontalMove(const int firstIndex, const int secondIndex) const;

    void moveItemDown(const int index);
    void moveItemLeft(const int index);
    void moveItemRight(const int index);
    void moveItemUp(const int index);

    QHash<int, QByteArray> m_roleNames;
    QVector<Cell> m_cells;
    QVector<QColor> m_colors = {QColor("cyan"), QColor("magenta"), QColor("red"),
                          QColor("green"), QColor("yellow"), QColor("blue")};
};



