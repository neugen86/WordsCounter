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
    int count{0};
    int totalCount{0};
    int wordsPerSec{0};
    float totalProgress{0};

};


class Reader : public QObject
{
    Q_OBJECT

    QMutex m_dataMutex;
    QMutex m_pauseMutex;
    QSemaphore m_semaphore{1};
    QWaitCondition m_resumeCond;
    QHash<QString, int> m_words;
    QAtomicInteger<bool> m_active{false};
    QAtomicInteger<bool> m_paused{false};

public:
    explicit Reader(QObject* parent = nullptr);
    ~Reader() override;

    void notifyDataReceived();
    QHash<QString, int> words();

    void start(const QString& filePath);
    void pause();
    void resume();
    void stop();

signals:
    void dataChanged(const ReaderData& data);
    void finished(const QString& error = {});

};