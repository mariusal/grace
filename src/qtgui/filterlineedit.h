#ifndef __FILTERLINEEDIT_H
#define __FILTERLINEEDIT_H

#include <QLineEdit>

class FilterLineEdit : public QLineEdit
{
  Q_OBJECT

public:
  FilterLineEdit(QWidget *parent = 0);

protected:
  void focusInEvent(QFocusEvent *);

};

#endif /* __FILTERLINEEDIT_H */

