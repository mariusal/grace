#include <QMainWindow>

#include "ui_mainwindow.h"

class MainWindow : public QMainWindow
{
   Q_OBJECT

public:
   MainWindow(QMainWindow *parent = 0);

private:
   Ui::MainWindow ui;
};

