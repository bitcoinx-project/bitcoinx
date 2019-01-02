#include "tokendescdialog.h"
#include "ui_tokendescdialog.h"

#include "styleSheet.h"
#include "tokenfilterproxy.h"

#include <QModelIndex>

TokenDescDialog::TokenDescDialog(const QModelIndex &idx, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TokenDescDialog)
{
    ui->setupUi(this);

    // Set stylesheet
    SetObjectStyleSheet(this, StyleSheetNames::ScrollBarDark);

    setWindowTitle(tr("Details for %1").arg(idx.data(TokenTransactionTableModel::TxHashRole).toString()));
    QString desc = idx.data(TokenTransactionTableModel::LongDescriptionRole).toString();
    ui->detailText->setHtml(desc);
    ui->detailTextLayout->setMargin(1);
}

TokenDescDialog::~TokenDescDialog()
{
    delete ui;
}
