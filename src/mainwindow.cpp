#include "mainwindow.h"
#include "glwidget.h"
#include "fileworker.h"
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QTextEdit>
#include <QLabel>
#include <QFileDialog>
#include <QFileInfo>
#include <QDateTime>
#include <QMessageBox>
#include <QThread>
#include <QMimeDatabase>
#include <QMimeType>
#include <QCryptographicHash>
#include <QTextCodec>
#include <QKeyEvent>
#include <QGroupBox>
#include <QWidget>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_fileLoaded(false)
{
    setupUI();

    m_workerThread = new QThread(this);
    m_fileWorker = new FileWorker();
    m_fileWorker->moveToThread(m_workerThread);
    
    // Connect signals for reading
    connect(m_workerThread, &QThread::finished, m_fileWorker, &QObject::deleteLater);
    connect(m_fileWorker, &FileWorker::readProgress, this, &MainWindow::onReadProgress);
    connect(m_fileWorker, &FileWorker::readFinished, this, &MainWindow::onReadFinished);
    connect(m_fileWorker, &FileWorker::readError, this, &MainWindow::onReadError);
    
    // Connect signals for saving
    connect(m_fileWorker, &FileWorker::saveProgress, this, &MainWindow::onSaveProgress);
    connect(m_fileWorker, &FileWorker::saveFinished, this, &MainWindow::onSaveFinished);
    connect(m_fileWorker, &FileWorker::saveError, this, &MainWindow::onSaveError);

    connect(m_fileWorker, &FileWorker::startRead, glWidget, &GLWidget::setRunning, Qt::QueuedConnection);
    connect(m_fileWorker, &FileWorker::stoptRead, glWidget, &GLWidget::setRunning, Qt::QueuedConnection);
    connect(m_fileWorker, &FileWorker::setRotationDirection, glWidget, &GLWidget::setRotationDirection, Qt::QueuedConnection);

    connect(m_fileWorker, &FileWorker::startWrite, glWidget, &GLWidget::setRunning, Qt::QueuedConnection);
    connect(m_fileWorker, &FileWorker::stopWrite, glWidget, &GLWidget::setRunning, Qt::QueuedConnection);
    connect(m_fileWorker, &FileWorker::setRotationDirection, glWidget, &GLWidget::setRotationDirection, Qt::QueuedConnection);

    connect(m_cancelButton, &QPushButton::clicked, m_fileWorker, &FileWorker::cancelOperation, Qt::QueuedConnection);

    connect(m_fileWorker, &FileWorker::cancelOperation_, this, &MainWindow::cancelOperation, Qt::QueuedConnection);

    m_workerThread->start();
}

MainWindow::~MainWindow()
{
    m_workerThread->quit();
    m_workerThread->wait();
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Source file selection
    m_sourcePathEdit = new QLineEdit(this);
    m_sourcePathEdit->setPlaceholderText("Select source file to read...");
    m_browseSourceButton = new QPushButton("Browse...", this);
    m_readButton = new QPushButton("Read File", this);
    m_readButton->setEnabled(false);
    
    // Destination file selection
    m_destinationPathEdit = new QLineEdit(this);
    m_destinationPathEdit->setPlaceholderText("Select destination file to save...");
    m_browseDestinationButton = new QPushButton("Browse...", this);
    m_saveButton = new QPushButton("Save File", this);
    m_saveButton->setEnabled(false);
    
    // Progress bar
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    m_progressBar->setAlignment(Qt::AlignCenter);
    
    // Status label
    m_statusLabel = new QLabel("Ready", this);
    m_statusLabel->setStyleSheet("QLabel { color: blue; font-weight: bold; }");    

    // Controls
    QGroupBox *controlsGroup = new QGroupBox("Controls", this);    
    controlsGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Speed control
    QLabel *speedLabel = new QLabel("Rotation Speed:", this);
    m_speedSlider = new QSlider(Qt::Horizontal, this);
    m_speedSlider->setRange(0, 100);
    m_speedSlider->setValue(50);
    m_speedSlider->setToolTip("Adjust rotation speed");

    QLabel *minSpeedLabel = new QLabel("Slow", this);
    QLabel *maxSpeedLabel = new QLabel("Fast", this);
    QHBoxLayout *speedLayout = new QHBoxLayout;
    speedLayout->addWidget(minSpeedLabel);
    speedLayout->addWidget(m_speedSlider);
    speedLayout->addWidget(maxSpeedLabel);    

    m_cancelButton = new QPushButton("Cancel Operation", this);
    m_cancelButton->setToolTip("Cancel operation");

    // Layout for controls
    QHBoxLayout *controlsLayout = new QHBoxLayout(controlsGroup);    
    controlsLayout->addWidget(speedLabel);
    controlsLayout->addLayout(speedLayout);
    controlsLayout->addStretch();
    controlsLayout->addWidget(m_cancelButton);    

    // File information display
    QLabel *infoLabel = new QLabel("File Information:", this);
    infoLabel->setStyleSheet("QLabel { font-weight: bold; }");    

    m_infoTextEdit = new QTextEdit(this);
    m_infoTextEdit->setReadOnly(true);    
    
    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // Source file selection row
    QHBoxLayout *sourceLayout = new QHBoxLayout;
    sourceLayout->addWidget(new QLabel("Source:", this));
    sourceLayout->addWidget(m_sourcePathEdit, 1);
    sourceLayout->addWidget(m_browseSourceButton);
    sourceLayout->addWidget(m_readButton);
    mainLayout->addLayout(sourceLayout);
    
    // Destination file selection row
    QHBoxLayout *destLayout = new QHBoxLayout;
    destLayout->addWidget(new QLabel("Destination:", this));
    destLayout->addWidget(m_destinationPathEdit, 1);
    destLayout->addWidget(m_browseDestinationButton);
    destLayout->addWidget(m_saveButton);
    mainLayout->addLayout(destLayout);
    
    // Progress bar
    mainLayout->addWidget(m_progressBar);
    
    // Status
    mainLayout->addWidget(m_statusLabel);
    
    // File information
    mainLayout->addWidget(infoLabel);
    mainLayout->addWidget(m_infoTextEdit);

    // Rotate
    mainLayout->addWidget(controlsGroup);
            
    openglGroup = new QGroupBox(tr("Cube (rotation around axes and color change)"));

    glWidget = new GLWidget(this);

    xSlider = createSlider();
    ySlider = createSlider();
    zSlider = createSlider();

    QLabel *labelX = new QLabel("X");
    QLabel *valueX = new QLabel("0");
    QLabel *labelY = new QLabel("Y");
    QLabel *valueY = new QLabel("0");
    QLabel *labelZ = new QLabel("Z");
    QLabel *valueZ = new QLabel("0");

    QLabel *labelR = new QLabel("R");
    QLabel *valueR = new QLabel("128");
    QLabel *labelG = new QLabel("G");
    QLabel *valueG = new QLabel("128");
    QLabel *labelB = new QLabel("B");
    QLabel *valueB = new QLabel("128");

    rSlider = new QSlider(Qt::Vertical);
    rSlider->setRange(0, 255);
    gSlider = new QSlider(Qt::Vertical);
    gSlider->setRange(0, 255);
    bSlider = new QSlider(Qt::Vertical);
    bSlider->setRange(0, 255);    

    connect(xSlider, &QSlider::valueChanged, glWidget, &GLWidget::setXRotation);
    connect(glWidget, &GLWidget::xRotationChanged, xSlider, &QSlider::setValue);
    connect(ySlider, &QSlider::valueChanged, glWidget, &GLWidget::setYRotation);
    connect(glWidget, &GLWidget::yRotationChanged, ySlider, &QSlider::setValue);
    connect(zSlider, &QSlider::valueChanged, glWidget, &GLWidget::setZRotation);
    connect(glWidget, &GLWidget::zRotationChanged, zSlider, &QSlider::setValue);

    connect(rSlider, &QSlider::valueChanged, glWidget, &GLWidget::setRed);
    connect(glWidget, &GLWidget::rChanged, rSlider, &QSlider::setValue);

    connect(gSlider, &QSlider::valueChanged, glWidget, &GLWidget::setGreen);
    connect(glWidget, &GLWidget::gChanged, gSlider, &QSlider::setValue);

    connect(bSlider, &QSlider::valueChanged, glWidget, &GLWidget::setBlue);
    connect(glWidget, &GLWidget::bChanged, bSlider, &QSlider::setValue);

    QObject::connect(xSlider, SIGNAL(valueChanged(int)), valueX, SLOT(setNum(int)));
    QObject::connect(ySlider, SIGNAL(valueChanged(int)), valueY, SLOT(setNum(int)));
    QObject::connect(zSlider, SIGNAL(valueChanged(int)), valueZ, SLOT(setNum(int)));
    QObject::connect(rSlider, SIGNAL(valueChanged(int)), valueR, SLOT(setNum(int)));
    QObject::connect(gSlider, SIGNAL(valueChanged(int)), valueG, SLOT(setNum(int)));
    QObject::connect(bSlider, SIGNAL(valueChanged(int)), valueB, SLOT(setNum(int)));    

    QWidget *w = new QWidget;    
    QGridLayout *container = new QGridLayout(w);

    container->addWidget(glWidget, 0, 0);

    container->addWidget(xSlider, 0, 1);
    container->addWidget(labelX, 1, 1);
    container->addWidget(valueX, 2, 1);

    container->addWidget(ySlider, 0, 2);
    container->addWidget(labelY, 1, 2);
    container->addWidget(valueY, 2, 2);

    container->addWidget(zSlider, 0, 3);
    container->addWidget(labelZ, 1, 3);
    container->addWidget(valueZ, 2, 3);

    container->addWidget(rSlider, 0, 4);
    container->addWidget(labelR, 1, 4);
    container->addWidget(valueR, 2, 4);

    container->addWidget(gSlider, 0, 5);
    container->addWidget(labelG, 1, 5);
    container->addWidget(valueG, 2, 5);

    container->addWidget(bSlider, 0, 6);
    container->addWidget(labelB, 1, 6);
    container->addWidget(valueB, 2, 6);

    QVBoxLayout *mainLayout_ = new QVBoxLayout(this);

    openglGroup->setLayout(container);
    mainLayout_->addWidget(w);

    mainLayout->addWidget(openglGroup);    
    setLayout(mainLayout);//

    xSlider->setValue(0);//15 * 16);
    ySlider->setValue(0);//345 * 16);
    zSlider->setValue(0 * 16);
    rSlider->setValue(128);
    gSlider->setValue(128);
    bSlider->setValue(128);    

    connect(m_browseSourceButton, &QPushButton::clicked, this, &MainWindow::selectSourceFile);
    connect(m_browseDestinationButton, &QPushButton::clicked, this, &MainWindow::selectDestinationFile);
    connect(m_readButton, &QPushButton::clicked, this, &MainWindow::readFile);
    connect(m_saveButton, &QPushButton::clicked, this, &MainWindow::saveFile);
    
    connect(m_sourcePathEdit, &QLineEdit::textChanged, [this](const QString &text) {
        m_readButton->setEnabled(!text.isEmpty());
        if (!text.isEmpty()) {
            updateFileInfo(text);
        }
    });
    
    connect(m_destinationPathEdit, &QLineEdit::textChanged, [this](const QString &text) {
        m_saveButton->setEnabled(!text.isEmpty() && m_fileLoaded);
    });

    connect(m_speedSlider, &QSlider::valueChanged, glWidget, &GLWidget::setRotationSpeed);

    setWindowTitle("File Processor");
    resize(800, 800);
}

void MainWindow::selectSourceFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select Source File to Read");
    if (!fileName.isEmpty()) {
        m_sourcePathEdit->setText(fileName);
        m_currentSourcePath = fileName;
        updateFileInfo(fileName);
    }
}

void MainWindow::selectDestinationFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Select Destination File", 
                                                   m_destinationPathEdit->text());
    if (!fileName.isEmpty()) {
        m_destinationPathEdit->setText(fileName);
        m_currentDestinationPath = fileName;
    }
}

void MainWindow::readFile()
{
    if (m_currentSourcePath.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select a source file first.");
        return;
    }
    
    QFileInfo fileInfo(m_currentSourcePath);
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, "Error", "Selected source file does not exist.");
        return;
    }
    
    if (!fileInfo.isFile()) {
        QMessageBox::warning(this, "Error", "Selected path is not a file.");
        return;
    }    

    resetUI();
    m_progressBar->setVisible(true);
    m_progressBar->setFormat("Reading: %p%");
    m_statusLabel->setText("Reading file...");
    m_readButton->setEnabled(false);
    m_browseSourceButton->setEnabled(false);    

    QMetaObject::invokeMethod(m_fileWorker, "readFile", Qt::QueuedConnection,
                             Q_ARG(QString, m_currentSourcePath));
}

void MainWindow::saveFile()
{
    if (m_currentDestinationPath.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select a destination file first.");
        return;
    }
    
    if (!m_fileLoaded) {
        QMessageBox::warning(this, "Error", "No file data loaded. Please read a file first.");
        return;
    }    

    m_progressBar->setVisible(true);
    m_progressBar->setFormat("Saving: %p%");
    m_statusLabel->setText("Saving file...");
    m_saveButton->setEnabled(false);
    m_browseDestinationButton->setEnabled(false);    

    QMetaObject::invokeMethod(m_fileWorker, "saveFile", Qt::QueuedConnection,
                             Q_ARG(QString, m_currentDestinationPath),
                             Q_ARG(QByteArray, m_fileData));
}

void MainWindow::onReadProgress(qint64 bytesRead, qint64 totalBytes)
{
    if (totalBytes > 0) {
        int percentage = static_cast<int>((bytesRead * 100) / totalBytes);
        m_progressBar->setValue(percentage);
        m_progressBar->setFormat(QString("Reading: %p% (%1 / %2)")
                                .arg(formatFileSize(bytesRead))
                                .arg(formatFileSize(totalBytes)));
    }
}

void MainWindow::onReadFinished(const QByteArray &data)
{
    m_fileData = data;
    m_fileLoaded = true;
    
    m_progressBar->setVisible(false);
    m_statusLabel->setText(QString("File read successfully! Size: %1").arg(formatFileSize(data.size())));
    m_readButton->setEnabled(true);
    m_browseSourceButton->setEnabled(true);
    m_saveButton->setEnabled(!m_destinationPathEdit->text().isEmpty());    

    QString currentInfo = m_infoTextEdit->toPlainText();
    currentInfo += QString("\nRead completed in: %1 ms").arg(m_fileWorker->getLastOperationTime());
    m_infoTextEdit->setPlainText(currentInfo);
}

void MainWindow::onReadError(const QString &error)
{
    m_progressBar->setVisible(false);
    m_statusLabel->setText("Error reading file");
    m_statusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    m_readButton->setEnabled(true);
    m_browseSourceButton->setEnabled(true);
    
    QMessageBox::critical(this, "Read Error", error);
}

void MainWindow::onSaveProgress(qint64 bytesWritten, qint64 totalBytes)
{
    if (totalBytes > 0) {
        int percentage = static_cast<int>((bytesWritten * 100) / totalBytes);
        m_progressBar->setValue(percentage);
        m_progressBar->setFormat(QString("Saving: %p% (%1 / %2)")
                                .arg(formatFileSize(bytesWritten))
                                .arg(formatFileSize(totalBytes)));
    }
}

void MainWindow::onSaveFinished()
{
    m_progressBar->setVisible(false);
    m_statusLabel->setText("File saved successfully!");
    m_statusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
    m_saveButton->setEnabled(true);
    m_browseDestinationButton->setEnabled(true);
    
    QMessageBox::information(this, "Success", "File saved successfully!");
}

void MainWindow::onSaveError(const QString &error)
{
    m_progressBar->setVisible(false);
    m_statusLabel->setText("Error saving file");
    m_statusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    m_saveButton->setEnabled(true);
    m_browseDestinationButton->setEnabled(true);
    
    QMessageBox::critical(this, "Save Error", error);
}

void MainWindow::updateFileInfo(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        m_infoTextEdit->setPlainText("File does not exist.");
        return;
    }    

    QString info;
    info += QString("Name: %1\n").arg(fileInfo.fileName());
    info += QString("Path: %1\n").arg(fileInfo.absolutePath());
    info += QString("Size: %1 (%2 bytes)\n").arg(formatFileSize(fileInfo.size())).arg(fileInfo.size());
    info += QString("Type: %1\n").arg(getFileType(fileInfo.fileName()));
    info += QString("Created: %1\n").arg(fileInfo.birthTime().toString("yyyy-MM-dd hh:mm:ss"));
    info += QString("Modified: %1\n").arg(fileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
    info += QString("Readable: %1\n").arg(fileInfo.isReadable() ? "Yes" : "No");
    info += QString("Writable: %1\n").arg(fileInfo.isWritable() ? "Yes" : "No");
    info += QString("Executable: %1\n").arg(fileInfo.isExecutable() ? "Yes" : "No");    

    QMimeDatabase mimeDatabase;
    QMimeType mimeType = mimeDatabase.mimeTypeForFile(filePath);
    info += QString("MIME Type: %1\n").arg(mimeType.name());
    if (!mimeType.comment().isEmpty()) {
        info += QString("Description: %1\n").arg(mimeType.comment());
    }
    
    m_infoTextEdit->setPlainText(info);
}

void MainWindow::cancelOperation()
{
    m_progressBar->setVisible(false);
    m_statusLabel->setText(QString("Operation canceled"));
    m_readButton->setEnabled(true);
    m_browseSourceButton->setEnabled(true);
    m_saveButton->setEnabled(!m_destinationPathEdit->text().isEmpty());
}

void MainWindow::resetUI()
{
    m_progressBar->setValue(0);
    m_statusLabel->setStyleSheet("QLabel { color: blue; font-weight: bold; }");    
}

QString MainWindow::formatFileSize(qint64 size) const
{
    if (size < 0) return "0 B";
    
    QStringList units = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double formattedSize = static_cast<double>(size);
    
    while (formattedSize >= 1024 && unitIndex < units.size() - 1) {
        formattedSize /= 1024;
        unitIndex++;
    }
    
    return QString("%1 %2").arg(formattedSize, 0, 'f', 2).arg(units[unitIndex]);
}

QString MainWindow::getFileType(const QString &fileName) const
{
    int dotIndex = fileName.lastIndexOf('.');
    if (dotIndex != -1 && dotIndex < fileName.length() - 1) {
        return fileName.mid(dotIndex + 1).toUpper() + " File";
    }
    return "File";
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if (isWindow() && e->key() == Qt::Key_Escape)
        close();
    else
        QWidget::keyPressEvent(e);
}

QSlider *MainWindow::createSlider()
{
    QSlider *slider = new QSlider(Qt::Vertical);
    slider->setRange(0, 360 * 16);
    slider->setSingleStep(16);
    slider->setPageStep(16);

    return slider;
}

