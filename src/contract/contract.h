#ifndef BITCOINX_CONTRACT_CONTRACT_H
#define BITCOINX_CONTRACT_CONTRACT_H

class Contract
{
public:
    static bool Enabled();
    static void SetEnabled(bool enabled);

private:
    static bool sEnabled;
};

#endif // BITCOINX_CONTRACT_CONTRACT_H