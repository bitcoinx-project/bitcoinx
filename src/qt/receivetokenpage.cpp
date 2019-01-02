#include "receivetokenpage.h"
#include "ui_receivetokenpage.h"

#include "guiutil.h"
#include "guiconstants.h"
#include "receiverequestdialog.h"
#include "platformstyle.h"

ReceiveTokenPage::ReceiveTokenPage(const PlatformStyle *_platformStyle, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReceiveTokenPage),
    platformStyle(_platformStyle)
{
    ui->setupUi(this);
    connect(ui->copyAddressButton, SIGNAL(clicked()), this, SLOT(on_copyAddressClicked()));
    ui->copyAddressButton->setIcon(platformStyle->MultiStatesIcon(":/icons/editcopy", PlatformStyle::PushButton));
    ui->copyAddressButton->setVisible(false);
    setAddress("");
}

ReceiveTokenPage::~ReceiveTokenPage()
{
    delete ui;
}

void ReceiveTokenPage::setAddress(QString address)
{
    m_address = address;
    createQRCode();
}

void ReceiveTokenPage::createQRCode()
{
    SendCoinsRecipient info;
    if(!m_address.isEmpty())
    {
        info.address = m_address;
        if(ReceiveRequestDialog::createQRCode(ui->lblQRCode, info))
        {
            ui->lblQRCode->setVisible(true);
            ui->lblQRCode->setScaledContents(true);
        }
        else
        {
            ui->lblQRCode->setVisible(false);
        }
        ui->labelTokenAddress->setText(m_address);
        ui->copyAddressButton->setVisible(true);
    }
    else
    {
        ui->lblQRCode->clear();
        ui->labelTokenAddress->setText("");
        ui->labelTokenAddressText->setText("");
        ui->copyAddressButton->setVisible(false);
    }
}

void ReceiveTokenPage::setSymbol(QString symbol)
{
    QString addressText = symbol.isEmpty() ? "" : (QString("%1 ").arg(symbol) + tr("Address"));
    ui->labelTokenAddressText->setText(addressText);
}

void ReceiveTokenPage::on_copyAddressClicked()
{
    if(!m_address.isEmpty())
        GUIUtil::setClipboard(m_address);
}
