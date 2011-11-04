#ifndef __QTGRACE_H_
#define __QTGRACE_H_

#include <QDebug>
#include <QObject>
#include <QApplication>
#include <QLineEdit>
#include <QHeaderView>
#include <QEvent>
#include <QKeyEvent>

extern "C" {
#include <config.h>
#include <motifinc.h>
#include <utils.h>
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

    void setDefaultColumnAlignment(Qt::Alignment align);
    void setDefaultColumnLabelAlignment(Qt::Alignment align);
    void setDefaultRowLabelAlignment(Qt::Alignment align);
    void setRowLabels(char **labels);
    void setColumnLabels(char **labels);
    void setDrawCellCallback(Table_CBData *cbdata);
    QVector<QVector<char *> > cells;

    QModelIndex newIndex(int row, int column) const {
        return createIndex(row, column);
    }

signals:
    void nRowsOrColsChanged();

private:
    int ncols;
    int nrows;
    Qt::Alignment defaultColumnAlignment;
    Qt::Alignment defaultColumnLabelAlignment;
    Qt::Alignment defaultRowLabelAlignment;
    QStringList rowLabels;
    QStringList columnLabels;
    Table_CBData *cbdata;
};


#include <QTableView>

class TableView : public QTableView
{
    Q_OBJECT

public:
    TableView(QAbstractItemModel *model, QWidget *parent = 0);

    void commitEdit();

    void setEditorMaxLengths(int *maxlengths);

    void setLeaveCellCallback(Table_CBData *cbdata) {
        leave_cbdata = cbdata;
    }
    void setEnterCellCallback(Table_CBData *cbdata) {
        enter_cbdata = cbdata;
    }

public slots:
    void closeEditor();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event) {}
    void mouseReleaseEvent(QMouseEvent *event) {}
    void mouseDoubleClickEvent(QMouseEvent *event) {}

    void keyPressEvent(QKeyEvent *event)
    {
//        int ncols = model()->columnCount();
//        int nrows = model()->rowCount();

//        switch (event->key()) {
//        case Qt::Key_Enter:
//        case Qt::Key_Return:
//        case Qt::Key_Down:
//            qDebug() << "enter key pressed";
//            row = (previous_row + 1 < nrows) ? previous_row + 1 : 0;
//            col = previous_col;
//            jumpToCell();
//            break;

//        case Qt::Key_Up:
//            row = previous_row - 1;
//            col = previous_col;
//            jumpToCell();
//            break;

//        case Qt::Key_Tab:
//            if (previous_col < ncols) {
//                row = previous_row;
//                col = previous_col + 1;
//            } else {
//                row = previous_row + 1;
//                col = 0;
//            }
//            jumpToCell();
//            break;

//        case Qt::Key_Backtab:
//            if (previous_col != 0) {
//                row = previous_row;
//                col = previous_col - 1;
//            } else {
//                row = previous_row - 1;
//                col = ncols;
//            }
//            jumpToCell();
//            break;
//        }
    }

private:
    void jumpToCell();
    int leaveCellEvent(int row, int col);
    int enterCellEvent(int row, int col);
    void openEditor(QModelIndex index);
    void closeEditor(QModelIndex index);

    Table_CBData *enter_cbdata;
    Table_CBData *leave_cbdata;
    int previous_row;
    int previous_col;
    int row;
    int col;
    QModelIndex previousIndex;
    QLineEdit *lineEditor;
    QList<int> editLengths;
};

class Validator : public QValidator
{
    Q_OBJECT

public:
    Validator(TextValidate_CBData *cbdata, QWidget *parent = 0);
    State validate(QString &input, int &pos) const;
    static char *text;
    static int pos;

private:
    TextValidate_CBData *cbdata;
};

class KeyPressListener : public QObject
 {
     Q_OBJECT
 public:
    KeyPressListener(QObject *parent);

 protected:
     bool eventFilter(QObject *obj, QEvent *event);
 };

class WhatsThisListener : public QObject
 {
     Q_OBJECT
 public:
    WhatsThisListener(QObject *parent);

 signals:
      void whatsThis();

 protected:
     bool eventFilter(QObject *obj, QEvent *event);
 };

class TreeModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    TreeModel(QObject *parent = 0)
    {
        qDebug() << "TreeModel";
        root_quark = 0;
    }

    QModelIndex index(int row, int col, const QModelIndex &parent = QModelIndex()) const
    {
        qDebug() << "index";
        if (!root_quark || col > 1) return QModelIndex();

        if (parent.isValid()) {
            Storage *sto = quark_get_children((Quark *) parent.internalPointer());
            void *q;
            if (storage_scroll_to_id(sto, row) == RETURN_SUCCESS) {
                if (storage_get_data(sto, &q) == RETURN_SUCCESS) {
                    return createIndex(row, 0, q);
                }
            }
        } else {
            if (row == 0) {
                return createIndex(0, 0, root_quark);
            } else {
                return QModelIndex();
            }
        }
    }


    QModelIndex parent(const QModelIndex &index) const
    {
        qDebug() << "parent";
        Quark *q = (Quark *) index.internalPointer();
        Quark *qparent = quark_parent_get(q);
        int row;

        if (qparent) {
            Storage *sto = quark_get_children(qparent);
            if (storage_scroll_to_data(sto, q) == RETURN_SUCCESS) {
                if ((row = storage_get_id(sto)) != -1) {
                    return createIndex(row, 0, qparent);
                }
            }
        }

        return QModelIndex();
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const
    {
        qDebug() << "rowCount";
        if (parent.isValid()) {
            Quark *q = (Quark *) parent.internalPointer();
            return quark_count_children(q);
        } else {
            return 1;
        }
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const
    {
        qDebug() << "colCount";
        return 1;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        qDebug() << "data";
        if (role == Qt::DisplayRole) {
            Quark *q = (Quark *)index.internalPointer();
            if (q) {
                return QVariant(q_labeling(q));
            }
        }
        return QVariant();
    }

    void setRootQuark(Quark *q)
    {
        qDebug() << "setRootQuark";
        this->root_quark = q;
    }

private:
    Tree_CBData *cbdata;
    Quark *root_quark;

};

#include <QTreeView>

class TreeView : public QTreeView
{
    Q_OBJECT

public:
    TreeView(QWidget *parent = 0);

    void setDropCallback(Tree_CBData *cbdata) {
        drop_cbdata = cbdata;
    }

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);

private:
    QPoint dragStartPosition;
    Tree_CBData *drop_cbdata;

};

#endif /* __QTGRACE_H_ */

