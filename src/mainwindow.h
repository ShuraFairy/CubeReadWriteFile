#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QByteArray>

class GLWidget;
class QPushButton;
class QLineEdit;
class QProgressBar;
class QTextEdit;
class QLabel;
class QThread;
class FileWorker;
class QGroupBox;
class QSlider;
class QStandardItemModel;

// QT_BEGIN_NAMESPACE
// class QGroupBox;
// class QSlider;
// class QStandardItemModel;
// QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void selectSourceFile();
    void selectDestinationFile();
    void readFile();
    void saveFile();
    void onReadProgress(qint64 bytesRead, qint64 totalBytes);
    void onReadFinished(const QByteArray &data);
    void onReadError(const QString &error);
    void onSaveProgress(qint64 bytesWritten, qint64 totalBytes);
    void onSaveFinished();
    void onSaveError(const QString &error);
    void updateFileInfo(const QString &filePath);
    void cancelOperation();

signals:
    void startRead(bool start);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void setupUI();
    void resetUI();
    QString formatFileSize(qint64 size) const;
    QString getFileType(const QString &fileName) const;

    // UI Components
    QLineEdit *m_sourcePathEdit;
    QPushButton *m_browseSourceButton;
    QPushButton *m_readButton;
    
    QLineEdit *m_destinationPathEdit;
    QPushButton *m_browseDestinationButton;
    QPushButton *m_saveButton;
    
    QProgressBar *m_progressBar;
    QTextEdit *m_infoTextEdit;
    //QTextEdit *m_contentPreview;
    QLabel *m_statusLabel;
    
    // Worker components
    FileWorker *m_fileWorker;
    QThread *m_workerThread;
    QString m_currentSourcePath;
    QString m_currentDestinationPath;
    QByteArray m_fileData;
    bool m_fileLoaded;

    // For Cube and OpenGL components
    QSlider *createSlider();
    GLWidget *glWidget;
    QSlider *xSlider;
    QSlider *ySlider;
    QSlider *zSlider;
    QSlider *rSlider;
    QSlider *gSlider;
    QSlider *bSlider;    
    QGroupBox * openglGroup;

    // Controls components
    //QPushButton *m_startStopButton;
    //QPushButton *m_directionButton;
    QPushButton *m_cancelButton;
    QSlider *m_speedSlider;
    QLabel *m_statusLabelRotate;
};

#endif // MAINWINDOW_H
