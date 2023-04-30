#include "Model.h"

#include <functional>

namespace
{
enum Roles
{
    Word = Qt::UserRole,
    HtmlWord,
    Count,
    Percent
};

const float cMinPerc = 0.1;
const int cDefaultSize = 15;

} // anonymous namespace


Model::Model(QObject* parent)
    : QAbstractListModel(parent)
{
    setMaxSize(cDefaultSize);
}

QHash<int, QByteArray> Model::roleNames() const
{
    return {
        { Roles::Word, "roleWord" },
        { Roles::HtmlWord, "roleHtmlWord" },
        { Roles::Count, "roleCount" },
        { Roles::Percent, "rolePercent" },
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

void Model::setViewOrder(int value)
{
    if (m_order == value)
    {
        return;
    }

    beginResetModel();
    std::reverse(m_items.begin(), m_items.end());
    endResetModel();

    m_order = Qt::SortOrder(value);
    emit viewOrderChanged();
}

void Model::setMaxSize(int value)
{
    if (value < 1 || m_maxSize == value)
    {
        return;
    }

    m_maxSize = value;
    emit maxSizeChanged();

    if (m_items.isEmpty())
    {
        return;
    }

    const int curSize = m_items.size();
    const int diff = curSize - value;

    if (diff <= 0)
    {
        emit needMoreWords();
        return;
    }

    const int removeFrom = isAsc() ? 0 : value;
    const int removeTo = (isAsc() ? diff : curSize) - 1;

    beginRemoveRows({}, removeFrom, removeTo);
    m_items.remove(removeFrom, diff);
    endRemoveRows();

    rescale();
}

int Model::find(const QString& word) const
{
    auto it = std::find_if(m_items.cbegin(), m_items.cend(),
                           [&word](const auto& item)
    {
        return item.word == word;
    });

    if (it != m_items.cend())
    {
        return std::distance(m_items.cbegin(), it);
    }

    return -1;
}

void Model::update(const QString& word, int count)
{
    if (count < 1 || word.isEmpty())
    {
        return;
    }

    int row = find(word);

    if (row != -1)
    {
        m_items[row].count = count;

        const auto index = QAbstractListModel::index(row);
        emit dataChanged(index, index, { Roles::Count });
    }
    else
    {
        int minCount = 0;

        if (!m_items.isEmpty())
        {
            minCount = isAsc() ? m_items.first().count
                               : m_items.last().count;
        }

        if (count > minCount || m_items.size() < m_maxSize)
        {
            if (m_items.size() == m_maxSize)
            {
                const int index = isAsc() ? 0 : (m_items.size() - 1);

                beginRemoveRows({}, index, index);
                m_items.remove(index);
                endRemoveRows();
            }

            row = isAsc() ? 0 : m_items.size();

            beginInsertRows({}, row, row);
            m_items.insert(row, { count, word });
            endInsertRows();
        }
    }

    if (row != -1)
    {
        move(row);
        rescale();
    }
}

void Model::move(int row)
{
    const QString word = m_items[row].word;

    std::sort(m_items.begin(), m_items.end(),
              [this](const auto& lhv, const auto& rhv)
    {
        return isAsc() ? (lhv.count < rhv.count) : (lhv.count > rhv.count);
    });

    int newRow = find(word);

    if (newRow > row)
    {
        ++newRow;
    }

    if (row != newRow)
    {
        beginMoveRows({}, row, row, {}, newRow);
        endMoveRows();
    }
}

void Model::rescale()
{
    const int maxCount = isAsc() ? m_items.last().count
                                 : m_items.first().count;

    m_ratio = (1. - cMinPerc) / maxCount;

    const auto first = QAbstractListModel::index(0);
    const auto last = QAbstractListModel::index(m_items.size()-1);
    emit dataChanged(first, last, { Roles::Percent });
}

void Model::reset()
{
    beginResetModel();
    m_items.clear();
    endResetModel();
}
