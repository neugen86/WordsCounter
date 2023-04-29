#pragma once

#include <QAbstractListModel>

class Model : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int maxSize READ maxSize WRITE setMaxSize NOTIFY maxSizeChanged)

public:
    explicit Model(QObject* parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    bool isFull() const { return m_items.size() >= m_maxSize; }
    bool contains(const QString& word) const { return m_rows.contains(word); }

    int maxSize() const { return m_maxSize; }
    void setMaxSize(int value);

    void update(const QString& word, int count);
    void reset();

private:
    void sort(int row);
    void rescale();

signals:
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
    QHash<QString, int> m_rows;
};
