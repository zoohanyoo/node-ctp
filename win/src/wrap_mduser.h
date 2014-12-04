#ifndef N_MDUSER_H_
#define N_MDUSER_H_

#include "stdafx.h"
#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <map>
#include <fstream>
#include <atomic>
#include <node.h>
#include "ThostFtdcTraderApi.h"
#include "ThostFtdcUserApiDataType.h"
#include <uv.h>
#include "uv_mduser.h"

using namespace v8;

extern bool islog;
extern void logger_cout(const char* content);

class WrapMdUser :public node::ObjectWrap {
public:
	WrapMdUser(void);
	~WrapMdUser(void);

	///连接前置机
	static Handle<Value> Connect(const Arguments& args);
	///注册事件
	static Handle<Value> On(const Arguments& args);
	///用户登录请求
	static Handle<Value> ReqUserLogin(const Arguments& args);
	///登出请求 
	static Handle<Value> ReqUserLogout(const Arguments& args);
	///
	static Handle<Value>  SubscribeMarketData(const Arguments& args);

	static Handle<Value>  UnSubscribeMarketData(const Arguments& args);
	///删除接口对象
	static Handle<Value> Disposed(const Arguments& args);
	//对象初始化
	static void Init(int args);
	static Handle<Value> NewInstance(const Arguments& args);

private:
	static void initEventMap();
	static Handle<Value> New(const Arguments& args);
	static void pkg_cb_userlogin(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_userlogout(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rspsubmarketdata(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_unrspsubmarketdata(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rtndepthmarketdata(CbRtnField* data, Local<Value>*cbArray); 
	static void pkg_cb_rsperror(CbRtnField* data, Local<Value>*cbArray);
	static Local<Value> pkg_rspinfo(CThostFtdcRspInfoField *pRspInfo);

	uv_mduser* uvMdUser;
	static std::atomic_int s_uuid;
	static void FunCallback(CbRtnField *data);
	static void FunRtnCallback(int result, void* baton);
	static Persistent<Function> constructor;
	static std::map<std::string, int> event_map;
	static std::map<int, Persistent<Function>> callback_map;
	static std::map<int, Persistent<Function>> fun_rtncb_map;
};


#endif