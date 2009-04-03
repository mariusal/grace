#ifndef __MAINWINDOW_H_
#define __MAINWINDOW_H_

#include <QMainWindow>
#include "ui_mainwindow.h"

class MainWindow : public QMainWindow
{
   Q_OBJECT

public:
  MainWindow(QMainWindow *parent = 0);

  Ui::MainWindow ui;

protected:
  void closeEvent(QCloseEvent*);

};

#endif /* __MAINWINDOW_H_ */

