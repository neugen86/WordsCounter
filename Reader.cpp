#include "Reader.h"

#include <QFile>
#include <QTextStream>
#include <QMutexLocker>
#include <QElapsedTimer>

namespace
{
const int cReaderDataId =
        qRegisterMetaType<Reader::Data>("ReaderData");

void FilterChars(QString& value)
{
    value.removeIf([](const QChar& ch)
    {
        return !ch.isPrint();
    });

    auto isSuitable = [](const QChar& ch)
    {
        return ch.isLetterOrNumber();
    };

    auto frontIt = std::find_if(value.cbegin(), value.cend(), isSuitable);
    const int left = std::distance(value.cbegin(), frontIt);

    if (frontIt == value.cend())
    {
        value.clear();
        return;
    }

    auto backIt = std::find_if(value.crbegin(), value.crend(), isSuitable);
    const int right = backIt != value.crend() ? std::distance(value.crbegin(), backIt) : 0;

    if (left || right)
    {
        value = value.mid(left, value.length() - left - right);
    }
}

bool ReadNextWord(QTextStream& stream, QString& result)
{
    do
    {
        if (stream.atEnd())
        {
            return false;
        }

        stream >> result;
    }
    while(result.isEmpty());

    FilterChars(result);

    if (result.isEmpty())
    {
        return ReadNextWord(stream, result);
    }

    result = result.toLower();

    return true;
}
} // anonymous namespace


Reader::Reader(QObject* parent)
    : QObject(parent)
{
}

Reader::~Reader()
{
    stop();
}

void Reader::notifyDataReceived()
{
    m_semaphore.release();
}

QHash<QString, int> Reader::words()
{
    QMutexLocker lock(&m_dataMutex);
    return m_words;
}

void Reader::start(const QString& filePath)
{
    if (m_active)
    {
        return;
    }

    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit finished(file.errorString());
        return;
    }

    m_active = true;
    m_paused = false;

    m_semaphore.release();
    m_pauseMutex.tryLock();

    QElapsedTimer timer;
    timer.start();

    QTextStream stream(&file);

    qint64 processedWords = 0;
    const float totalBytes = file.size();

    for (Data data; ReadNextWord(stream, data.word) && m_active;)
    {
        {
            QMutexLocker lock(&m_dataMutex);
            data.count = ++m_words[data.word];
            data.totalWordsCount = m_words.size();
        }

        data.wordsPerSec = (++processedWords * 1000) / qMax(1, timer.elapsed());
        data.totalProgress = (totalBytes - file.bytesAvailable()) / totalBytes;

        m_semaphore.acquire();
        emit dataChanged(data);

        if (m_pauseMutex.tryLock())
        {
            m_resumeCond.wait(&m_pauseMutex);
            m_paused = false;

            processedWords = 0;
            timer.restart();
        }
    }

    const bool hasError = file.error() != QFile::NoError;
    emit finished(hasError ? file.errorString() : QLatin1String());

    m_active = false;
}

void Reader::pause()
{
    if (m_paused)
    {
        return;
    }

    if (!m_pauseMutex.tryLock())
    {
        m_paused = true;
        m_pauseMutex.unlock();
    }
}

void Reader::resume()
{
    m_resumeCond.wakeOne();
}

void Reader::stop()
{
    m_active = false;
    notifyDataReceived();
    resume();
}
