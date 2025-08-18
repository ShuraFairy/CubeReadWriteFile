#pragma once

#include <QObject>
#include <QByteArray>
#include <QElapsedTimer>

class FileWorker : public QObject
{
    Q_OBJECT

public:
    explicit FileWorker(QObject *parent = nullptr);
    
    qint64 getLastOperationTime() const;

public slots:
    void readFile(const QString &filePath);
    void saveFile(const QString &filePath, const QByteArray &data);
    void cancelOperation();

signals:
    void readProgress(qint64 bytesRead, qint64 totalBytes);
    void readFinished(const QByteArray &data);
    void readError(const QString &error);

    void startRead(const bool start);
    void stoptRead(const bool start);
    void startWrite(const bool start);
    void stopWrite(const bool start);
    void setRotationDirection(const bool direction);
    
    void saveProgress(qint64 bytesWritten, qint64 totalBytes);
    void saveFinished();
    void saveError(const QString &error);

    void cancelOperation_();

private:
    QElapsedTimer m_timer;
    qint64 m_lastOperationTime;
    bool m_stop;
    bool m_start;
};


