#include "Reader.h"

#include <QFile>
#include <QTextStream>
#include <QMutexLocker>
#include <QElapsedTimer>

namespace
{
const int cReaderProgressId =
        qRegisterMetaType<ReaderData>("ReaderData");

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

    result.removeIf([](const QChar& ch)
    {
        return !ch.isPrint();
    });

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

    qint64 totalWords = 0;
    const auto totalBytes = file.size();

    for (ReaderData data; ReadNextWord(stream, data.word) && m_active;)
    {
        if (data.word.isEmpty() || !data.word.isValidUtf16())
        {
            continue;
        }

        {
            QMutexLocker lock(&m_dataMutex);
            data.count = ++m_words[data.word];
        }

        data.wordsPerSec = (++totalWords * 1000) / qMax(1, timer.elapsed());
        data.totalProgress = float(totalBytes - file.bytesAvailable()) / totalBytes;

        m_semaphore.acquire();
        emit dataChanged(data);

        if (m_pauseMutex.tryLock())
        {
            m_resumeCond.wait(&m_pauseMutex);
            m_paused = false;

            timer.restart();
            totalWords = 0;
        }
    }

    emit finished(file.error() != QFile::NoError ? file.errorString() : QLatin1String());

    m_active = false;
}

void Reader::pause()
{
    if (!m_paused)
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
    m_semaphore.release();

    resume();
}
