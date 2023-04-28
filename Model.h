#pragma once

#include <QHash>
#include <QAbstractListModel>

class Model : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int maxCount READ maxCount WRITE setMaxCount NOTIFY maxCountChanged)

public:
    explicit Model(QObject* parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    int maxCount() const { return m_maxCount; }
    void setMaxCount(int value);

    void handle(const QString& word, int count);
    void reset();

private:
    void sort(int row);
    void rescale();

signals:
    void maxCountChanged();
    void needMoreWords();

private:
    struct Item
    {
        int count{0};
        QString word;
    };

    float m_ratio{1};
    int m_maxCount{0};
    QList<Item> m_items;
    QHash<QString, int> m_rows;
};
