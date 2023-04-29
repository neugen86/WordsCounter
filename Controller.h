#pragma once

#include <qqml.h>
#include <QObject>
#include <QPointer>

#include "Model.h"

class Reader;
class QThread;

class Controller : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QAbstractListModel* model READ model CONSTANT)
    Q_PROPERTY(QString file READ file WRITE setFile NOTIFY fileChanged)

    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(float progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(int wordsCount READ wordsCount NOTIFY wordsCountChanged)
    Q_PROPERTY(int wordsPerSec READ wordsPerSec NOTIFY wordsPerSecChanged)

public:
    enum State
    {
        Running,
        Paused,
        Stopped
    };
    Q_ENUM(State)

    explicit Controller(QObject* parent = nullptr);
    ~Controller() override;

    QString file() const;
    void setFile(const QString& filePath);

    QAbstractListModel* model() { return &m_model; }

    State state() const { return m_state; }
    QString error() const { return m_error; }
    float progress() const { return m_progress; }
    int wordsCount() const { return m_wordsCount; }
    int wordsPerSec() const { return m_wordsPerSec; }

    Q_INVOKABLE void startPause();
    Q_INVOKABLE void stop();

private:
    void setState(State value);
    void setError(QString value);
    void setProgress(float value);
    void setWordsCount(int value);
    void setWordsPerSec(int value);

signals:
    void fileChanged(QPrivateSignal = {});
    void stateChanged(QPrivateSignal = {});
    void errorChanged(QPrivateSignal = {});
    void progressChanged(QPrivateSignal = {});
    void wordsCountChanged(QPrivateSignal = {});
    void wordsPerSecChanged(QPrivateSignal = {});

private:
    QString m_error;
    float m_progress{0};
    int m_wordsCount{0};
    int m_wordsPerSec{0};

    QUrl m_file;
    Model m_model;
    QPointer<Reader> m_reader;
    QPointer<QThread> m_thread;

    bool m_terminated{false};
    State m_state{State::Stopped};
};
