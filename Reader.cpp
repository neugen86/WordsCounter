#include "Reader.h"

#include <QFile>
#include <QTextStream>
#include <QElapsedTimer>

namespace
{
const int cReaderProgressId =
        qRegisterMetaType<ReaderData>("ReaderData");

bool ReadNextWord(QTextStream& stream, QString& result)
{
    do
    {
        stream >> result;
    }
    while(result.isEmpty() && !stream.atEnd());

    result = result.toLower();

    return !result.isEmpty();
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

    m_mutex.lock();
    m_active = true;

    QElapsedTimer timer;
    timer.start();

    QTextStream stream(&file);

    qint64 totalWords = 0;
    const auto totalBytes = file.size();

    for (ReaderData data; ReadNextWord(stream, data.word) && m_active;)
    {
        data.wordCount = ++m_words[data.word];
        data.wordsPerSec = (++totalWords * 1000) / qMax(1, timer.elapsed());
        data.totalProgress = float(totalBytes - file.bytesAvailable()) / totalBytes;

        m_semaphore.acquire();
        emit dataChanged(data);

        if (m_mutex.tryLock())
        {
            m_cond.wait(&m_mutex);

            timer.restart();
            totalWords = 0;
        }
    }

    m_active = false;
    m_mutex.unlock();

    emit finished(file.error() != QFile::NoError ? file.errorString() : QLatin1String());
}

void Reader::pause()
{
    if (!m_mutex.tryLock())
    {
        m_mutex.unlock();
    }
}

void Reader::resume()
{
    m_cond.notify_one();
}

void Reader::stop()
{
    m_active = false;
    m_semaphore.release();

    resume();
}
