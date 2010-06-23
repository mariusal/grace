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

  void setDirectory(QString);

private slots:
  void dirDoubleClicked(const QModelIndex);
  void showHidden(bool);
  void fileClicked(const QModelIndex index);
  void cdToDir(int value);
  void setAsCwd();

private:
  void reapplyFilter();
  void showDrives();

};

#endif /* __FILESELECTIONDIALOG_H_ */

