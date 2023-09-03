//Получить следующие параметры процессора: имя, производитель, максимальная частота

#include <windows.h>
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>
#include <locale.h>
#pragma comment(lib, "wbemuuid.lib")
using namespace std;

int main()
{
    setlocale(LC_ALL, "Russian");

    HRESULT hres;

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        cout << "Не удалось инициализировать COM." << endl;
        return 1;
    }

    hres = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL
    );
    if (FAILED(hres))
    {
        cout << "Не удалось инициализировать безопасность COM." << endl;
        CoUninitialize();
        return 1;
    }

    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&pLoc
    );
    if (FAILED(hres))
    {
        cout << "Не удалось создать IWbemLocator объект." << endl;
        CoUninitialize();
        return 1;
    }

    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        &pSvc
    );
    if (FAILED(hres))
    {
        cout << "Не удалось подключиться к ROOT\\CIMV2 пространству имен." << endl;
        pLoc->Release();
        CoUninitialize();
        return 1;
    }

    hres = CoSetProxyBlanket(
        pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
    );
    if (FAILED(hres))
    {
        cout << "Не удалось установить параметры проверки безопасности." << endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return 1;
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_Processor"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator
    );
    if (FAILED(hres))
    {
        cout << "Не удалось выполнить запрос." << endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return 1;
    }

    IWbemClassObject* pclsObj;
    ULONG uReturn = 0;

    while (pEnumerator)
    {
        hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (uReturn == 0) break;

        VARIANT vtProp{};

        hres = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
        wprintf(L"Имя процессора: %s\n\n", vtProp.bstrVal);
        VariantClear(&vtProp);

        hres = pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0);
        wprintf(L"Производитель процессора: %s\n\n", vtProp.bstrVal);
        VariantClear(&vtProp);

        hres = pclsObj->Get(L"MaxClockSpeed", 0, &vtProp, 0, 0);
        wprintf(L"Максимальная тактовая частота: %u МГц\n\n", vtProp.uintVal);
        VariantClear(&vtProp);

        pclsObj->Release();
    }

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();

    return 0;
}
