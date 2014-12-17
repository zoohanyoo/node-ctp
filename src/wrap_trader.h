#ifndef N_TRADER_H_
#define N_TRADER_H_

#include "stdafx.h"
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <node.h>
#include "ThostFtdcTraderApi.h"
#include "ThostFtdcUserApiDataType.h"
#include <uv.h>
#include "uv_trader.h"

using namespace v8;

extern bool islog;
extern void logger_cout(const char* content);
extern std::string to_string(int val);
extern std::string charto_string(char val);

class WrapTrader : public node::ObjectWrap {
public:
	WrapTrader(void);
	~WrapTrader(void);

	///连接前置机
	static Handle<Value> Connect(const Arguments& args);	
	///注册事件
	static Handle<Value> On(const Arguments& args);
	///用户登录请求
	static Handle<Value> ReqUserLogin(const Arguments& args);
	///登出请求 
	static Handle<Value> ReqUserLogout(const Arguments& args);
	///投资者结算结果确认
	static Handle<Value> ReqSettlementInfoConfirm(const Arguments& args);
	///请求查询合约
	static Handle<Value> ReqQryInstrument(const Arguments& args);
	///请求查询资金账户
	static Handle<Value> ReqQryTradingAccount(const Arguments& args);
	///请求查询投资者持仓
	static Handle<Value> ReqQryInvestorPosition(const Arguments& args);
	///持仓明细
	static Handle<Value> ReqQryInvestorPositionDetail(const Arguments& args);
	///报单录入请求
	static Handle<Value> ReqOrderInsert(const Arguments& args);
	///报单操作请求
	static Handle<Value> ReqOrderAction(const Arguments& args);
	///请求查询合约保证金率 
	static Handle<Value> ReqQryInstrumentMarginRate(const Arguments& args);
	///请求查询行情 
	static Handle<Value> ReqQryDepthMarketData(const Arguments& args);
	///请求查询投资者结算结果 
	static Handle<Value> ReqQrySettlementInfo(const Arguments& args);
	///删除接口对象
	static Handle<Value> Disposed(const Arguments& args);
	//对象初始化
	static void Init(int args);
	static Handle<Value> NewInstance(const Arguments& args);
    static Handle<Value> GetTradingDay(const Arguments& args);

private:
	static void initEventMap();	
	static Handle<Value> New(const Arguments& args);
	static void pkg_cb_userlogin(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_userlogout(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_confirm(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_orderinsert(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_errorderinsert(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_orderaction(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_errorderaction(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rspqryorder(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rtnorder(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rqtrade(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rtntrade(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rqinvestorposition(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rqinvestorpositiondetail(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rqtradingaccount(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rqinstrument(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rqdepthmarketdata(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rqsettlementinfo(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rsperror(CbRtnField* data, Local<Value>*cbArray);

	static Local<Value> pkg_rspinfo(void *vpRspInfo);
	uv_trader* uvTrader;
	static int s_uuid;
	static void FunCallback(CbRtnField *data);
	static void FunRtnCallback(int result, void* baton);
	static Persistent<Function> constructor;
	static std::map<const char*, int,ptrCmp> event_map;
	static std::map<int, Persistent<Function> > callback_map;
	static std::map<int, Persistent<Function> > fun_rtncb_map; 	
};



#endif
