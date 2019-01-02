#ifndef BITCOIN_QT_GASPRICEFIELD_H
#define BITCOIN_QT_GASPRICEFIELD_H

#include "amount.h"

#include <QWidget>

class GasAmountSpinBox;
class QComboBox;

/** Widget for entering bitcoin amounts.
  */
class GasPriceField: public QWidget
{
   Q_OBJECT
   Q_PROPERTY(qint64 value READ value WRITE setValue WRITE setMinimum  USER true)
public:
    explicit GasPriceField(QWidget *parent = 0);
    CAmount value() const;
    void setValue(const CAmount& value);
    void setMinimum(const CAmount& min);
    void setDecimals(int len);
    ~GasPriceField(){}
public Q_SLOTS:
    void unitChanged(int index);
private:
    GasAmountSpinBox *amount;
    QComboBox *unit;
};

#endif // BITCOIN_QT_GASPRICEFIELD_H
