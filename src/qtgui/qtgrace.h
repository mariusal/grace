#ifndef __QTGRACE_H_
#define __QTGRACE_H_

#include <QObject>
#include <QApplication>
#include <QItemDelegate>
#include <QHeaderView>
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
//            //the Hide event will take care of he editors that are in fact complete dialogs
//            if (!editor->isActiveWindow() || (QApplication::focusWidget() != editor)) {
//                QWidget *w = QApplication::focusWidget();
//                while (w) { // don't worry about focus changes internally in the editor
//                    if (w == editor)
//                        return false;
//                    w = w->parentWidget();
//                }

//                emit commitData(editor);
//                emit closeEditor(editor, NoHint);
//            }
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

class HeaderView : public QHeaderView
{
    Q_OBJECT

public:
    HeaderView(Qt::Orientation orientation, QWidget *parent = 0)
        : QHeaderView(orientation, parent)
    {
        cbdata = 0;
    }

    void setCallBackData(Table_CBData *cbdata) {
        this->cbdata = cbdata;
    }

protected:
    bool event(QEvent *e);
    void mousePressEvent(QMouseEvent *e) {e->ignore();}
    void mouseReleaseEvent(QMouseEvent *e) {e->ignore();}
    void mouseDoubleClickEvent(QMouseEvent *e) {e->ignore();}

private:
     Table_CBData *cbdata;

};


#include <QAbstractTableModel>

class TableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    TableModel(QObject *parent = 0);

    void setRowCount(int rows);
    void setColumnCount(int cols);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole);

    void setDefaultColumnAlignment(Qt::Alignment align);
    void setDefaultColumnLabelAlignment(Qt::Alignment align);
    void setRowLabels(char **labels);
    void setColumnLabels(char **labels);
    void setDrawCellCallback(Table_CBData *cbdata);

private:
    int ncols;
    int nrows;
    Qt::Alignment defaultColumnAlignment;
    Qt::Alignment defaultColumnLabelAlignment;
    QStringList rowLabels;
    QStringList columnLabels;
    Table_CBData *cbdata;
};


#include <QTableView>

class TableView : public QTableView
{
    Q_OBJECT

public:
    TableView(QWidget *parent = 0);

};

#endif /* __QTGRACE_H_ */

