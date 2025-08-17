#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    app.setApplicationName("File Processor");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Alex Petrov Company");
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
