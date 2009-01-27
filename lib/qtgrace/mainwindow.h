#ifndef __MAINWINDOW_H_
#define __MAINWINDOW_H_

#include <QMainWindow>

#include "ui_mainwindow.h"
#include "canvaswidget.h"
extern "C" {
#include <graceapp.h>
}

class MainWindow : public QMainWindow
{
   Q_OBJECT

public:
  MainWindow(GraceApp *gapp, QMainWindow *parent = 0);
  ~MainWindow();

protected:
  void closeEvent(QCloseEvent*);

private slots:
  void on_actionExit_triggered();
  void on_actionOpen_triggered();
  void page_zoom_inout(GraceApp *gapp, int inout);
  void on_actionSmaller_triggered();
  void on_actionLarger_triggered();
  void on_actionOriginalSize_triggered();

  void readSettings();
  void writeSettings();
  bool maybeSave();
  void loadFile(const QString &fileName);

private:
  GraceApp *gapp;

  void setCurrentFile(const QString &fileName);
  QString strippedName(const QString &fullFileName);
  void setToolBarIcons();

  Ui::MainWindow ui;
  CanvasWidget *canvasWidget;
  
  QString curFile;
};

#endif /* __MAINWINDOW_H_ */

