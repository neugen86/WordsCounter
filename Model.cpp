#include "Model.h"

namespace
{
const float cMinPerc = 0.1;
const int cDefaultCount = 15;

} // anonymous namespace


Model::Model(QObject* parent)
    : QAbstractListModel(parent)
{
    reset();
}

QHash<int,QByteArray> Model::roleNames() const
{
    return {
        { Roles::Word, "word" },
        { Roles::HtmlWord, "htmlWord" },
        { Roles::Count, "count" },
        { Roles::Percent, "percent" },
    };
}

int Model::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_items.size();
}

QVariant Model::data(const QModelIndex& index, int role) const
{
    const Item& item = m_items.at(index.row());

    switch (role)
    {
    case Roles::Word:
        return item.word;

    case Roles::HtmlWord:
        return item.word.toHtmlEscaped();

    case Roles::Count:
        return item.count;

    case Roles::Percent:
        return cMinPerc + m_ratio * item.count;

    default:
        return {};
    }
}

void Model::setMaxCount(int value)
{
    if (value < 1 || m_maxCount == value)
    {
        return;
    }

    m_maxCount = value;
    emit maxCountChanged();

    if (m_items.isEmpty())
    {
        return;
    }

    const int curSize = m_items.size();

    if (value >= curSize)
    {
        if (value > curSize)
        {
            emit needMoreWords();
        }
        return;
    }

    beginRemoveRows({}, value, curSize - 1);

    m_rows.removeIf([value](const std::pair<QString, int>& it)
    {
        return it.second >= value;
    });
    m_items.remove(value, curSize - value);

    endRemoveRows();

    rescale();
}

void Model::handle(const QString& word, int count)
{
    if (count < 1 || word.isEmpty())
    {
        return;
    }

    int row = m_rows.value(word, -1);

    if (row < 0)
    {
        if (m_items.isEmpty() ||
            m_items.size() < m_maxCount ||
            count > m_items.last().count)
        {
            if (m_items.size() == m_maxCount)
            {
                const int lastRow = m_items.size() - 1;

                beginRemoveRows({}, lastRow, lastRow);
                m_rows.remove(m_items.last().word);
                m_items.removeLast();
                endRemoveRows();
            }

            row = m_items.size();

            beginInsertRows({}, row, row);
            m_items.append({ word, count });
            m_rows[word] = row;
            endInsertRows();
        }
        else
        {
            return;
        }
    }
    else
    {
        Item& item = m_items[row];

        if (item.count < count)
        {
            item.count = count;

            const auto index = QAbstractListModel::index(row);
            emit dataChanged(index, index, { Roles::Count });
        }
        else
        {
            return;
        }
    }

    sort(row);
    rescale();
}

void Model::sort(int row)
{
    const int prevRow = row;

    for (; row > 0; --row)
    {
        Item& cur = m_items[row];
        Item& prev = m_items[row-1];

        if (cur.count > prev.count)
        {
            m_items.swapItemsAt(row, row-1);
            std::swap(m_rows[cur.word], m_rows[prev.word]);
        }
        else
        {
            break;
        }
    }

    if (row != prevRow)
    {
        beginMoveRows({}, prevRow, prevRow, {}, row);
        endMoveRows();
    }
}

void Model::rescale()
{
    m_ratio = (1. - cMinPerc) / m_items.first().count;

    const auto first = QAbstractListModel::index(0);
    const auto last = QAbstractListModel::index(m_items.size()-1);
    emit dataChanged(first, last, { Roles::Percent });
}

void Model::reset()
{
    beginResetModel();
    m_items.clear();
    m_rows.clear();
    endResetModel();

    setMaxCount(cDefaultCount);
}
