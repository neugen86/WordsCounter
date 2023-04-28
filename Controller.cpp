#include "Controller.h"

#include <QThread>

#include "Reader.h"

Controller::Controller(QObject* parent)
    : QObject(parent)
{
    connect(&m_model, &Model::needMoreWords, this, [this]()
    {
        if (!m_reader)
        {
            return;
        }

        const QHash<QString, int> words = m_reader->words();

        QHash<QString, int>::const_iterator it;
        for (it = words.cbegin(); it != words.cend() && !m_model.isFull(); ++it)
        {
            const QString& word = it.key();

            if (!m_model.contains(word))
            {
                QMetaObject::invokeMethod(this, [this, word, count=it.value()]()
                {
                    m_model.update(word, count);
                },
                Qt::QueuedConnection);
            }
        }
    });
}

Controller::~Controller()
{
    stop();
}

QString Controller::file() const
{
    return m_file.fileName();
}

void Controller::setFile(const QString& filePath)
{
    if (m_file != filePath)
    {
        m_file = filePath;
        emit fileChanged();

        stop();
    }
}

void Controller::startPause()
{
    switch (m_state)
    {
    case State::Running:
        m_reader->pause();
        setState(State::Paused);
        return;

    case State::Paused:
        m_reader->resume();
        setState(State::Running);
        return;

    default:
        break;
    }

    stop();
    m_terminated = false;

    m_reader = new Reader;
    m_thread = new QThread(this);
    m_reader->moveToThread(m_thread);

    connect(m_thread, &QThread::started, m_reader, [this]()
    {
        m_reader->start(m_file.toLocalFile());
    });

    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);

    connect(m_reader, &Reader::dataChanged, this, [this](const ReaderData& data)
    {
        m_reader->notifyDataReceived();

        if (m_state == State::Running)
        {
            setProgress(data.totalProgress);
            setWordsCount(data.totalWordsCount);
            setWordsPerSec(data.wordsPerSec);
        }

        if (!m_terminated)
        {
            m_model.update(data.word, data.count);
        }
    });

    connect(m_reader, &Reader::finished, this, [this](const QString& error)
    {
        setError(error);
        setState(State::Stopped);
    });

    setState(State::Running);

    m_thread->start();
}

void Controller::stop()
{
    m_terminated = true;

    if (m_reader)
    {
        m_reader->stop();
    }

    if (m_thread)
    {
        m_thread->quit();
        m_thread->wait();
    }

    setError({});
    setProgress(0);
    setWordsCount(0);
    setState(State::Stopped);

    m_model.reset();
}

void Controller::setState(State value)
{
    if (m_state != value)
    {
        m_state = value;
        emit stateChanged();
    }

    if (m_state != State::Running)
    {
        setWordsPerSec(0);
    }
}

void Controller::setError(QString value)
{
    if (m_error != value)
    {
        m_error = value;
        emit errorChanged();
    }
}

void Controller::setProgress(float value)
{
    if (!qFuzzyCompare(m_progress, value))
    {
        m_progress = value;
        emit progressChanged();
    }
}

void Controller::setWordsCount(int value)
{
    if (m_wordsCount != value)
    {
        m_wordsCount = value;
        emit wordsCountChanged();
    }
}

void Controller::setWordsPerSec(int value)
{
    if (m_wordsPerSec != value)
    {
        m_wordsPerSec = value;
        emit wordsPerSecChanged();
    }
}
