#include <QMainWindow>

#include "ui_mainwindow.h"

class MainWindow : public QMainWindow
{
   Q_OBJECT

public:
   MainWindow(QMainWindow *parent = 0);

private slots:
  void on_actionExit_triggered();

private:
   Ui::MainWindow ui;
};

