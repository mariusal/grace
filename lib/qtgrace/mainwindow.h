#ifndef __MAINWINDOW_H_
#define __MAINWINDOW_H_

#include <QMainWindow>

#include "ui_mainwindow.h"
#include "canvaswidget.h"

class MainWindow : public QMainWindow
{
   Q_OBJECT

public:
  MainWindow(QMainWindow *parent = 0);
  ~MainWindow();

protected:
  void closeEvent(QCloseEvent*);

private slots:
  void on_actionExit_triggered();
  void on_actionOpen_triggered();
  void readSettings();
  void writeSettings();
  bool maybeSave();
  void loadFile(const QString &fileName);

private:
  void setCurrentFile(const QString &fileName);
  QString strippedName(const QString &fullFileName);

  Ui::MainWindow ui;
  CanvasWidget *canvasWidget;
  
  QString curFile;
};

#endif /* __MAINWINDOW_H_ */

