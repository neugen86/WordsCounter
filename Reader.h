#pragma once

#include <QHash>
#include <QString>
#include <QObject>
#include <QMutex>
#include <QAtomicInt>
#include <QSemaphore>
#include <QWaitCondition>

struct ReaderData
{
    QString word;
    int wordCount = 0;
    int wordsPerSec = 0;
    float totalProgress = 0;

};
Q_DECLARE_METATYPE(ReaderData)


class Reader : public QObject
{
    Q_OBJECT

    QMutex m_mutex;
    QWaitCondition m_cond;
    QSemaphore m_semaphore{1};
    QHash<QString, int> m_words;
    QAtomicInteger<bool> m_active{false};

public:
    explicit Reader(QObject* parent = nullptr);
    ~Reader() override;

    void notifyDataReceived();

    void start(const QString& filePath);
    void pause();
    void resume();
    void stop();

signals:
    void dataChanged(const ReaderData& data);
    void finished(const QString& error = {});

};
