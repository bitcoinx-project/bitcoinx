#include "callcontract.h"
#include "ui_callcontract.h"
#include "platformstyle.h"
#include "walletmodel.h"
#include "clientmodel.h"
#include "guiconstants.h"
#include "rpcconsole.h"
#include "execrpccommand.h"
#include "abifunctionfield.h"
#include "contractabi.h"
#include "tabbarinfo.h"
#include "contractresult.h"
#include "contractbookpage.h"
#include "editcontractinfodialog.h"
#include "contracttablemodel.h"
#include "styleSheet.h"
#include "guiutil.h"
#include <QClipboard>
#include "titlebar.h"
#include "switchbuttonstyle.h"

namespace CallContract_NS
{
// Contract data names
static const QString PRC_COMMAND = "callcontract";
static const QString PARAM_ADDRESS = "address";
static const QString PARAM_DATAHEX = "datahex";
static const QString PARAM_SENDER = "sender";
}
using namespace CallContract_NS;

CallContract::CallContract(const PlatformStyle *platformStyle, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CallContract),
    m_model(0),
    m_clientModel(0),
    m_contractModel(0),
    m_execRPCCommand(0),
    m_ABIFunctionField(0),
    m_contractABI(0),
    m_tabInfo(0),
    m_callBar(0),
    m_results(1)
{
    m_platformStyle = platformStyle;
    // Setup ui components
    ui->setupUi(this);
    ui->saveInfoButton->setIcon(platformStyle->MultiStatesIcon(":/icons/filesave", PlatformStyle::PushButton));
    ui->loadInfoButton->setIcon(platformStyle->MultiStatesIcon(":/icons/address-book", PlatformStyle::PushButton));
    ui->pasteAddressButton->setIcon(platformStyle->MultiStatesIcon(":/icons/editpaste", PlatformStyle::PushButton));
    // Format tool buttons
    GUIUtil::formatToolButtons(ui->saveInfoButton, ui->loadInfoButton, ui->pasteAddressButton);

    // Set stylesheet
    SetObjectStyleSheet(ui->pushButtonClearAll, StyleSheetNames::ButtonBlack);

    m_ABIFunctionField = new ABIFunctionField(platformStyle, ABIFunctionField::Call, ui->scrollAreaFunction);
    ui->scrollAreaFunction->setWidget(m_ABIFunctionField);

    ui->labelContractAddress->setToolTip(tr("The account address."));
    ui->labelSenderAddress->setToolTip(tr("The sender address hex string."));
    ui->pushButtonCallContract->setEnabled(false);
    ui->lineEditSenderAddress->setComboBoxEditable(true);

    m_tabInfo = new TabBarInfo(ui->stackedWidget);
    m_tabInfo->addTab(0, tr("Call Contract"));

    m_callBar = new TitleBar(platformStyle);
    ui->callLayout->addWidget(m_callBar);

    // Create new PRC command line interface
    QStringList lstMandatory;
    lstMandatory.append(PARAM_ADDRESS);
    lstMandatory.append(PARAM_DATAHEX);
    QStringList lstOptional;
    lstOptional.append(PARAM_SENDER);
    QMap<QString, QString> lstTranslations;
    lstTranslations[PARAM_ADDRESS] = ui->labelContractAddress->text();
    lstTranslations[PARAM_SENDER] = ui->labelSenderAddress->text();
    m_execRPCCommand = new ExecRPCCommand(PRC_COMMAND, lstMandatory, lstOptional, lstTranslations, this);
    m_contractABI = new ContractABI();
    // Connect signals with slots
    connect(ui->pushButtonClearAll, SIGNAL(clicked()), SLOT(on_clearAllClicked()));
    connect(ui->pushButtonCallContract, SIGNAL(clicked()), SLOT(on_callContractClicked()));
    connect(ui->lineEditContractAddress, SIGNAL(textChanged(QString)), SLOT(on_updateCallContractButton()));
    connect(ui->textEditInterface, SIGNAL(textChanged()), SLOT(on_newContractABI()));
    connect(ui->stackedWidget, SIGNAL(currentChanged(int)), SLOT(on_updateCallContractButton()));
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
    ui->sendToContractButton->setStyleSheet(QString::fromStdString(commonMiddleBtStyle));
    ui->callContractButton->setStyleSheet(QString::fromStdString(checkedRightBtStyle));
}
CallContract::~CallContract()
{
    delete m_contractABI;
    delete ui;
}

void CallContract::setClientModel(ClientModel *_clientModel)
{
    m_clientModel = _clientModel;

    if (m_clientModel)
    {
        connect(m_clientModel, SIGNAL(tipChanged()), this, SLOT(on_numBlocksChanged()));
        on_numBlocksChanged();
    }
}

bool CallContract::isValidContractAddress()
{
    ui->lineEditContractAddress->checkValidity();
    return ui->lineEditContractAddress->isValid();
}

void CallContract::setModel(WalletModel *_model)
{
    m_model = _model;
    m_contractModel = m_model->getContractTableModel();
}

bool CallContract::isDataValid()
{
    bool dataValid = true;

    if(!isValidContractAddress())
        dataValid = false;
    if(!isValidInterfaceABI())
        dataValid = false;
    if(!m_ABIFunctionField->isValid())
        dataValid = false;
    if(!ui->lineEditSenderAddress->isValidAddress())
        dataValid = false;

    return dataValid;
}

bool CallContract::isValidInterfaceABI()
{
    ui->textEditInterface->checkValidity();
    return ui->textEditInterface->isValid();
}

void CallContract::setContractAddress(const QString &address)
{
    ui->lineEditContractAddress->setText(address);
    ui->lineEditContractAddress->setFocus();
}

void CallContract::on_clearAllClicked()
{
    ui->lineEditContractAddress->clear();
    ui->lineEditSenderAddress->setCurrentIndex(-1);
    ui->textEditInterface->clear();
    m_tabInfo->clear();
}

void CallContract::on_numBlocksChanged()
{
    if(m_clientModel)
    {
        ui->lineEditSenderAddress->on_refresh();
    }
}

void CallContract::on_updateCallContractButton()
{
    int func = m_ABIFunctionField->getSelectedFunction();
    bool enabled = func != -1;
    if(ui->lineEditContractAddress->text().isEmpty())
    {
        enabled = false;
    }
    enabled &= ui->stackedWidget->currentIndex() == 0;

    ui->pushButtonCallContract->setEnabled(enabled);
}

void CallContract::on_callContractClicked()
{
    if(isDataValid())
    {
        // Initialize variables
        QMap<QString, QString> lstParams;
        QVariant result;
        QString errorMessage;
        QString resultJson;
        int func = m_ABIFunctionField->getSelectedFunction();

        // Append params to the list
        ExecRPCCommand::appendParam(lstParams, PARAM_ADDRESS, ui->lineEditContractAddress->text());
        ExecRPCCommand::appendParam(lstParams, PARAM_DATAHEX, toDataHex(func, errorMessage));
        ExecRPCCommand::appendParam(lstParams, PARAM_SENDER, ui->lineEditSenderAddress->currentText());

        // Execute RPC command line
        if(errorMessage.isEmpty() && m_execRPCCommand->exec(lstParams, result, resultJson, errorMessage))
        {
            ContractResult *widgetResult = new ContractResult(ui->stackedWidget);
            widgetResult->setResultData(result, m_contractABI->functions[func], m_ABIFunctionField->getParamsValues(), ContractResult::CallResult);
            ui->stackedWidget->addWidget(widgetResult);
            int position = ui->stackedWidget->count() - 1;
            m_results = position == 1 ? 1 : m_results + 1;

            m_tabInfo->addTab(position, tr("Result %1").arg(m_results));
            m_tabInfo->setCurrent(position);
        }
        else
        {
            QMessageBox::warning(this, tr("Call contract"), errorMessage);
        }
    }else{
        QString questionString = tr("Input error or format error <br />");
        QMessageBox::warning(NULL, "warning",questionString, QMessageBox::Yes, QMessageBox::Yes);  
    }
}

void CallContract::on_saveInfoClicked()
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

void CallContract::on_newContractABI()
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

    on_updateCallContractButton();
}

void CallContract::on_loadInfoClicked()
{
    ContractBookPage dlg(m_platformStyle, this);
    dlg.setModel(m_model->getContractTableModel());
    if(dlg.exec())
    {
        ui->lineEditContractAddress->setText(dlg.getAddressValue());
        on_contractAddressChanged();
    }
}

void CallContract::on_pasteAddressClicked()
{
    setContractAddress(QApplication::clipboard()->text());
}

void CallContract::on_contractAddressChanged()
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

void CallContract::setTabBarInfo()
{
    QObject* info = (QWidget*)this->findChild<TabBarInfo *>("");
    m_callBar->setTabBarInfo(info);
}

QString CallContract::toDataHex(int func, QString& errorMessage)
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

void CallContract::on_createContractButtonClicked()
{
    Q_EMIT gotoCreateContract();
}

void CallContract::on_sendToContractButtonClicked()
{
    Q_EMIT gotoSendToContract();
}

void CallContract::on_callContractButtonClicked()
{
    Q_EMIT gotoCallContract();
}
