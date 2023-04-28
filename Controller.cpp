#include "Controller.h"

#include <QThread>

#include "Reader.h"

Controller::Controller(QObject* parent)
    : QObject(parent)
{
}

Controller::~Controller()
{
    stop();

    if (m_thread)
    {
        m_thread->quit();
        m_thread->wait();
    }
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

    auto reader = new Reader;
    auto thread = new QThread(this);
    reader->moveToThread(thread);

    connect(thread, &QThread::started, reader, [this, reader]()
    {
        reader->start(m_file.toLocalFile());
    });

    connect(thread, &QThread::finished, this, [thread, reader]()
    {
        thread->deleteLater();
        reader->deleteLater();
    });

    connect(reader, &Reader::dataChanged, this, [this, reader](const ReaderData& data)
    {
        reader->notifyDataReceived();

        if (m_state == State::Running)
        {
            setProgress(data.totalProgress);
            setWordsPerSec(data.wordsPerSec);
        }

        if (m_state != State::Stopped)
        {
            m_model.handle(data.word, data.wordCount);
        }
    });

    connect(reader, &Reader::finished, this, [this, thread](const QString& error)
    {
        setError(error);
        setState(State::Stopped);
        thread->quit();
    });

    setState(State::Running);

    m_reader = reader;
    m_thread = thread;

    m_thread->start();
}

void Controller::stop()
{
    setError({});
    setProgress(0.);
    setState(State::Stopped);

    if (m_reader)
    {
        m_reader->stop();
    }

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

void Controller::setWordsPerSec(int value)
{
    if (m_wordsPerSec != value)
    {
        m_wordsPerSec = value;
        emit wordsPerSecChanged();
    }
}
