#include "gaspricefield.h"

#include "bitcoinunits.h"
#include "styleSheet.h"
#include "guiconstants.h"
#include "qvaluecombobox.h"

#include <QApplication>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include "amount.h"
static const double SATOSHISTEP = 0.0001;
static const double BCXSTEP = 0.00000001;
static const double MINIMUN = 0.004;
static const double MAXIMUN = 0.01;
class GasAmountSpinBox: public QDoubleSpinBox
{
   Q_OBJECT

public:
    explicit GasAmountSpinBox(QWidget *parent)
    {
        setAlignment(Qt::AlignRight);
        setSingleStep(SATOSHISTEP);
        setRange(MINIMUN,MAXIMUN);
        connect(this,SIGNAL(valueChanged(double)),this,SLOT(mySlot(double)));
    }
    void setValue(double val)
    {
        QString valStr = QString::number(val,10,8);
        int len = valStr.length() - 1;
        while(valStr[len] == '0' || valStr[len] == '.')
            len--;
        lineEdit()->setText(valStr.left(len + 1));
    }
protected:
    void focusInEvent(QFocusEvent *event)
    {
        setValue(value());
    }
    void focusOutEvent(QFocusEvent *event)
    {
        setValue(value());
    }
public Q_SLOTS:
    void mySlot(double v)
    {
        setValue(v);
    }
};
#include "gaspricefield.moc"

GasPriceField::GasPriceField(QWidget *parent) :
    QWidget(parent),
    amount(0)
{
    amount = new GasAmountSpinBox(this);
    amount->setLocale(QLocale::c());
    amount->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(amount);
    unit = new QValueComboBox(this);
    unit->addItem("satoshi");
    unit->addItem("BCX");
    unit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    unit->setMinimumWidth(120);
    layout->addWidget(unit);
    unit->setCurrentIndex(0);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);
    setFocusPolicy(Qt::TabFocus);
    setFocusProxy(amount);
    connect(unit, SIGNAL(currentIndexChanged(int)), this, SLOT(unitChanged(int)));
}
CAmount GasPriceField::value() const
{
    if(unit->currentIndex() == 0){
        return amount->value() * BTC_2_BCX_RATE;
    }else if(unit->currentIndex() == 1){
        return amount->value() * BTC_2_BCX_RATE * 10000;
    }
    return 0;
}
void GasPriceField::unitChanged(int index)
{
    double amountNum = 0;
    if(index == 0){
        amountNum = amount->value() * BTC_2_BCX_RATE;
        amount->setRange(MINIMUN,MAXIMUN);
        amount->setSingleStep(SATOSHISTEP);
        amount->setValue(amountNum); 
    }else if(index == 1){
        amountNum = amount->value() / BTC_2_BCX_RATE;
        amount->setRange(MINIMUN / BTC_2_BCX_RATE,MAXIMUN / BTC_2_BCX_RATE);
        amount->setSingleStep(BCXSTEP);
        amount->setValue(amountNum);
    }
}
void GasPriceField::setValue(const CAmount& value)
{
    amount->setValue(double(value) / BTC_2_BCX_RATE);
}
void GasPriceField::setMinimum(const CAmount& min)
{
    if(unit->currentIndex() == 0){
        amount->setMinimum(double(min) / BTC_2_BCX_RATE);
    }else if(unit->currentIndex() == 1){
        amount->setMinimum(double(min) / BTC_2_BCX_RATE / 10000);
    }
}
void GasPriceField::setDecimals(int len)
{
    amount->setDecimals(len);
}