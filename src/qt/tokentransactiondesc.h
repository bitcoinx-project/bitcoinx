#ifndef BITCOINX_QT_TOKENTRANSACTIONDESC_H
#define BITCOINX_QT_TOKENTRANSACTIONDESC_H

#include <QObject>
#include <QString>

class TokenTransactionRecord;

class CWallet;
class CTokenTx;

/** Provide a human-readable extended HTML description of a token transaction.
 */
class TokenTransactionDesc: public QObject
{
    Q_OBJECT

public:
    static QString toHTML(CWallet *wallet, CTokenTx &wtx, TokenTransactionRecord *rec);

private:
    TokenTransactionDesc() {}

    static QString FormatTxStatus(CWallet *wallet, const CTokenTx& wtx);
};

#endif // BITCOINX_QT_TOKENTRANSACTIONDESC_H
