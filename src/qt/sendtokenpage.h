#ifndef SENDTOKENPAGE_H
#define SENDTOKENPAGE_H

#include <QWidget>
class WalletModel;
class Token;
class ClientModel;

struct SelectedToken;

namespace Ui {
class SendTokenPage;
}

class SendTokenPage : public QWidget
{
    Q_OBJECT

public:
    explicit SendTokenPage(QWidget *parent = 0);
    ~SendTokenPage();

    bool isDataValid();
    void initGasPriceStyle();
    void setTokenData(std::string address, std::string sender, std::string symbol, int8_t decimals, std::string balance);
	 void setModel(WalletModel *_model);
    void setClientModel(ClientModel *clientModel);
    void clearAll();
    bool isValidAddress();

private Q_SLOTS:
    void on_updateConfirmButton();
    void on_confirmClicked();
    void on_clearButton_clicked();
    void on_numBlocksChanged();


private:
    Ui::SendTokenPage *ui;
    WalletModel* m_model;
    ClientModel* m_clientModel;
    SelectedToken *m_selectedToken;
	Token *m_tokenABI;
};

#endif // SENDTOKENPAGE_H
