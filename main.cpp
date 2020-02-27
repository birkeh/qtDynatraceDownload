#include "cmainwindow.h"

#include <QApplication>
#include <QSettings>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationDisplayName("qtDynatraceDownload");
    a.setOrganizationName("WIN-DESIGN");
    a.setOrganizationDomain("windesign.at");
    a.setApplicationName("qtDynatraceDownload");

    QSettings		settings;

    cMainWindow w;

    if(settings.value("main/maximized").toBool())
        w.showMaximized();
    else
        w.show();

    return a.exec();
}
