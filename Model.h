#pragma once

#include <QHash>
#include <QAbstractListModel>

class Model : public QAbstractListModel
{
    Q_OBJECT

    enum Roles
    {
        Word = Qt::UserRole,
        HtmlWord,
        Count,
        Percent
    };

    struct Item
    {
        QString word;
        int count = 0;
    };

public:
    explicit Model(QObject* parent = nullptr);

    QHash<int,QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void reset();
    void handle(const QString& word, int count);

private:
    int tryUpdate(const QString& word, int count);

private:
    float m_ratio{1};
    QList<Item> m_items;
    QHash<QString, int> m_rows;

};
