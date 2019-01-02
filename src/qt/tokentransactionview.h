#ifndef TOKENVIEW_H
#define TOKENVIEW_H

#include "guiutil.h"

#include <QWidget>
#include <QKeyEvent>

class PlatformStyle;
class TokenFilterProxy;
class WalletModel;

QT_BEGIN_NAMESPACE
class QComboBox;
class QDateTimeEdit;
class QLineEdit;
class QMenu;
class QTableView;
QT_END_NAMESPACE

class TokenTransactionView : public QWidget
{
    Q_OBJECT
public:
    explicit TokenTransactionView(const PlatformStyle *platformStyle, QWidget *parent = 0);

    void setModel(WalletModel *model);

    // Date ranges for filter
    enum DateEnum
    {
        All,
        Today,
        ThisWeek,
        ThisMonth,
        LastMonth,
        ThisYear,
        Range
    };

    enum ColumnWidths {
        STATUS_COLUMN_WIDTH = 30,
        DATE_COLUMN_WIDTH = 130,
        TYPE_COLUMN_WIDTH = 150,
        NAME_COLUMN_WIDTH = 100,
        AMOUNT_COLUMN_WIDTH = 230,
        AMOUNT_MINIMUM_COLUMN_WIDTH = 160,
        MINIMUM_COLUMN_WIDTH = 23
    };

private:
    WalletModel *model;
    TokenFilterProxy *tokenProxyModel;
    QTableView *tokenView;

    QComboBox *nameWidget;
    QLineEdit *addressWidget;
    QLineEdit *amountWidget;
    QComboBox *dateWidget;
    QComboBox *typeWidget;

    QMenu *contextMenu;

    QFrame *dateRangeWidget;
    QDateTimeEdit *dateFrom;
    QDateTimeEdit *dateTo;

    QWidget *createDateRangeWidget();

    GUIUtil::TableViewLastColumnResizingFixer *columnResizingFixer;

    virtual void resizeEvent(QResizeEvent* event);
    bool eventFilter(QObject *obj, QEvent *event);

private Q_SLOTS:
    void contextualMenu(const QPoint &);
    void dateRangeChanged();
    void copyTxPlainText();
	void showDetails();
    void copyAddress();
    void copyAmount();
    void copyTxID();

Q_SIGNALS:

public Q_SLOTS:
    void chooseDate(int idx);
    void chooseType(int idx);
    void chooseName(int idx);
    void addToNameWidget(const QModelIndex& parent, int start, int /*end*/);
    void removeFromNameWidget(const QModelIndex& parent, int start, int /*end*/);
	void changedPrefix(const QString &prefix);
    void changedAmount(const QString &amount);
    void refreshNameWidget();
};

#endif // TOKENVIEW_H
