#include <QMainWindow>

#include "ui_mainwindow.h"

#include "grace/grace.h"

class MainWindow : public QMainWindow
{
   Q_OBJECT

public:
  MainWindow(QMainWindow *parent = 0);
  void drawGraph(const GProject *gp);

private slots:
  void on_actionExit_triggered();

private:
   Ui::MainWindow ui;
};

