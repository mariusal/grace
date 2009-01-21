#include <QPainter>
#include <QFont>

#include "mainwindow.h"
#include <QCloseEvent>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QTextStream>
extern "C" {
#include <files.h>
}

MainWindow::MainWindow(GraceApp *gapp, QMainWindow *parent) : QMainWindow(parent)
{
    ui.setupUi(this);
    this->gapp = gapp;
    gapp->gui->inwin = TRUE; // TODO: reimplement startup_gui(gapp) function here

    canvasWidget = ui.widget;
    canvasWidget->qtdrawgraph(gapp->gp);

    readSettings();

    setCurrentFile("");
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  if (maybeSave()) {
    writeSettings();
    event->accept();
  } else {
    event->ignore();
  }
}

void MainWindow::on_actionExit_triggered()
{
  close();
}

void MainWindow::on_actionOpen_triggered()
{
  if (maybeSave()) {
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
      loadFile(fileName);
  }
}

void MainWindow::readSettings()
{
  QSettings settings("Grace Project", "Grace");
  QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
  QSize size = settings.value("size", QSize(400, 400)).toSize();
  resize(size);
  move(pos);
}

void MainWindow::writeSettings()
{
  QSettings settings("Grace Project", "Grace");
  settings.setValue("pos", pos());
  settings.setValue("size", size());
}

bool MainWindow::maybeSave()
{
  //if (textEdit->document()->isModified()) {
  if (false) {
    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, tr("Grace"),
	tr("The document has been modified.\n"
	  "Do you want to save your changes?"),
	QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
      //return save();
      return true;
    else if (ret == QMessageBox::Cancel)
      return false;
  }
  return true;
}

void MainWindow::loadFile(const QString &fileName)
{
/*  QFile file(fileName);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    QMessageBox::warning(this, tr("Grace"),
	tr("Cannot read file %1:\n%2.")
	.arg(fileName)
	.arg(file.errorString()));
    return;
  }*/

    QByteArray bytes = fileName.toAscii();
    char *filename = bytes.data();

    if (load_project(gapp, filename) == RETURN_SUCCESS) {
	QApplication::setOverrideCursor(Qt::WaitCursor);
	canvasWidget->qtdrawgraph(gapp->gp);
	//canvasWidget->update_all();
	QApplication::restoreOverrideCursor();
	setCurrentFile(fileName);
	statusBar()->showMessage(tr("File loaded"), 2000);
    } else {
	statusBar()->showMessage(tr("File failed to load"), 2000);
    }

  //canvasWidget->draw(fileName);

  //QTextStream in(&file);
  //textEdit->setPlainText(in.readAll());

}

void MainWindow::setCurrentFile(const QString &fileName)
{
  curFile = fileName;
  //textEdit->document()->setModified(false);
  setWindowModified(false);

  QString shownName;
  if (curFile.isEmpty())
    shownName = "untitled.txt";
  else
    shownName = strippedName(curFile);

  setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr("Grace")));
}

QString MainWindow::strippedName(const QString &fullFileName)
{
  return QFileInfo(fullFileName).fileName();
}

