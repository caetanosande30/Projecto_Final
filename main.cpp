#include <QApplication>
#include <QFontDatabase>
#include "database.h"
#include "loginwindow.h"
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Identidade da aplicação
    app.setApplicationName("Caeta Stark");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("MaktStark");

    // Abrir base de dados
    if (!Database::instance().open()) {
        return -1;
    }

    // Login
    bool firstTime = !Database::instance().hasPassword();
    LoginWindow login(firstTime);
    if (login.exec() != QDialog::Accepted) {
        return 0;
    }

    // Janela principal
    MainWindow w;
    w.show();

    return app.exec();
}
