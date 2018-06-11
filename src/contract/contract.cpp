#include "contract.h"
#include "sync.h"

static CCriticalSection cs_contract;

bool Contract::sEnabled = false;

bool Contract::Enabled()
{
    LOCK(cs_contract);
    return sEnabled;
}

void Contract::SetEnabled(bool enabled)
{
    LOCK(cs_contract);
    sEnabled = enabled;
}
