#include "mainwindow.h"

MainWindow::MainWindow(QMainWindow *parent) : QMainWindow(parent)
{
   ui.setupUi(this);
}

void MainWindow::on_actionExit_triggered()
{
  close();
}

