#include "sendtocontract.h"
#include "ui_sendtocontract.h"
#include "platformstyle.h"
#include "walletmodel.h"
#include "clientmodel.h"
#include "guiconstants.h"
#include "rpcconsole.h"
#include "execrpccommand.h"
#include "bitcoinunits.h"
#include "optionsmodel.h"
#include "validation.h"
#include "utilmoneystr.h"
#include "abifunctionfield.h"
#include "contractabi.h"
#include "tabbarinfo.h"
#include "contractresult.h"
#include "contractbookpage.h"
#include "editcontractinfodialog.h"
#include "contracttablemodel.h"
#include "styleSheet.h"
#include "guiutil.h"
#include "sendcoinsdialog.h"
#include <QClipboard>
#include "contract/config.h"
#include "titlebar.h"
#include "switchbuttonstyle.h"

namespace SendToContract_NS
{
// Contract data names
static const QString PRC_COMMAND = "sendtocontract";
static const QString PARAM_ADDRESS = "address";
static const QString PARAM_DATAHEX = "datahex";
static const QString PARAM_AMOUNT = "amount";
static const QString PARAM_GASLIMIT = "gaslimit";
static const QString PARAM_GASPRICE = "gasprice";
static const QString PARAM_SENDER = "sender";

static const CAmount HIGH_GASPRICE = 0.01*COIN;
}
using namespace SendToContract_NS;

SendToContract::SendToContract(const PlatformStyle *platformStyle, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SendToContract),
    m_model(0),
    m_clientModel(0),
    m_sendToBar(0),
    m_tabInfo(0),
    m_contractModel(0),
    m_execRPCCommand(0),
    m_ABIFunctionField(0),
    m_contractABI(0),
    m_results(1)
{
    m_platformStyle = platformStyle;

    // Setup ui components
    Q_UNUSED(platformStyle);
    ui->setupUi(this);
    ui->saveInfoButton->setIcon(platformStyle->MultiStatesIcon(":/icons/filesave", PlatformStyle::PushButton));
    ui->loadInfoButton->setIcon(platformStyle->MultiStatesIcon(":/icons/address-book", PlatformStyle::PushButton));
    ui->pasteAddressButton->setIcon(platformStyle->MultiStatesIcon(":/icons/editpaste", PlatformStyle::PushButton));
    // Format tool buttons
    GUIUtil::formatToolButtons(ui->saveInfoButton, ui->loadInfoButton, ui->pasteAddressButton);

    // Set stylesheet
    SetObjectStyleSheet(ui->pushButtonClearAll, StyleSheetNames::ButtonBlack);

    m_ABIFunctionField = new ABIFunctionField(platformStyle, ABIFunctionField::SendTo, ui->scrollAreaFunction);
    ui->scrollAreaFunction->setWidget(m_ABIFunctionField);
    ui->lineEditAmount->setEnabled(true);
    ui->labelContractAddress->setToolTip(tr("The contract address that will receive the funds and data."));
    ui->labelAmount->setToolTip(tr("The amount in BitcoinX to send. Default = 0."));
    ui->labelSenderAddress->setToolTip(tr("The quantum address that will be used as sender."));

    m_tabInfo = new TabBarInfo(ui->stackedWidget);
    m_tabInfo->addTab(0, tr("Send To Contract"));

    m_sendToBar = new TitleBar(platformStyle);
    ui->sendToLayout->addWidget(m_sendToBar);

    // Set defaults
    ui->lineEditGasPrice->setDecimals(MAX_GAS_PRICE_PRECISION);
    ui->lineEditGasPrice->setValue(DEFAULT_GAS_PRICE);
    ui->lineEditGasLimit->setMinimum(MINIMUM_GAS_LIMIT);
    ui->lineEditGasLimit->setMaximum(DEFAULT_BLOCK_GAS_LIMIT);
    ui->lineEditGasLimit->setValue(DEFAULT_GAS_LIMIT_OP_SEND);
    ui->textEditInterface->setIsValidManually(true);
    ui->pushButtonSendToContract->setEnabled(false);
    // Create new PRC command line interface
    QStringList lstMandatory;
    lstMandatory.append(PARAM_ADDRESS);
    lstMandatory.append(PARAM_DATAHEX);
    QStringList lstOptional;
    lstOptional.append(PARAM_AMOUNT);
    lstOptional.append(PARAM_GASLIMIT);
    lstOptional.append(PARAM_GASPRICE);
    lstOptional.append(PARAM_SENDER);
    QMap<QString, QString> lstTranslations;
    lstTranslations[PARAM_ADDRESS] = ui->labelContractAddress->text();
    lstTranslations[PARAM_AMOUNT] = ui->labelAmount->text();
    lstTranslations[PARAM_GASLIMIT] = ui->labelGasLimit->text();
    lstTranslations[PARAM_GASPRICE] = ui->labelGasPrice->text();
    lstTranslations[PARAM_SENDER] = ui->labelSenderAddress->text();
    m_execRPCCommand = new ExecRPCCommand(PRC_COMMAND, lstMandatory, lstOptional, lstTranslations, this);
    m_contractABI = new ContractABI();

    // Connect signals with slots
    connect(ui->pushButtonClearAll, SIGNAL(clicked()), SLOT(on_clearAllClicked()));
    connect(ui->pushButtonSendToContract, SIGNAL(clicked()), SLOT(on_sendToContractClicked()));
    connect(ui->lineEditContractAddress, SIGNAL(textChanged(QString)), SLOT(on_updateSendToContractButton()));
    connect(ui->textEditInterface, SIGNAL(textChanged()), SLOT(on_newContractABI()));
    connect(ui->stackedWidget, SIGNAL(currentChanged(int)), SLOT(on_updateSendToContractButton()));
    connect(m_ABIFunctionField, SIGNAL(functionChanged()), SLOT(on_functionChanged()));
    connect(ui->saveInfoButton, SIGNAL(clicked()), SLOT(on_saveInfoClicked()));
    connect(ui->loadInfoButton, SIGNAL(clicked()), SLOT(on_loadInfoClicked()));
    connect(ui->pasteAddressButton, SIGNAL(clicked()), SLOT(on_pasteAddressClicked()));
    connect(ui->lineEditContractAddress, SIGNAL(textChanged(QString)), SLOT(on_contractAddressChanged()));
    connect(ui->createContractButton,SIGNAL(clicked()),SLOT(on_createContractButtonClicked()));
    connect(ui->sendToContractButton,SIGNAL(clicked()),SLOT(on_sendToContractButtonClicked()));
    connect(ui->callContractButton,SIGNAL(clicked()),SLOT(on_callContractButtonClicked()));

    // Set contract address validator
    QRegularExpression regEx;
    regEx.setPattern(paternAddress);
    QRegularExpressionValidator *addressValidatr = new QRegularExpressionValidator(ui->lineEditContractAddress);
    addressValidatr->setRegularExpression(regEx);
    ui->lineEditContractAddress->setCheckValidator(addressValidatr);
    ui->createContractButton->setFocusPolicy(Qt::NoFocus);
    ui->sendToContractButton->setFocusPolicy(Qt::NoFocus);
    ui->callContractButton->setFocusPolicy(Qt::NoFocus);

    ui->horizontalLayout_7->setContentsMargins(0,0,0,0);
    ui->horizontalLayout_7->setContentsMargins(QMargins(0,0,0,0));
    ui->horizontalLayout_7->setMargin(0);
    ui->horizontalLayout_7->setSpacing(0);
    ui->createContractButton->setStyleSheet(QString::fromStdString(commonLeftBtStyle));
    ui->sendToContractButton->setStyleSheet(QString::fromStdString(checkedMiddleBtStyle));
    ui->callContractButton->setStyleSheet(QString::fromStdString(commonRightBtStyle));
}

SendToContract::~SendToContract()
{
    delete m_contractABI;
    delete ui;
}

bool SendToContract::isValidContractAddress()
{
    ui->lineEditContractAddress->checkValidity();
    return ui->lineEditContractAddress->isValid();
}
void SendToContract::initGasPriceStyle()
{
    ui->lineEditGasPrice->setValue(ui->lineEditGasPrice->value());
}
void SendToContract::setModel(WalletModel *_model)
{
    m_model = _model;
    m_contractModel = m_model->getContractTableModel();
}

bool SendToContract::isValidInterfaceABI()
{
    ui->textEditInterface->checkValidity();
    return ui->textEditInterface->isValid();
}

void SendToContract::setContractAddress(const QString &address)
{
    ui->lineEditContractAddress->setText(address);
    ui->lineEditContractAddress->setFocus();
}

bool SendToContract::isDataValid()
{
    bool dataValid = true;
    if(!isValidContractAddress())
        dataValid = false;
    if(!isValidInterfaceABI())
        dataValid = false;
    if(!m_ABIFunctionField->isValid())
        dataValid = false;
    return dataValid;
}

void SendToContract::setClientModel(ClientModel *_clientModel)
{
    m_clientModel = _clientModel;

    if (m_clientModel)
    {
        connect(m_clientModel, SIGNAL(tipChanged()), this, SLOT(on_numBlocksChanged()));
        on_numBlocksChanged();
    }
}

void SendToContract::on_sendToContractClicked()
{
    if(isDataValid())
    {
        WalletModel::UnlockContext ctx(m_model->requestUnlock());
        if(!ctx.isValid())
        {
            return;
        }

        // Initialize variables
        QMap<QString, QString> lstParams;
        QVariant result;
        QString errorMessage;
        QString resultJson;
        int unit = m_model->getOptionsModel()->getDisplayUnit();
        uint64_t gasLimit = ui->lineEditGasLimit->value();
        CAmount gasPrice = ui->lineEditGasPrice->value();
        int func = m_ABIFunctionField->getSelectedFunction();

        // Check for high gas price
        if(gasPrice > HIGH_GASPRICE)
        {
            QString message = tr("The Gas Price is too high, are you sure you want to possibly spend a max of %1 for this transaction?");
            if(QMessageBox::question(this, tr("High Gas price"), message.arg(QString::number(gasLimit * gasPrice) + " satoshi")) == QMessageBox::No)
                return;
        }

        // Append params to the list
        ExecRPCCommand::appendParam(lstParams, PARAM_ADDRESS, ui->lineEditContractAddress->text());
        ExecRPCCommand::appendParam(lstParams, PARAM_DATAHEX, toDataHex(func, errorMessage));
        QString amount = isFunctionPayable() ? BitcoinUnits::format(unit, ui->lineEditAmount->value(), false, BitcoinUnits::separatorNever) : "0";
        ExecRPCCommand::appendParam(lstParams, PARAM_AMOUNT, amount);
        ExecRPCCommand::appendParam(lstParams, PARAM_GASLIMIT, QString::number(gasLimit));
        ExecRPCCommand::appendParam(lstParams, PARAM_GASPRICE, QString::number((double)gasPrice / BTC_2_BCX_RATE));
        ExecRPCCommand::appendParam(lstParams, PARAM_SENDER, ui->lineEditSenderAddress->currentText());

        QString questionString = tr("Are you sure you want to send to the contract: <br /><br />");
        questionString.append(tr("<b>%1</b>?")
                              .arg(ui->lineEditContractAddress->text()));

        SendConfirmationDialog confirmationDialog(tr("Confirm sending to contract."), questionString, 3, this);
        confirmationDialog.exec();
        QMessageBox::StandardButton retval = (QMessageBox::StandardButton)confirmationDialog.result();
        if(retval == QMessageBox::Yes)
        {
            // Execute RPC command line
            if(errorMessage.isEmpty() && m_execRPCCommand->exec(lstParams, result, resultJson, errorMessage))
            {
                ContractResult *widgetResult = new ContractResult(ui->stackedWidget);
                widgetResult->setResultData(result, FunctionABI(), m_ABIFunctionField->getParamsValues(), ContractResult::SendToResult);
                ui->stackedWidget->addWidget(widgetResult);
                int position = ui->stackedWidget->count() - 1;
                m_results = position == 1 ? 1 : m_results + 1;

                m_tabInfo->addTab(position, tr("Result %1").arg(m_results));
                m_tabInfo->setCurrent(position);
            }
            else
            {
                QMessageBox::warning(this, tr("Send to contract"), errorMessage);
            }
        }
    }else{
        QString questionString = tr("Input error or format error <br />");
        QMessageBox::warning(NULL, "warning",questionString, QMessageBox::Yes, QMessageBox::Yes);  
    }
}

void SendToContract::on_clearAllClicked()
{
    ui->lineEditContractAddress->clear();
    ui->lineEditAmount->clear();
    ui->lineEditAmount->setEnabled(true);
    ui->lineEditGasLimit->setValue(DEFAULT_GAS_LIMIT_OP_SEND);
    ui->lineEditGasPrice->setValue(DEFAULT_GAS_PRICE);
    ui->lineEditSenderAddress->setCurrentIndex(-1);
    ui->textEditInterface->clear();
    ui->textEditInterface->setIsValidManually(true);
    m_tabInfo->clear();
}

void SendToContract::on_numBlocksChanged()
{
    if(m_clientModel)
    {
        uint64_t blockGasLimit = 0;
        uint64_t minGasPrice = 0;
        uint64_t nGasPrice = 0;
        m_clientModel->getGasInfo(blockGasLimit, minGasPrice, nGasPrice);

        ui->labelGasLimit->setToolTip(tr("Gas limit: Default = %1, Max = %2.").arg(DEFAULT_GAS_LIMIT_OP_SEND).arg(blockGasLimit));
        ui->labelGasPrice->setToolTip(tr("Gas price: BitcoinX price per gas unit. Default = %1, Min = %2.").arg(QString::fromStdString(FormatMoney(DEFAULT_GAS_PRICE))).arg(QString::fromStdString(FormatMoney(minGasPrice))));
        ui->lineEditGasPrice->setMinimum(minGasPrice);

        ui->lineEditSenderAddress->on_refresh();
    }
}

void SendToContract::on_newContractABI()
{
    std::string json_data = ui->textEditInterface->toPlainText().toStdString();
    if(!m_contractABI->loads(json_data))
    {
        m_contractABI->clean();
        ui->textEditInterface->setIsValidManually(false);
    }
    else
    {
        ui->textEditInterface->setIsValidManually(true);
    }
    m_ABIFunctionField->setContractABI(m_contractABI);

    on_updateSendToContractButton();
}

void SendToContract::on_updateSendToContractButton()
{
    int func = m_ABIFunctionField->getSelectedFunction();
    bool enabled = func >= -1;
    if(ui->lineEditContractAddress->text().isEmpty())
    {
        enabled = false;
    }
    enabled &= ui->stackedWidget->currentIndex() == 0;
    ui->pushButtonSendToContract->setEnabled(enabled);
}

void SendToContract::on_functionChanged()
{
    bool payable = isFunctionPayable();
    ui->lineEditAmount->setEnabled(payable);
    if(!payable)
    {
        ui->lineEditAmount->clear();
    }
}

void SendToContract::on_loadInfoClicked()
{
    ContractBookPage dlg(m_platformStyle, this);
    dlg.setModel(m_model->getContractTableModel());
    if(dlg.exec())
    {
        ui->lineEditContractAddress->setText(dlg.getAddressValue());
        on_contractAddressChanged();
    }
}

void SendToContract::on_saveInfoClicked()
{
    if(!m_contractModel)
        return;

    bool valid = true;

    if(!isValidContractAddress())
        valid = false;

    if(!isValidInterfaceABI())
        valid = false;

    if(!valid)
        return;

    QString contractAddress = ui->lineEditContractAddress->text();
    int row = m_contractModel->lookupAddress(contractAddress);
    EditContractInfoDialog::Mode dlgMode = row > -1 ? EditContractInfoDialog::EditContractInfo : EditContractInfoDialog::NewContractInfo;
    EditContractInfoDialog dlg(dlgMode, this);
    dlg.setModel(m_contractModel);
    if(dlgMode == EditContractInfoDialog::EditContractInfo)
    {
        dlg.loadRow(row);
    }
    dlg.setAddress(ui->lineEditContractAddress->text());
    dlg.setABI(ui->textEditInterface->toPlainText());
    if(dlg.exec())
    {
        ui->lineEditContractAddress->setText(dlg.getAddress());
        ui->textEditInterface->setText(dlg.getABI());
        on_contractAddressChanged();
    }
}

void SendToContract::on_pasteAddressClicked()
{
    setContractAddress(QApplication::clipboard()->text());
}

QString SendToContract::toDataHex(int func, QString& errorMessage)
{
    if(func == -1 || m_ABIFunctionField == NULL || m_contractABI == NULL)
    {
        std::string defSelector = FunctionABI::defaultSelector();
        return QString::fromStdString(defSelector);
    }

    std::string strData;
    std::vector<std::vector<std::string>> values = m_ABIFunctionField->getValuesVector();
    FunctionABI function = m_contractABI->functions[func];
    std::vector<ParameterABI::ErrorType> errors;
    if(function.abiIn(values, strData, errors))
    {
        return QString::fromStdString(strData);
    }
    else
    {
        errorMessage = function.errorMessage(errors, true);
    }
    return "";
}

void SendToContract::on_contractAddressChanged()
{
    if(isValidContractAddress() && m_contractModel)
    {
        QString contractAddress = ui->lineEditContractAddress->text();
        if(m_contractModel->lookupAddress(contractAddress) > -1)
        {
            QString contractAbi = m_contractModel->abiForAddress(contractAddress);
            if(ui->textEditInterface->toPlainText() != contractAbi)
            {
                ui->textEditInterface->setText(m_contractModel->abiForAddress(contractAddress));
            }
        }
    }
}

bool SendToContract::isFunctionPayable()
{
    int func = m_ABIFunctionField->getSelectedFunction();
    if(func < 0) return true;
    FunctionABI function = m_contractABI->functions[func];
    return function.payable;
}

void SendToContract::setTabBarInfo()
{
    QObject* info = (QWidget*)this->findChild<TabBarInfo *>("");
    m_sendToBar->setTabBarInfo(info);
}

void SendToContract::on_createContractButtonClicked()
{
    Q_EMIT gotoCreateContract();
}

void SendToContract::on_sendToContractButtonClicked()
{
    Q_EMIT gotoSendToContract();
}

void SendToContract::on_callContractButtonClicked()
{
    Q_EMIT gotoCallContract();
}

