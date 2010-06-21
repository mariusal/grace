#ifndef __FILESELECTIONDIALOG_H
#define __FILESELECTIONDIALOG_H

#include <QDialog>
#include "ui_fileselectiondialog.h"

#include <QFileSystemModel>

class FileSelectionDialog : public QDialog
{
  Q_OBJECT

public:
  FileSelectionDialog(QWidget *parent = 0);

  Ui::FileSelectionDialog ui;

  QFileSystemModel *dirModel;
  QFileSystemModel *fileModel;

private slots:
  void dirDoubleClicked(const QModelIndex);

protected:
  void closeEvent(QCloseEvent*);

};

#endif /* __FILESELECTIONDIALOG_H_ */

