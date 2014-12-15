#ifndef _WRAPER_STRUCT_
#define _WRAPER_STRUCT_

#include <node.h>

#define MSG_MAX_COUNT 200
///////////////////////////////内部使用
#define T_CONNECT_RE 1
#define T_LOGIN_RE 2
#define T_LOGOUT_RE 3
#define T_CONFIRM_RE 4
#define T_INSTRUMENT_RE 5
#define T_TRADINGACCOUNT_RE 6
#define T_INVESTORPOSITION_RE 7
#define T_INVESTORPOSITIONDETAIL_RE 8
#define T_INSERT_RE 9
#define T_ACTION_RE 10
#define T_MARGINRATE_RE 11
#define T_DEPTHMARKETDATA_RE 12
#define T_SETTLEMENTINFO_RE 13
#define T_SUBSCRIBE_MARKET_DATA_RE 14
#define T_UNSUBSCRIBE_MARKET_DATA_RE 15
#define T_DISCONNECT_RE	16
///////////////////////////////外部使用
#define T_ON_CONNECT 1
#define T_ON_DISCONNECTED 2
#define T_ON_RSPUSERLOGIN 3
#define T_ON_RSPUSERLOGOUT 4
#define T_ON_RSPINFOCONFIRM 5
#define T_ON_RSPINSERT 6
#define T_ON_ERRINSERT 7
#define T_ON_RSPACTION 8
#define T_ON_ERRACTION 9
#define T_ON_RQORDER 10
#define T_ON_RTNORDER 11
#define T_ON_RQTRADE 12
#define T_ON_RTNTRADE 13
#define T_ON_RQINVESTORPOSITION 14
#define T_ON_RQINVESTORPOSITIONDETAIL 15
#define T_ON_RQTRADINGACCOUNT 16
#define T_ON_RQINSTRUMENT 17
#define T_ON_RQDEPTHMARKETDATA 18
#define T_ON_RQSETTLEMENTINFO 19
#define T_ON_RSPERROR 20
#define T_ON_RSPSUBMARKETDATA 21
#define T_ON_RSPUNSUBMARKETDATA 22
#define T_ON_RTNDEPTHMARKETDATA 23

struct UVConnectField {
	char front_addr[200];
	char szPath[50];
	int public_topic_type;
	int private_topic_type;
};

struct LookupCtpApiBaton {
	uv_work_t work;
	void(*callback)(int, void*);
	void* uv_trader_obj;
	void* args;
	int fun;
	int iRequestID;
	int nResult;
	int uuid;//回调标识
	int nCount;
};

struct CbRtnField {
    uv_work_t work;
	int eFlag;//事件标识
	int nRequestID;
	int nReason;
	void* rtnField;
	void* rspInfo;
	bool bIsLast;
};

struct CbWrap {
	void(*callback)(CbRtnField *data);
};

struct ptrCmp
{
    bool operator()( const char * s1, const char * s2 ) const
    {
        return strcmp( s1, s2 ) < 0;
    }
};

#endif
