#ifndef __FILESELECTIONDIALOG_H
#define __FILESELECTIONDIALOG_H

#include <QDialog>
#include "ui_fileselectiondialog.h"

class FileSelectionDialog : public QDialog
{
   Q_OBJECT

public:
  FileSelectionDialog(QWidget *parent = 0);

  Ui::FileSelectionDialog ui;

protected:
  void closeEvent(QCloseEvent*);

};

#endif /* __FILESELECTIONDIALOG_H_ */

