#ifndef __QTGRACE_H_
#define __QTGRACE_H_

#include <QObject>
#include <QItemDelegate>
extern "C" {
#include <config.h>
#include <motifinc.h>
}

class CallBack : public QObject
{
    Q_OBJECT

public:
    CallBack(QObject *parent = 0);

    Widget sender;
    void (*callback)(Widget, XtPointer, XtPointer);
    void *data;

    void setCallBack(const QObject *sender,
                     void (*callback)(Widget, XtPointer, XtPointer),
                     void *data) {
        this->sender = (Widget) sender;
        this->callback = callback;
        this->data = data;
    };

public slots:
    void callBack() {
        callback(sender, (XtPointer) data, NULL);
    };
};

class LineEditDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    LineEditDelegate(int maxLength = 0, QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option, const QModelIndex &index) const;
private:
    int maxLength;
};

#endif /* __QTGRACE_H_ */

