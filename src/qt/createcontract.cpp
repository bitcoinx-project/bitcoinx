#include "createcontract.h"
#include "ui_createcontract.h"
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
#include "addressfield.h"
#include "abifunctionfield.h"
#include "contractabi.h"
#include "tabbarinfo.h"
#include "contractresult.h"
#include "sendcoinsdialog.h"
#include "styleSheet.h"
#include "contract/config.h"
#include "titlebar.h"
#include "switchbuttonstyle.h"
#include <QRegularExpressionValidator>

namespace CreateContract_NS
{
// Contract data names
static const QString PRC_COMMAND = "createcontract";
static const QString PARAM_BYTECODE = "bytecode";
static const QString PARAM_SENDER = "sender";
static const QString PARAM_GASLIMIT = "gaslimit";
static const QString PARAM_GASPRICE = "gasprice";

static const CAmount HIGH_GASPRICE = 0.01*COIN;
}
using namespace CreateContract_NS;

CreateContract::CreateContract(const PlatformStyle *platformStyle, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CreateContract),
    m_model(0),
    m_clientModel(0),
    m_execRPCCommand(0),
    m_ABIFunctionField(0),
    m_contractABI(0),
    m_tabInfo(0),
    m_createBar(0),
    m_results(1)
{
    // Setup ui components
    Q_UNUSED(platformStyle);
    ui->setupUi(this);

    // Set stylesheet
    SetObjectStyleSheet(ui->pushButtonClearAll, StyleSheetNames::ButtonBlack);

    setLinkLabels();
    m_ABIFunctionField = new ABIFunctionField(platformStyle, ABIFunctionField::Create, ui->scrollAreaConstructor);
    ui->scrollAreaConstructor->setWidget(m_ABIFunctionField);
    ui->labelBytecode->setToolTip(tr("The bytecode of the contract"));
    ui->labelSenderAddress->setToolTip(tr("The quantum address that will be used to create the contract."));
    m_createBar = new TitleBar(platformStyle);
    m_tabInfo = new TabBarInfo(ui->stackedWidget);
    m_tabInfo->addTab(0, tr("Create Contract"));
    ui->createLayout->addWidget(m_createBar);
    // Set defaults
    ui->lineEditGasPrice->setDecimals(MAX_GAS_PRICE_PRECISION);
    ui->lineEditGasPrice->setValue(DEFAULT_GAS_PRICE);
    ui->lineEditGasLimit->setMinimum(MINIMUM_GAS_LIMIT);
    ui->lineEditGasLimit->setMaximum(DEFAULT_BLOCK_GAS_LIMIT);
    ui->lineEditGasLimit->setValue(DEFAULT_GAS_LIMIT_OP_CREATE);
    ui->pushButtonCreateContract->setEnabled(false);
    ui->createContractButton->setFocusPolicy(Qt::NoFocus);
    ui->sendToContractButton->setFocusPolicy(Qt::NoFocus);
    ui->callContractButton->setFocusPolicy(Qt::NoFocus);
    // Create new PRC command line interface
    QStringList lstMandatory;
    lstMandatory.append(PARAM_BYTECODE);
    QStringList lstOptional;
    lstOptional.append(PARAM_GASLIMIT);
    lstOptional.append(PARAM_GASPRICE);
    lstOptional.append(PARAM_SENDER);
    QMap<QString, QString> lstTranslations;
    lstTranslations[PARAM_BYTECODE] = ui->labelBytecode->text();
    lstTranslations[PARAM_GASLIMIT] = ui->labelGasLimit->text();
    lstTranslations[PARAM_GASPRICE] = ui->labelGasPrice->text();
    lstTranslations[PARAM_SENDER] = ui->labelSenderAddress->text();
    m_execRPCCommand = new ExecRPCCommand(PRC_COMMAND, lstMandatory, lstOptional, lstTranslations, this);
    m_contractABI = new ContractABI();

    // Connect signals with slots
    connect(ui->pushButtonClearAll, SIGNAL(clicked()),SLOT(on_clearAllClicked()));
    connect(ui->pushButtonCreateContract, SIGNAL(clicked()), SLOT(on_createContractClicked()));
    connect(ui->textEditBytecode, SIGNAL(textChanged()), SLOT(on_updateCreateButton()));
    connect(ui->textEditInterface, SIGNAL(textChanged()), SLOT(on_newContractABI()));
    connect(ui->stackedWidget, SIGNAL(currentChanged(int)), SLOT(on_updateCreateButton()));
    connect(ui->createContractButton,SIGNAL(clicked()),SLOT(on_createContractButtonClicked()));
    connect(ui->sendToContractButton,SIGNAL(clicked()),SLOT(on_sendToContractButtonClicked()));
    connect(ui->callContractButton,SIGNAL(clicked()),SLOT(on_callContractButtonClicked()));

    // Set bytecode validator
    QRegularExpression regEx;
    regEx.setPattern(paternHex);
    QRegularExpressionValidator *bytecodeValidator = new QRegularExpressionValidator(ui->textEditBytecode);
    bytecodeValidator->setRegularExpression(regEx);
    ui->textEditBytecode->setCheckValidator(bytecodeValidator);

    ui->horizontalLayout_7->setContentsMargins(0,0,0,0);
    ui->horizontalLayout_7->setContentsMargins(QMargins(0,0,0,0));
    ui->horizontalLayout_7->setMargin(0);
    ui->horizontalLayout_7->setSpacing(0);
    ui->createContractButton->setStyleSheet(QString::fromStdString(checkedLeftBtStyle));
    ui->sendToContractButton->setStyleSheet(QString::fromStdString(commonMiddleBtStyle));
    ui->callContractButton->setStyleSheet(QString::fromStdString(commonRightBtStyle));
}

CreateContract::~CreateContract()
{
    delete m_contractABI;
    delete ui;
}
void CreateContract::initGasPriceStyle()
{
    ui->lineEditGasPrice->setValue(ui->lineEditGasPrice->value());
}
void CreateContract::setLinkLabels()
{
    ui->labelSolidity->setOpenExternalLinks(true);
    ui->labelSolidity->setText("<a href=\"https://ethereum.github.io/browser-solidity/\">Solidity compiler</a>");

    ui->labelToken->setOpenExternalLinks(true);
    ui->labelToken->setText("<a href=\"https://ethereum.org/token#the-code\">Token template</a>");
}

void CreateContract::setModel(WalletModel *_model)
{
    m_model = _model;
}

bool CreateContract::isValidBytecode()
{
    ui->textEditBytecode->checkValidity();
    return ui->textEditBytecode->isValid();
}

bool CreateContract::isValidInterfaceABI()
{
    ui->textEditInterface->checkValidity();
    return ui->textEditInterface->isValid();
}

bool CreateContract::isDataValid()
{
    bool dataValid = true;
    int func = m_ABIFunctionField->getSelectedFunction();
    bool funcValid = func == -1 ? true : m_ABIFunctionField->isValid();

    if(!isValidBytecode())
        dataValid = false;
    if(!isValidInterfaceABI())
        dataValid = false;
    if(!funcValid)
        dataValid = false;

    return dataValid;
}

void CreateContract::setClientModel(ClientModel *_clientModel)
{
    m_clientModel = _clientModel;

    if (m_clientModel)
    {
        connect(m_clientModel, SIGNAL(tipChanged()), this, SLOT(on_numBlocksChanged()));
        on_numBlocksChanged();
    }
}

void CreateContract::on_clearAllClicked()
{
    ui->textEditBytecode->clear();
    ui->lineEditGasLimit->setValue(DEFAULT_GAS_LIMIT_OP_CREATE);
    ui->lineEditGasPrice->setValue(DEFAULT_GAS_PRICE);
    ui->lineEditSenderAddress->setCurrentIndex(-1);
    ui->textEditInterface->clear();
    m_tabInfo->clear();
}

void CreateContract::on_createContractClicked()
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
        QString bytecode = ui->textEditBytecode->toPlainText() + toDataHex(func, errorMessage);
        ExecRPCCommand::appendParam(lstParams, PARAM_BYTECODE, bytecode);
        ExecRPCCommand::appendParam(lstParams, PARAM_GASLIMIT, QString::number(gasLimit));
        ExecRPCCommand::appendParam(lstParams, PARAM_GASPRICE, QString::number((double)gasPrice / BTC_2_BCX_RATE));
        ExecRPCCommand::appendParam(lstParams, PARAM_SENDER, ui->lineEditSenderAddress->currentText());

        QString questionString = tr("Are you sure you want to create contract? <br />");

        SendConfirmationDialog confirmationDialog(tr("Confirm contract creation."), questionString, 3, this);
        confirmationDialog.exec();
        QMessageBox::StandardButton retval = (QMessageBox::StandardButton)confirmationDialog.result();
        if(retval == QMessageBox::Yes)
        {
            // Execute RPC command line
            if(errorMessage.isEmpty() && m_execRPCCommand->exec(lstParams, result, resultJson, errorMessage))
            {
                ContractResult *widgetResult = new ContractResult(ui->stackedWidget);
                widgetResult->setResultData(result, FunctionABI(), QList<QStringList>(), ContractResult::CreateResult);
                ui->stackedWidget->addWidget(widgetResult);
                int position = ui->stackedWidget->count() - 1;
                m_results = position == 1 ? 1 : m_results + 1;

                m_tabInfo->addTab(position, tr("Result %1").arg(m_results));
                m_tabInfo->setCurrent(position);
            }
            else
            {
                QMessageBox::warning(this, tr("Create contract"), errorMessage);
            }
        }
    }else{
        QString questionString = tr("Input error or format error <br />");
        QMessageBox::warning(NULL, "warning",questionString, QMessageBox::Yes, QMessageBox::Yes);  
    }
}

void CreateContract::on_numBlocksChanged()
{
    if(m_clientModel)
    {
        uint64_t blockGasLimit = 0;
        uint64_t minGasPrice = 0;
        uint64_t nGasPrice = 0;
        m_clientModel->getGasInfo(blockGasLimit, minGasPrice, nGasPrice);

        ui->labelGasLimit->setToolTip(tr("Gas limit. Default = %1, Max = %2").arg(DEFAULT_GAS_LIMIT_OP_CREATE).arg(DEFAULT_BLOCK_GAS_LIMIT));
        ui->labelGasPrice->setToolTip(tr("Gas price:BitcoinX price per gas unit. Default = %1, Min = %2").arg(QString::fromStdString(FormatMoney(DEFAULT_GAS_PRICE))).arg(QString::fromStdString(FormatMoney(minGasPrice))));
        ui->lineEditGasLimit->setMaximum(DEFAULT_BLOCK_GAS_LIMIT);

        ui->lineEditSenderAddress->on_refresh();
    }
}

void CreateContract::on_updateCreateButton()
{
    bool enabled = true;
    if(ui->textEditBytecode->toPlainText().isEmpty())
    {
        enabled = false;
    }
    enabled &= ui->stackedWidget->currentIndex() == 0;

    ui->pushButtonCreateContract->setEnabled(enabled);
}

void CreateContract::on_newContractABI()
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

    on_updateCreateButton();
}

QString CreateContract::toDataHex(int func, QString& errorMessage)
{
    if(func == -1 || m_ABIFunctionField == NULL || m_contractABI == NULL)
    {
        return "";
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

void CreateContract::setTabBarInfo()
{
    QObject* info = (QWidget*)this->findChild<TabBarInfo *>("");
    m_createBar->setTabBarInfo(info);
}

void CreateContract::on_createContractButtonClicked()
{
    Q_EMIT gotoCreateContract();
}

void CreateContract::on_sendToContractButtonClicked()
{
    Q_EMIT gotoSendToContract();
}

void CreateContract::on_callContractButtonClicked()
{
    Q_EMIT gotoCallContract();
}
