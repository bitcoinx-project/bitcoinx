#include "abiparamitem.h"
#include "contractabi.h"
#include "platformstyle.h"

#include <QHBoxLayout>
#include <QRegularExpressionValidator>

ABIParamItem::ABIParamItem(const PlatformStyle *platformStyle, const ParameterABI &param, QWidget *parent) :
    QWidget(parent),
    m_buttonAdd(new QToolButton(this)),
    m_buttonRemove(new QToolButton(this)),
    m_itemValue(new QValidatedLineEdit(this)),
    m_isDeleted(false)
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(2);
    mainLayout->setContentsMargins(0,0,0,0);

    m_buttonAdd->setIcon(platformStyle->MultiStatesIcon(":/icons/add", PlatformStyle::PushButton));
    m_buttonRemove->setIcon(platformStyle->MultiStatesIcon(":/icons/remove", PlatformStyle::PushButton));

    m_buttonAdd->setFixedSize(30,30);
    m_buttonRemove->setFixedSize(30,30);

    m_buttonAdd->setFocusPolicy(Qt::NoFocus);
    m_buttonRemove->setFocusPolicy(Qt::NoFocus);

    QRegularExpression regEx;
    if(ParameterABI::getRegularExpession(param.decodeType(), regEx))
    {
        QRegularExpressionValidator *validator = new QRegularExpressionValidator(m_itemValue);
        validator->setRegularExpression(regEx);
        m_itemValue->setEmptyIsValid(false);
        m_itemValue->setCheckValidator(validator);
    }

    mainLayout->addWidget(m_buttonAdd);
    mainLayout->addWidget(m_buttonRemove);
    mainLayout->addWidget(m_itemValue);
    setLayout(mainLayout);

    connect(m_buttonAdd, SIGNAL(clicked(bool)), this, SLOT(on_addItemClicked()));
    connect(m_buttonRemove, SIGNAL(clicked(bool)), this, SLOT(on_removeItemClicked()));
}

QString ABIParamItem::getValue()
{
    return m_itemValue->text();
}

void ABIParamItem::setFixed(bool isFixed)
{
    m_buttonAdd->setVisible(!isFixed);
    m_buttonRemove->setVisible(!isFixed);
}

void ABIParamItem::on_addItemClicked()
{
    Q_EMIT on_addItemClicked(m_position + 1);
}

void ABIParamItem::on_removeItemClicked()
{
    Q_EMIT on_removeItemClicked(m_position);
}


int ABIParamItem::getPosition() const
{
    return m_position;
}

void ABIParamItem::setPosition(int position)
{
    m_position = position;
}


bool ABIParamItem::getIsDeleted() const
{
    return m_isDeleted;
}

void ABIParamItem::setIsDeleted(bool isDeleted)
{
    m_isDeleted= isDeleted;
    if(isDeleted)
    {
        m_itemValue->setText("");
        m_itemValue->setValid(true);
    }
    m_itemValue->setVisible(!isDeleted);
    m_buttonRemove->setEnabled(!isDeleted);
}

bool ABIParamItem::isValid()
{
    m_itemValue->checkValidity();
    return  m_itemValue->isValid();
}
