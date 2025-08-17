#include "fileworker.h"
#include <QFile>
#include <QFileInfo>
#include <QApplication>

FileWorker::FileWorker(QObject *parent)
    : QObject(parent)
    , m_lastOperationTime(0)
    , m_stop(false)
    , m_start(false)
{
}

void FileWorker::readFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.exists()) {
        emit readError("File does not exist.");
        return;
    }
    
    QFileInfo fileInfo(filePath);
    qint64 fileSize = fileInfo.size();
    
    if (!file.open(QIODevice::ReadOnly)) {
        emit readError(QString("Cannot open file for reading: %1").arg(file.errorString()));
        return;
    }
    
    // Start timer
    m_timer.start();
    emit startRead(true);
    emit setRotationDirection(true);
    m_start = true;
    
    QByteArray data;
    const qint64 chunkSize = 64 * 1024; // 64 KB chunks
    qint64 totalBytesRead = 0;
    
    while (!file.atEnd()) {
        QByteArray chunk = file.read(chunkSize);
        if (chunk.isEmpty()) {
            if (file.error() != QFile::NoError) {
                emit readError(QString("Error reading file: %1").arg(file.errorString()));
                return;
            }
            break;
        }

        if(m_stop == true)
        {
            qDebug() << "read" << m_stop;
            m_stop = false;
            emit readFinished(data);
            emit stoptRead(false);
            emit cancelOperation_();
            return;
        }

        data.append(chunk);
        totalBytesRead += chunk.size();
        
        // Emit progress for every 5% or at the end
        if (fileSize > 0) {
            static qint64 lastProgressPercent = 0;
            qint64 currentPercent = (totalBytesRead * 100) / fileSize;
            if (currentPercent != lastProgressPercent || totalBytesRead == fileSize) {
                emit readProgress(totalBytesRead, fileSize);
                lastProgressPercent = currentPercent;
                QApplication::processEvents(); // Keep UI responsive                
            }            
        }        
    }
    
    file.close();
    
    // Record operation time
    m_lastOperationTime = m_timer.elapsed();
        
    emit readFinished(data);
    emit stoptRead(false);
    m_start = false;
}

void FileWorker::saveFile(const QString &filePath, const QByteArray &data)
{
    QFile file(filePath);
    qint64 totalBytes = data.size();
    
    if (!file.open(QIODevice::WriteOnly)) {
        emit saveError(QString("Cannot open file for writing: %1").arg(file.errorString()));
        return;
    }
    
    // Start timer
    m_timer.start();
    emit startWrite(true);
    emit setRotationDirection(false);
    m_start = true;
    
    const qint64 chunkSize = 64 * 1024; // 64 KB chunks
    qint64 totalBytesWritten = 0;
    
    while (totalBytesWritten < totalBytes) {
        qint64 bytesToWrite = qMin(chunkSize, totalBytes - totalBytesWritten);
        QByteArray chunk = data.mid(static_cast<int>(totalBytesWritten), static_cast<int>(bytesToWrite));
        
        qint64 bytesWritten = file.write(chunk);
        if (bytesWritten == -1) {
            file.close();
            emit saveError(QString("Error writing to file: %1").arg(file.errorString()));
            return;
        }

        if(m_stop == true)
        {
            qDebug() << "read" << m_stop;
            m_stop = false;
            emit readFinished(data);
            emit stoptRead(false);
            emit cancelOperation_();
            return;
        }
        
        totalBytesWritten += bytesWritten;
        
        // Emit progress for every 5% or at the end
        static qint64 lastProgressPercent = 0;
        qint64 currentPercent = (totalBytesWritten * 100) / totalBytes;
        if (currentPercent != lastProgressPercent || totalBytesWritten == totalBytes) {
            emit saveProgress(totalBytesWritten, totalBytes);
            lastProgressPercent = currentPercent;
            QApplication::processEvents(); // Keep UI responsive
        }
    }
    
    file.close();
    
    // Record operation time
    m_lastOperationTime = m_timer.elapsed();

    emit saveFinished();
    emit stopWrite(false);
    m_start = false;
}

void FileWorker::cancelOperation()
{    
    if(m_start == false)
        return;

    m_stop = true;
    qDebug() << m_stop;
}

qint64 FileWorker::getLastOperationTime() const
{
    return m_lastOperationTime;
}
