#include <QApplication>
#include <QSurfaceFormat>

#include "mainwindow.h"

int main(int argc, char* argv[])
{
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    app.setApplicationName("EquationLearner");
    app.setOrganizationName("EquationLearner");

    MainWindow window;
    window.show();
    return app.exec();
}
