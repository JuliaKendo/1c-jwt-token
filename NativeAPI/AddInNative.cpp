
#include "stdafx.h"


#ifdef __linux__
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#endif

#include <stdio.h>
#include <wchar.h>
#include <string>
#include "AddInNative.h"

#define PICOJSON_USE_INT64 0
#include "jwt-cpp/jwt.h"

#define TIME_LEN 34

#define BASE_ERRNO     7

static wchar_t *g_PropNames[] = {L"IsEnabled"};
static wchar_t *g_MethodNames[] = {L"Enable", L"Disable", L"ShowInStatusLine", L"GenerateToken"};

static wchar_t *g_PropNamesRu[] = {L"Включен"};
static wchar_t *g_MethodNamesRu[] = {L"Включить", L"Выключить", L"ПоказатьВСтрокеСтатуса", L"СформироватьТокен"};

static const wchar_t g_kClassNames[] = L"CAddInNative"; //"|OtherClass1|OtherClass2";
static IAddInDefBase *pAsyncEvent = NULL;

uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, uint32_t len = 0);
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len = 0);
uint32_t getLenShortWcharStr(const WCHAR_T* Source);
char* get_str_param(tVariant* paParams, int idx);

//---------------------------------------------------------------------------//
long GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface)
{
    if(!*pInterface)
    {
        *pInterface= new CAddInNative;
        return (long)*pInterface;
    }
    return 0;
}
//---------------------------------------------------------------------------//
long DestroyObject(IComponentBase** pIntf)
{
   if(!*pIntf)
      return -1;

   delete *pIntf;
   *pIntf = 0;
   return 0;
}
//---------------------------------------------------------------------------//
const WCHAR_T* GetClassNames()
{
    static WCHAR_T* names = 0;
    if (!names)
        ::convToShortWchar(&names, g_kClassNames);
    return names;
}
//---------------------------------------------------------------------------//
#ifndef __linux__
VOID CALLBACK MyTimerProc(
        HWND hwnd, // handle of window for timer messages
        UINT uMsg, // WM_TIMER message
        UINT idEvent, // timer identifier
        DWORD dwTime // current system time
);
#else
static void MyTimerProc(int sig);
#endif //__linux__

// CAddInNative
//---------------------------------------------------------------------------//
CAddInNative::CAddInNative()
{
    m_iMemory = 0;
    m_iConnect = 0;
}
//---------------------------------------------------------------------------//
CAddInNative::~CAddInNative()
{
}
//---------------------------------------------------------------------------//
bool CAddInNative::Init(void* pConnection)
{ 
    m_iConnect = (IAddInDefBase*)pConnection;
    return m_iConnect != NULL;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetInfo()
{ 
    // Component should put supported component technology version 
    // This component supports 2.0 version
    return 2000; 
}
//---------------------------------------------------------------------------//
void CAddInNative::Done()
{
}
/////////////////////////////////////////////////////////////////////////////
// ILanguageExtenderBase
//---------------------------------------------------------------------------//
bool CAddInNative::RegisterExtensionAs(WCHAR_T** wsExtensionName)
{ 
    wchar_t *wsExtension = L"1c-jwt-token";
    int iActualSize = ::wcslen(wsExtension) + 1;
    WCHAR_T* dest = 0;

    if (m_iMemory)
    {
        if(m_iMemory->AllocMemory((void**)wsExtensionName, iActualSize * sizeof(WCHAR_T)))
            ::convToShortWchar(wsExtensionName, wsExtension, iActualSize);
        return true;
    }

    return false; 
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNProps()
{ 
    // You may delete next lines and add your own implementation code here
    return ePropLast;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindProp(const WCHAR_T* wsPropName)
{ 
    long plPropNum = -1;
    wchar_t* propName = 0;

    ::convFromShortWchar(&propName, wsPropName);
    plPropNum = findName(g_PropNames, propName, ePropLast);

    if (plPropNum == -1)
        plPropNum = findName(g_PropNamesRu, propName, ePropLast);

    delete[] propName;

    return plPropNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetPropName(long lPropNum, long lPropAlias)
{ 
    if (lPropNum >= ePropLast)
        return NULL;

    wchar_t *wsCurrentName = NULL;
    WCHAR_T *wsPropName = NULL;
    int iActualSize = 0;

    switch(lPropAlias)
    {
    case 0: // First language
        wsCurrentName = g_PropNames[lPropNum];
        break;
    case 1: // Second language
        wsCurrentName = g_PropNamesRu[lPropNum];
        break;
    default:
        return 0;
    }
    
    iActualSize = wcslen(wsCurrentName)+1;

    if (m_iMemory && wsCurrentName)
    {
        if (m_iMemory->AllocMemory((void**)&wsPropName, iActualSize * sizeof(WCHAR_T)))
            ::convToShortWchar(&wsPropName, wsCurrentName, iActualSize);
    }

    return wsPropName;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetPropVal(const long lPropNum, tVariant* pvarPropVal)
{ 
    switch(lPropNum)
    {
    case ePropIsEnabled:
        TV_VT(pvarPropVal) = VTYPE_BOOL;
        TV_BOOL(pvarPropVal) = m_boolEnabled;
        break;
    default:
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::SetPropVal(const long lPropNum, tVariant *varPropVal)
{ 
    switch(lPropNum)
    { 
    case ePropIsEnabled:
        if (TV_VT(varPropVal) != VTYPE_BOOL)
            return false;
        m_boolEnabled = TV_BOOL(varPropVal);
        break;
    default:
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropReadable(const long lPropNum)
{ 
    switch(lPropNum)
    { 
    case ePropIsEnabled:
        return true;
    default:
        return false;
    }

    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropWritable(const long lPropNum)
{
    switch(lPropNum)
    { 
    case ePropIsEnabled:
        return true;
    default:
        return false;
    }

    return false;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNMethods()
{ 
    return eMethLast;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindMethod(const WCHAR_T* wsMethodName)
{ 
    long plMethodNum = -1;
    wchar_t* name = 0;

    ::convFromShortWchar(&name, wsMethodName);

    plMethodNum = findName(g_MethodNames, name, eMethLast);

    if (plMethodNum == -1)
        plMethodNum = findName(g_MethodNamesRu, name, eMethLast);

    return plMethodNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetMethodName(const long lMethodNum, const long lMethodAlias)
{ 
    if (lMethodNum >= eMethLast)
        return NULL;

    wchar_t *wsCurrentName = NULL;
    WCHAR_T *wsMethodName = NULL;
    int iActualSize = 0;

    switch(lMethodAlias)
    {
    case 0: // First language
        wsCurrentName = g_MethodNames[lMethodNum];
        break;
    case 1: // Second language
        wsCurrentName = g_MethodNamesRu[lMethodNum];
        break;
    default: 
        return 0;
    }

    iActualSize = wcslen(wsCurrentName)+1;

    if (m_iMemory && wsCurrentName)
    {
        if(m_iMemory->AllocMemory((void**)&wsMethodName, iActualSize * sizeof(WCHAR_T)))
            ::convToShortWchar(&wsMethodName, wsCurrentName, iActualSize);
    }

    return wsMethodName;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNParams(const long lMethodNum)
{ 
    switch(lMethodNum)
    { 
    case eMethGenerateToken:
        return 9;
    default:
        return 0;
    }
    
    return 0;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetParamDefValue(const long lMethodNum, const long lParamNum,
                          tVariant *pvarParamDefValue)
{ 
    TV_VT(pvarParamDefValue)= VTYPE_EMPTY;

    switch(lMethodNum)
    { 
    case eMethEnable:
    case eMethDisable:
    case eMethShowInStatusLine:
        // There are no parameter values by default 
        break;
    default:
        return false;
    }

    return false;
} 
//---------------------------------------------------------------------------//
bool CAddInNative::HasRetVal(const long lMethodNum)
{ 
    switch(lMethodNum)
    { 
    case eMethGenerateToken:
        return true;
    default:
        return false;
    }

    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsProc(const long lMethodNum,
                    tVariant* paParams, const long lSizeArray)
{ 
    switch(lMethodNum)
    { 
    case eMethEnable:
        m_boolEnabled = true;
        break;
    case eMethDisable:
        m_boolEnabled = false;
        break;
    case eMethShowInStatusLine:
        if (m_iConnect && lSizeArray)
        {
            tVariant *var = paParams;
            m_iConnect->SetStatusLine(var->pwstrVal);
#ifndef __linux__
            Sleep(5000);
#else
            sleep(5);
#endif
        }
        break;

    default:
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsFunc(const long lMethodNum,
                tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{ 
    tVariant* param;
    bool ret = false;
    FILE *file = 0;
    char *name = 0;
    int size = 0;

    switch(lMethodNum)
    {
    case eMethGenerateToken:
        int32_t merchant_id = 0;
        int32_t shop_id = 0;
        int32_t pub_key_id = 0;
        double amount_sum = 0;
        char* es256_priv_key = 0;
        char* currency = 0;
        char* card_id = 0;
        char* cardholder_id = 0;
        char* sbp = 0;
        std::string token = "";
        
        {
            if (!lSizeArray || !paParams)
                return false;

            for (int i = 0; i < lSizeArray; i++) {
                switch (TV_VT(&paParams[i]))
                {
                case VTYPE_I4:
                    if (i == 1) {
                        merchant_id = TV_I4(&paParams[1]);
                        shop_id = TV_I4(&paParams[2]);
                        pub_key_id = TV_I4(&paParams[3]);
                    }
                    if (i == 4) amount_sum = TV_I4(&paParams[4]);
                    break;
                case VTYPE_R8:
                    amount_sum = TV_R8(&paParams[4]);
                    break;
                case VTYPE_PWSTR:
                    if (i == 0) {
                        es256_priv_key = get_str_param(paParams, 0);
                        currency = get_str_param(paParams, 5);
                        card_id = get_str_param(paParams, 6);
                        cardholder_id = get_str_param(paParams, 7);
                        sbp = get_str_param(paParams, 8);
                    }
                    break;
                default:
                    break;
                }
            }

        }
      
        try {

            picojson::value cert_params;
            std::string err = picojson::parse(cert_params, es256_priv_key);

            if (!err.empty()) throw err;

            const std::string cert = cert_params.get("cert").get<std::string>();
            const std::string key = cert_params.get("key").get<std::string>();

            picojson::object amountObject;
            if (amount_sum && currency[0] != '\0') {
                amountObject["value"] = picojson::value(amount_sum);
                amountObject["currency"] = picojson::value(currency);
            }

            picojson::object payoutMethodObject;
            if (card_id[0] != '\0' && cardholder_id[0] != '\0') {
                picojson::object cardObject;
                cardObject["id"] = picojson::value(card_id);
                cardObject["cardholder_id"] = picojson::value(cardholder_id);
                payoutMethodObject["card"] = picojson::value(cardObject);
            }

            if (sbp[0] != '\0') {
                picojson::value sbp_params;
                std::string err = picojson::parse(sbp_params, sbp);
                if (err.empty()) {
                    const std::string bank_member_id = sbp_params.get("bank_member_id").get<std::string>();
                    const std::string phone = sbp_params.get("phone").get<std::string>();

                    if (!bank_member_id.empty()) {
                        picojson::object sbpObject;
                        sbpObject["bank_member_id"] = picojson::value(bank_member_id);
                        if (!phone.empty()) sbpObject["phone"] = picojson::value(phone);
                        payoutMethodObject["sbp"] = picojson::value(sbpObject);
                    }

                }
            }

            if (!amountObject.empty() && !payoutMethodObject.empty())
            {

                token = jwt::create()
                    .set_type("JWT")
                    .set_id("ES256")
                    .set_payload_claim("merchant_id", picojson::value(int64_t{ merchant_id }))
                    .set_payload_claim("shop_id", picojson::value(int64_t{ shop_id }))
                    .set_payload_claim("pub_key_id", picojson::value(int64_t{ pub_key_id }))
                    .set_payload_claim("idempotency_key", picojson::value(key))
                    .set_payload_claim("amount", picojson::value(amountObject))
                    .set_payload_claim("payout_method", picojson::value(payoutMethodObject))
                    .sign(jwt::algorithm::es256("", cert, "", ""));
            }
            else if(!amountObject.empty() && payoutMethodObject.empty())
            {

                token = jwt::create()
                    .set_type("JWT")
                    .set_id("ES256")
                    .set_payload_claim("merchant_id", picojson::value(int64_t{ merchant_id }))
                    .set_payload_claim("shop_id", picojson::value(int64_t{ shop_id }))
                    .set_payload_claim("pub_key_id", picojson::value(int64_t{ pub_key_id }))
                    .set_payload_claim("idempotency_key", picojson::value(key))
                    .set_payload_claim("amount", picojson::value(amountObject))
                    .sign(jwt::algorithm::es256("", cert, "", ""));
            }
            else if (amountObject.empty() && !payoutMethodObject.empty())
            {

                token = jwt::create()
                    .set_type("JWT")
                    .set_id("ES256")
                    .set_payload_claim("merchant_id", picojson::value(int64_t{ merchant_id }))
                    .set_payload_claim("shop_id", picojson::value(int64_t{ shop_id }))
                    .set_payload_claim("pub_key_id", picojson::value(int64_t{ pub_key_id }))
                    .set_payload_claim("idempotency_key", picojson::value(key))
                    .set_payload_claim("payout_method", picojson::value(payoutMethodObject))
                    .sign(jwt::algorithm::es256("", cert, "", ""));
            }
            else
            {
                token = jwt::create()
                    .set_type("JWT")
                    .set_id("ES256")
                    .set_payload_claim("merchant_id", picojson::value(int64_t{ merchant_id }))
                    .set_payload_claim("shop_id", picojson::value(int64_t{ shop_id }))
                    .set_payload_claim("pub_key_id", picojson::value(int64_t{ pub_key_id }))
                    .set_payload_claim("idempotency_key", picojson::value(key))
                    .sign(jwt::algorithm::es256("", cert, "", ""));
            }
        }
        catch (const char* error_message) {
            size = strlen(error_message);
            m_iMemory->AllocMemory((void**)&pvarRetValue->pstrVal, size);
            TV_VT(pvarRetValue) = VTYPE_PSTR;
            memcpy((void*)pvarRetValue->pstrVal, error_message, size);
            pvarRetValue->strLen = size;

            if (es256_priv_key)
                delete[] es256_priv_key;
            if (currency)
                delete[] currency;

            return true;
        }

        char* es256_key = &token[0];
        size = strlen(es256_key);
        m_iMemory->AllocMemory((void**)&pvarRetValue->pstrVal, size);
        TV_VT(pvarRetValue) = VTYPE_PSTR;
        memcpy((void*)pvarRetValue->pstrVal, es256_key, size);
        pvarRetValue->strLen = size;

        ret = true;

        if (es256_priv_key)
            delete[] es256_priv_key;
        if (currency)
            delete[] currency;

        break;

    }
    return ret; 
}
//---------------------------------------------------------------------------//
// This code will work only on the client!
#ifndef __linux__
VOID CALLBACK MyTimerProc(
  HWND hwnd,    // handle of window for timer messages
  UINT uMsg,    // WM_TIMER message
  UINT idEvent, // timer identifier
  DWORD dwTime  // current system time
)
{
    if (!pAsyncEvent)
        return;

    wchar_t *who = L"ComponentNative", *what = L"Timer";

    wchar_t *wstime = new wchar_t[TIME_LEN];
    if (wstime)
    {
        wmemset(wstime, 0, TIME_LEN);
        ::_ultow(dwTime, wstime, 10);
        pAsyncEvent->ExternalEvent(who, what, wstime);
        delete[] wstime;
    }
}
#else
void MyTimerProc(int sig)
{
    if (pAsyncEvent)
        return;

    WCHAR_T *who = 0, *what = 0, *wdata = 0;
    wchar_t *data = 0;
    time_t dwTime = time(NULL);

    data = new wchar_t[TIME_LEN];
    
    if (data)
    {
        wmemset(data, 0, TIME_LEN);
        swprintf(data, TIME_LEN, L"%ul", dwTime);
        ::convToShortWchar(&who, L"ComponentNative");
        ::convToShortWchar(&what, L"Timer");
        ::convToShortWchar(&wdata, data);

        pAsyncEvent->ExternalEvent(who, what, wdata);

        delete[] who;
        delete[] what;
        delete[] wdata;
        delete[] data;
    }
}
#endif
//---------------------------------------------------------------------------//
void CAddInNative::SetLocale(const WCHAR_T* loc)
{
#ifndef __linux__
    //_wsetlocale(LC_ALL, loc);
#else
    //We convert in char* char_locale
    //also we establish locale
    //setlocale(LC_ALL, char_locale);
#endif
}
/////////////////////////////////////////////////////////////////////////////
// LocaleBase
//---------------------------------------------------------------------------//
bool CAddInNative::setMemManager(void* mem)
{
    m_iMemory = (IMemoryManager*)mem;
    return m_iMemory != 0;
}
//---------------------------------------------------------------------------//
void CAddInNative::addError(uint32_t wcode, const wchar_t* source, 
                        const wchar_t* descriptor, long code)
{
    if (m_iConnect)
    {
        WCHAR_T *err = 0;
        WCHAR_T *descr = 0;
        
        ::convToShortWchar(&err, source);
        ::convToShortWchar(&descr, descriptor);

        m_iConnect->AddError(wcode, err, descr, code);
        delete[] err;
        delete[] descr;
    }
}
//---------------------------------------------------------------------------//
long CAddInNative::findName(wchar_t* names[], const wchar_t* name, 
                         const uint32_t size) const
{
    long ret = -1;
    for (uint32_t i = 0; i < size; i++)
    {
        if (!wcscmp(names[i], name))
        {
            ret = i;
            break;
        }
    }
    return ret;
}
//---------------------------------------------------------------------------//
uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, uint32_t len)
{
    if (!len)
        len = ::wcslen(Source)+1;

    if (!*Dest)
        *Dest = new WCHAR_T[len];

    WCHAR_T* tmpShort = *Dest;
    wchar_t* tmpWChar = (wchar_t*) Source;
    uint32_t res = 0;

    ::memset(*Dest, 0, len*sizeof(WCHAR_T));
    do
    {
        *tmpShort++ = (WCHAR_T)*tmpWChar++;
        ++res;
    }
    while (len-- && *tmpWChar);

    return res;
}
//---------------------------------------------------------------------------//
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len)
{
    if (!len)
        len = getLenShortWcharStr(Source)+1;

    if (!*Dest)
        *Dest = new wchar_t[len];

    wchar_t* tmpWChar = *Dest;
    WCHAR_T* tmpShort = (WCHAR_T*)Source;
    uint32_t res = 0;

    ::memset(*Dest, 0, len*sizeof(wchar_t));
    do
    {
        *tmpWChar++ = (wchar_t)*tmpShort++;
        ++res;
    }
    while (len-- && *tmpShort);

    return res;
}
//---------------------------------------------------------------------------//
uint32_t getLenShortWcharStr(const WCHAR_T* Source)
{
    uint32_t res = 0;
    WCHAR_T *tmpShort = (WCHAR_T*)Source;

    while (*tmpShort++)
        ++res;

    return res;
}
//---------------------------------------------------------------------------//

char* get_str_param(tVariant* paParams, int idx) {
    int size = 0;
    char* mbstr = 0;
    wchar_t* wsTmp = 0;
    char* result = 0;

    ::convFromShortWchar(&wsTmp, TV_WSTR(&paParams[idx]));
    size = wcstombs(0, wsTmp, 0) + 1;
    mbstr = new char[size];
    memset(mbstr, 0, size);
    size = wcstombs(mbstr, wsTmp, getLenShortWcharStr(TV_WSTR(&paParams[idx])));

    return mbstr;

}
