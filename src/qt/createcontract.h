#ifndef CREATECONTRACT_H
#define CREATECONTRACT_H

#include <QWidget>

class PlatformStyle;
class WalletModel;
class ClientModel;
class ExecRPCCommand;
class ABIFunctionField;
class ContractABI;
class TabBarInfo;
class TitleBar;

namespace Ui {
class CreateContract;
}

class CreateContract : public QWidget
{
    Q_OBJECT

public:
    explicit CreateContract(const PlatformStyle *platformStyle, QWidget *parent = 0);
    ~CreateContract();

    void setLinkLabels();
    void setClientModel(ClientModel *clientModel);
    void setModel(WalletModel *model);
    bool isValidBytecode();
    bool isValidInterfaceABI();
    bool isDataValid();
    void setTabBarInfo();
    void initGasPriceStyle();
Q_SIGNALS:
    void gotoCreateContract();
    void gotoSendToContract();
    void gotoCallContract();
public Q_SLOTS:
    void on_clearAllClicked();
    void on_createContractClicked();
    void on_numBlocksChanged();
    void on_updateCreateButton();
    void on_newContractABI();
    void on_createContractButtonClicked();
    void on_sendToContractButtonClicked();
    void on_callContractButtonClicked();

private:
    QString toDataHex(int func, QString& errorMessage);


private:

    Ui::CreateContract *ui;
    WalletModel* m_model;
    ClientModel* m_clientModel;
    ExecRPCCommand* m_execRPCCommand;
    ABIFunctionField* m_ABIFunctionField;
    ContractABI* m_contractABI;
    TabBarInfo* m_tabInfo;
    TitleBar* m_createBar;
    int m_results;
};

#endif // CREATECONTRACT_H
