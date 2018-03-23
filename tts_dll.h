#ifndef TTS_DLL_H
#define TTS_DLL_H
#include <QtCore>
#include <Windows.h>
#include "tts_common.h"
#include "tts_setting.h"
#include <map>
#include <memory>
#include "json.hpp"
#include "tts_common.h"
#include <QTextCodec>

using json = nlohmann::json;

typedef void (__stdcall *LPFN_OPENTDX)();
typedef void (__stdcall *LPFN_CLOSETDX)();

typedef int (__stdcall *LPFN_LOGON)(const char* IP, const short Port, const char* Version, short YybID,  const char* AccountNo, const char* TradeAccount, const char* JyPassword, const char* TxPassword, char* ErrInfo);
typedef void(__stdcall *LPFN_LOGOFF)(int ClientID);
typedef void(__stdcall *LPFN_QUERYDATA)(int ClientID, int Category, char* result, char* errInfo);
typedef void(__stdcall *LPFN_SENDORDER)(int ClientID, int Category ,int PriceType,  const char* Gddm,  const char* Zqdm , float Price, int Quantity,  char* Result, char* ErrInfo);
typedef void(__stdcall *LPFN_CANCELORDER)(int ClientID, const char* ExchangeID, const char* hth, char* Result, char* ErrInfo);
typedef void(__stdcall *LPFN_GETQUOTE)(int ClientID, const char* Zqdm, char* Result, char* ErrInfo);
typedef void(__stdcall *LPFN_REPAY)(int ClientID, const char* Amount, char* Result, char* ErrInfo);
typedef void(__stdcall *LPFN_QUERYHISTORYDATA)(int ClientID, int Category, const char* BeginDate, const char* EndDate, char* Result, char* ErrInfo); //QueryHistoryData
typedef void(__stdcall *LPFN_SENDORDERS)(int clientId, int categories[], int priceTypes[], const char* gddms, const char* zqdms, float prices[], int quantities, int count, char** results, char** errInfos); // SendOrders
typedef void(__stdcall *LPFN_QUERYDATAS)(int clientId, int categories[], int count, char** results, char** errInfos);

#define P_LOGON         "logon"
#define P_LOGOFF        "logoff"
#define P_QUERYDATA     "query_data"
#define P_SENDORDER     "send_order"
#define P_CANCELORDER   "cancel_order"
#define P_GETQUOTE      "get_quote"
#define P_REPAY         "repay"
#define P_QUERYHISTORYDATA "query_history_data"
#define P_GETACTIVECLIENTS "get_active_clients"
#define P_SENDORDERS    "send_orders"
#define P_QUERYDATAS    "query_datas"

class TTS_Dll
{
private:
    HINSTANCE hDLL;
    TTS_Dll(const TTS_SettingObject& so, const std::string& accountNo);
    bool initSuccess;
    // store error and result;
    char* errout;
    char* result;

    /// api far call
    LPFN_OPENTDX lpOpenTdx;
    LPFN_CLOSETDX lpCloseTdx;
    LPFN_LOGON lpLogon;
    LPFN_LOGOFF lpLogoff;
    LPFN_QUERYDATA lpQueryData;
    LPFN_SENDORDER lpSendOrder;
    LPFN_CANCELORDER lpCancelOrder;
    LPFN_GETQUOTE lpGetQuote;
    LPFN_REPAY lpRepay;
    LPFN_QUERYHISTORYDATA lpQueryHistoryData;
    LPFN_SENDORDERS lpSendOrders;
    LPFN_QUERYDATAS lpQueryDatas;

    QMutex apiCallMutex; // add lock to all network call
    bool outputUtf8;

    void setupErrForJson(const char* errout, json& resultJSON);
    uint32_t    seq;
    void allocResultsAndErrorInfos(int count, /* out */ char**& results, /* out */ char**& errorInfos);
    void freeResulsAndErrorInfos(int count, /* out */ char**& results, /* out */ char**& errorInfos);
protected:
    json convertTableToJSON(const char* result, const char* errout);
    json convertMultiTableToJSON(int count, char**& results, char**& errout);

    void _strToTable(const char *result, json& j);

public:
    ~TTS_Dll();
    std::string makeSig(const std::string& accountNo);
    void setOutputUtf8(bool utf8);

    json logon(const char* IP, const short Port,
              const char* Version, short YybID,
              const char* AccountNo, const char* TradeAccount,
              const char* JyPassword, const char* TxPassword);

    json logoff(int ClientID);
    json queryData(int ClientID, int Category);
    json sendOrder(int ClientID, int Category ,int PriceType, const char* Gddm, const char* Zqdm , float Price, int Quantity);
    json cancelOrder(int ClientID, const char* ExchangeID, const char* hth);
    json getQuote(int ClientID, const char* Zqdm);
    json repay(int ClientID, const char* Amount);
    json queryHistoryData(int ClientID, int Category, const char* BeginDate, const char* EndDate);
    json sendOrders(int clientId, int categories[], int priceTypes[], const char* gddms, const char* zqdms, float prices[], int quantities, int count);
    json queryDatas(int clientId, int categories[], int count);
    const uint32_t getSeq() const {return seq; }

// 实现一个多例模式，针对不同的帐号名，返回不同的
private:
     static std::map<std::string, std::shared_ptr<TTS_Dll>> dlls;
     static std::map<uint32_t, std::string> seqAccountMapping;
     static QMutex initMutex; // add lock init stage
     static QMutex seqMutex;
public:
     static std::shared_ptr<TTS_Dll> getInstance(const TTS_SettingObject& so, const std::string& accountNo);
     static std::shared_ptr<TTS_Dll> getInstance(const TTS_SettingObject& so, const uint32_t seqNo);
     static void preloadDlls(TTS_SettingObject& so);
     const static std::map<std::string, std::shared_ptr<TTS_Dll>>& allDlls() {
        return dlls;
     }
     static volatile uint32_t maxSeq; // 最大id, 从0开始

};

#endif // TTS_DLL_H
