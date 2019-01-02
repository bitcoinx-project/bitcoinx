#ifndef SENDTOCONTRACT_H
#define SENDTOCONTRACT_H

#include <QWidget>

class PlatformStyle;
class WalletModel;
class ClientModel;
class ContractTableModel;
class ContractABI;
class TabBarInfo;
class TitleBar;
class ExecRPCCommand;
class ABIFunctionField;

namespace Ui {
class SendToContract;
}

class SendToContract : public QWidget
{
    Q_OBJECT

public:
    explicit SendToContract(const PlatformStyle *platformStyle, QWidget *parent = 0);
    ~SendToContract();

    void setClientModel(ClientModel *clientModel);
    void setModel(WalletModel *model);
    bool isValidContractAddress();
    bool isValidInterfaceABI();
    bool isDataValid();
    void setContractAddress(const QString &address);
    void setTabBarInfo();
    void initGasPriceStyle();

Q_SIGNALS:
    void gotoCreateContract();
    void gotoSendToContract();
    void gotoCallContract();

public Q_SLOTS:
    void on_clearAllClicked();
    void on_sendToContractClicked();
	void on_loadInfoClicked();
    void on_pasteAddressClicked();
    void on_contractAddressChanged();
    void on_numBlocksChanged();
    void on_updateSendToContractButton();
    void on_newContractABI();
    void on_functionChanged();
    void on_saveInfoClicked();
    void on_createContractButtonClicked();
    void on_sendToContractButtonClicked();
    void on_callContractButtonClicked();

private:
    QString toDataHex(int func, QString& errorMessage);
    bool isFunctionPayable();

private:
    Ui::SendToContract *ui;
    WalletModel* m_model;
    ClientModel* m_clientModel;
	TitleBar* m_sendToBar;
    TabBarInfo* m_tabInfo;
    const PlatformStyle* m_platformStyle;
    ContractTableModel* m_contractModel;
    ExecRPCCommand* m_execRPCCommand;
    ABIFunctionField* m_ABIFunctionField;
    ContractABI* m_contractABI;
    int m_results;
};

#endif // SENDTOCONTRACT_H
