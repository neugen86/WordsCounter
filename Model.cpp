#include "Model.h"

#include <functional>

namespace
{
enum Roles
{
    Word = Qt::UserRole,
    HtmlWord,
    Count,
    Proportion
};

const float cMinPerc = 0.0;
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
        { Roles::Proportion, "roleProportion" },
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

    case Roles::Proportion:
        return m_ratio * item.count;

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

    m_order = Qt::SortOrder(value);
    emit viewOrderChanged();

    beginResetModel();
    std::reverse(m_items.begin(), m_items.end());
    endResetModel();
}

void Model::setMaxSize(int value)
{
    if (value < 1 || m_maxSize == value)
    {
        return;
    }

    m_maxSize = value;
    emit maxSizeChanged();

    if (m_items.size() < value)
    {
        emit needMoreWords();
        return;
    }

    removeExtraItems();
}

int Model::indexOf(const QString& word) const
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

    int index = indexOf(word);

    if (index != -1)
    {
        m_items[index].count = count;

        const auto modelIdx = QAbstractListModel::index(index);
        emit dataChanged(modelIdx, modelIdx, { Roles::Count });
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
            index = isAsc() ? 0 : m_items.size();

            beginInsertRows({}, index, index);
            m_items.insert(index, { count, word });
            endInsertRows();
        }
    }

    if (index != -1)
    {
        sort(index);
        removeExtraItems();
        updateProportions();
    }
}

void Model::sort(int changedIndex)
{
    const QString word = m_items[changedIndex].word;

    std::sort(m_items.begin(), m_items.end(),
              [this](const auto& lhv, const auto& rhv)
    {
        return isAsc() ? (lhv.count < rhv.count)
                       : (lhv.count > rhv.count);
    });

    int newIndex = indexOf(word);
    Q_ASSERT(newIndex != -1);

    if (changedIndex == newIndex)
    {
        return;
    }

    if (newIndex > changedIndex)
    {
        ++newIndex;
    }

    beginMoveRows({}, changedIndex, changedIndex,
                  {}, newIndex);
    endMoveRows();
}

void Model::removeExtraItems()
{
    const int curSize = m_items.size();
    const int count = curSize - m_maxSize;

    if (count < 1)
    {
        return;
    }

    const int removeFrom = isAsc() ? 0 : (curSize - count);
    const int removeTo = (isAsc() ? count : curSize) - 1;

    beginRemoveRows({}, removeFrom, removeTo);
    m_items.remove(removeFrom, count);
    endRemoveRows();
}

void Model::updateProportions()
{
    const int maxCount = isAsc() ? m_items.last().count
                                 : m_items.first().count;

    m_ratio = (1. - cMinPerc) / maxCount;

    const auto firstIdx = QAbstractListModel::index(0);
    const auto lastIdx = QAbstractListModel::index(m_items.size()-1);
    emit dataChanged(firstIdx, lastIdx, { Roles::Proportion });
}

void Model::reset()
{
    beginResetModel();
    m_items.clear();
    endResetModel();
}
