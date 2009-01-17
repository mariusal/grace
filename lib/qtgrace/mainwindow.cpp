#include <QPainter>
#include <QFont>

#include "mainwindow.h"
#include <QCloseEvent>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QTextStream>

MainWindow::MainWindow(QMainWindow *parent) : QMainWindow(parent)
{
  ui.setupUi(this);
  canvasWidget = ui.widget;

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

  /* Allocate Grace object */
  grace = grace_new("");
  if (!grace) {
    exit(1);
  } 

  QByteArray bytes = fileName.toAscii();
  const char *ptr = bytes.data();

  /* Open input stream from a project file */
  grf = grfile_openr(ptr);
  if (!grf) {
    errmsg("Can't open input for reading");
    exit(1);
  }

  /* Parse & load the project */
  gp = gproject_load(grace, grf, AMEM_MODEL_SIMPLE);
  if (!gp) {
    errmsg("Failed parsing project");
    exit(1);
  }

  /* Free the stream */
  grfile_free(grf); 

  /* Sync device dimensions with the plot page size */
  grace_sync_canvas_devices(gp);

  /* Assign the output stream */
  //canvas_set_prstream(grace_get_canvas(gapp->grace), &xstream);
  //canvas_set_prstream(grace_get_canvas(grace), fpout);
  canvasWidget->draw(grace, gp);

  /* Switch to the hardcopy device */
//  select_device(canvas, hdevice);

  /* Do the actual rendering */
//  gproject_render(gp);

  /* Free the GProject object */
//  gproject_free(gp);

  /* Free the Grace object (or, it could be re-used for other projects) */
//  grace_free(grace);

//  exit(0); 

  //QTextStream in(&file);
  QApplication::setOverrideCursor(Qt::WaitCursor);
  //textEdit->setPlainText(in.readAll());
  QApplication::restoreOverrideCursor();

  setCurrentFile(fileName);
  statusBar()->showMessage(tr("File loaded"), 2000);
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

