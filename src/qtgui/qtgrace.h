#ifndef __QTGRACE_H_
#define __QTGRACE_H_

#include <QObject>
#include <QApplication>
#include <QItemDelegate>
#include <QEvent>
#include <QKeyEvent>

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
    void (*callback3)(int index, void *);
    void setCallBack(void (*callback)(int index, void *),
                     void *data) {
        this->callback3 = callback;
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
    void table_label_activate_cb_proc(int index) {
        callback3(index, data);
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
protected:
    bool eventFilter(QObject *object, QEvent *event)
    {
        QWidget *editor = qobject_cast<QWidget*>(object);
        if (!editor)
            return false;
        if (event->type() == QEvent::KeyPress) {
            switch (static_cast<QKeyEvent *>(event)->key()) {
            case Qt::Key_Tab:
                emit commitData(editor);
                emit closeEditor(editor);
                return true;
            case Qt::Key_Backtab:
                emit commitData(editor);
                emit closeEditor(editor);
                return true;
            case Qt::Key_Enter:
            case Qt::Key_Return:
                printf("%s", "editor enter key\n");
                QMetaObject::invokeMethod(this, "_q_commitDataAndCloseEditor",
                                          Qt::QueuedConnection, Q_ARG(QWidget*, editor));
                return false;
            case Qt::Key_Escape:
                // don't commit data
                emit closeEditor(editor, QAbstractItemDelegate::RevertModelCache);
                break;
            default:
                return false;
            }
            if (editor->parentWidget())
                editor->parentWidget()->setFocus();
            return true;
        } else if (event->type() == QEvent::FocusOut || (event->type() == QEvent::Hide && editor->isWindow())) {
            //the Hide event will take care of he editors that are in fact complete dialogs
            if (!editor->isActiveWindow() || (QApplication::focusWidget() != editor)) {
                QWidget *w = QApplication::focusWidget();
                while (w) { // don't worry about focus changes internally in the editor
                    if (w == editor)
                        return false;
                    w = w->parentWidget();
                }

                emit commitData(editor);
                emit closeEditor(editor, NoHint);
            }
        } else if (event->type() == QEvent::ShortcutOverride) {
            if (static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape) {
                event->accept();
                return true;
            }
        }
        return false;
    }

private:
    int maxLength;
};

#endif /* __QTGRACE_H_ */

