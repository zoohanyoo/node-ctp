#ifndef _UV_MDUSER_
#define _UV_MDUSER_

#include "stdafx.h"
#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include "ThostFtdcMdApi.h"
#include "ThostFtdcUserApiDataType.h"
#include <uv.h>
#include <node.h>
#include "wraper_struct.h"

extern void logger_cout(const char* content);
extern std::string to_string(int val);

class uv_mduser :public CThostFtdcMdSpi {
public:
	uv_mduser(void);
	~uv_mduser(void);

	///注册事件
	int On(const char* eName, int cb_type, void(*callback)(CbRtnField* cbResult));
	///连接前置机
	void  Connect(UVConnectField* pConnectField, void(*callback)(int, void*), int uuid);
	void  ReqUserLogin(CThostFtdcReqUserLoginField *pReqUserLoginField, void(*callback)(int, void*), int uuid);
	void  ReqUserLogout(CThostFtdcUserLogoutField *pUserLogout, void(*callback)(int, void*), int uuid);
	void  SubscribeMarketData(char *ppInstrumentID[], int nCount, void(*callback)(int, void*), int uuid);
	void  UnSubscribeMarketData(char *ppInstrumentID[], int nCount, void(*callback)(int, void*), int uuid);
	void  Disposed(); 	

private:
	///异步调用 queue
	static void _async(uv_work_t * work);
	///异步调用完成 queue
	static void _completed(uv_work_t * work, int);

    static void _on_async(uv_work_t * work);

    static void _on_completed(uv_work_t * work,int);
	///调用ctp api
	void invoke(void* field, int count, int ret, void(*callback)(int, void*), int uuid);

	void on_invoke(int event_type, void* _stru, CThostFtdcRspInfoField *pRspInfo_org, int nRequestID, bool bIsLast);
	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用
	virtual void OnFrontConnected();
	///连接断开时，该方法被调用
	///@param nReason 错误原因
	///        0x1001 网络读失败
	///        0x1002 网络写失败
	///        0x2001 接收心跳超时
	///        0x2002 发送心跳失败
	///        0x2003 收到错误报文
	virtual void OnFrontDisconnected(int nReason);
	///登录请求响应 
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///登出请求响应 
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///错误应答 
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///订阅行情应答 
	virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///取消订阅行情应答 
	virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///深度行情通知 
	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

	CThostFtdcMdApi* m_pApi;
	int iRequestID;
    uv_async_t async_t;
	static std::map<int, CbWrap*> cb_map;
};



#endif
