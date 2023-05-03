#pragma once

#include <QHash>
#include <QMutex>
#include <QString>
#include <QObject>
#include <QAtomicInt>
#include <QSemaphore>
#include <QWaitCondition>

class Reader : public QObject
{
    Q_OBJECT

public:
    struct Data
    {
        QString word;
        int count{0};
        int wordsPerSec{0};
        float totalProgress{0};
        int totalWordsCount{0};
    };

    explicit Reader(QObject* parent = nullptr);
    ~Reader() override;

    void notifyDataReceived();
    QHash<QString, int> words();

    bool isPaused() const { return m_paused; }

    void start(const QString& filePath);
    void pause();
    void resume();
    void stop();

signals:
    void dataChanged(const Reader::Data& data, QPrivateSignal = {});
    void finished(const QString& error, QPrivateSignal = {});
    void paused();

private:
    QMutex m_dataMutex;
    QMutex m_pauseMutex;
    QSemaphore m_semaphore{1};
    QWaitCondition m_resumeCond;
    QHash<QString, int> m_words;
    QAtomicInteger<bool> m_active{false};
    QAtomicInteger<bool> m_paused{false};
};
