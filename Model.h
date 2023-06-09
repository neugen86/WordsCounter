#pragma once

#include <QAbstractListModel>

class Model : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int maxSize READ maxSize WRITE setMaxSize NOTIFY maxSizeChanged)
    Q_PROPERTY(int sortOrder READ sortOrder WRITE setSortOrder NOTIFY sortOrderChanged)

public:
    explicit Model(QObject* parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    int maxSize() const { return m_maxSize; }
    void setMaxSize(int value);

    int sortOrder() const { return m_order; }
    void setSortOrder(int value);

    int indexOf(const QString& word) const;
    bool isFull() const { return m_items.size() >= m_maxSize; }

    void update(const QString& word, int count);
    void reset();

private:
    bool isAsc() const { return m_order == Qt::AscendingOrder; }

    void sort(int changedIndex);
    void removeExtraItems();
    void updateProportions();

signals:
    void sortOrderChanged(QPrivateSignal = {});
    void maxSizeChanged(QPrivateSignal = {});
    void needMoreWords(QPrivateSignal = {});

private:
    struct Item
    {
        int count{0};
        QString word;
    };

    int m_maxSize{0};
    float m_ratio{1};
    QList<Item> m_items;
    Qt::SortOrder m_order{Qt::DescendingOrder};
};
