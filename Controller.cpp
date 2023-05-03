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

        for (QHash<QString, int>::const_iterator it = words.cbegin();
             !m_model.isFull() && it != words.cend(); ++it)
        {
            const QString& word = it.key();

            if (m_model.indexOf(word) == -1)
            {
                QMetaObject::invokeMethod(this, [this, word, count=it.value()]()
                {
                    m_model.update(word, count);
                },
                Qt::QueuedConnection);
            }
        }
    });
    startStop();
}

Controller::~Controller()
{
    cancel();
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

        cancel();
    }
}

void Controller::startStop()
{
    switch (m_state)
    {
    case State::Running:
        m_reader->pause();
        setState(State::Stopped);
        return;

    case State::Stopped:
        m_reader->resume();
        setState(State::Running);
        return;

    default:
        Q_ASSERT(m_state == State::Idle);
        break;
    }

    cancel();
    m_cancelled = false;

    m_reader = new Reader;
    m_thread = new QThread(this);
    m_reader->moveToThread(m_thread);

    connect(m_thread, &QThread::started,
            m_reader, [this]()
    {
        m_reader->start(m_file.toLocalFile());
    });

    connect(m_thread, &QThread::finished,
            m_thread, &QThread::deleteLater);

    connect(m_reader, &Reader::dataChanged,
            this, [this](const Reader::Data& data)
    {
        if (m_cancelled || !m_reader)
        {
            return;
        }

        m_reader->notifyDataReceived();

        if (m_state == State::Running)
        {
            setProgress(data.totalProgress);
            setWordsCount(data.totalWordsCount);
            setWordsPerSec(data.wordsPerSec);
        }

        m_model.update(data.word, data.count);
    });

    connect(m_reader, &Reader::finished,
            this, [this](const QString& error)
    {
        setError(error);
        setState(State::Idle);
    });

    setState(State::Running);

    m_thread->start();
}

void Controller::cancel()
{
    m_cancelled = true;

    m_model.reset();

    setError({});
    setProgress(0);
    setWordsCount(0);
    setState(State::Idle);

    if (m_reader)
    {
        m_reader->stop();
        m_reader->deleteLater();
        m_reader = nullptr;
    }

    if (m_thread)
    {
        m_thread->quit();
        m_thread->wait();
        m_thread = nullptr;
    }

    setError({});
    setProgress(0);
    setWordsCount(0);
    setState(State::Idle);
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
