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

    void *data;

    Widget sender;
    void (*callback)(Widget, XtPointer, XtPointer);
    void setCallBack(const QObject *sender,
                     void (*callback)(Widget, XtPointer, XtPointer),
                     void *data) {
        this->sender = (Widget) sender;
        this->callback = callback;
        this->data = data;
    }

    void (*callback1)(const QModelIndex &current, const QModelIndex &previous, void *);
    void setCallBack(void (*callback)(const QModelIndex &current, const QModelIndex &previous, void *),
                     void *data) {
        this->callback1 = callback;
        this->data = data;
    }

    void (*callback2)(const QModelIndex &index, void *);
    void setCallBack(void (*callback)(const QModelIndex &index, void *),
                     void *data) {
        this->callback2 = callback;
        this->data = data;
    }

public slots:
    void callBack() {
        callback(sender, (XtPointer) data, NULL);
    }
    void table_int_cell_cb_proc(const QModelIndex &current, const QModelIndex &previous) {
        callback1(current, previous, data);
    }
    void table_int_cell_cb_proc(const QModelIndex &index) {
        callback2(index, data);
    }
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

