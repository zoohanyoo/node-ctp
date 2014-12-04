#include <uv.h>
#include "uv_trader.h"
#include "ThostFtdcTraderApi.h"
#include "ThostFtdcUserApiDataType.h"

std::map<int, uv_async_t*> uv_trader::async_map;
std::map<int, CbWrap*> uv_trader::cb_map;

void logger_cout(const char* content) {
	using namespace std;
	if (islog) {
		time_t t;
		t = time(NULL);
		tm* ptr = localtime(&t);
		char str[20];
		strftime(str, 20, "%Y-%m-%d %H:%M:%S", ptr);
		string log(str);
		log.append("-DEBUG:");
		log.append(content);
		cout << log.c_str() << endl;
	}
}

uv_trader::uv_trader(void) {
	iRequestID = 0;
}
uv_trader::~uv_trader(void) {

}

void uv_trader::Disconnect() {
	m_pApi->RegisterSpi(NULL);
	m_pApi->Release();
	m_pApi = NULL;

	std::map<int, CbWrap*>::iterator callback_it = cb_map.begin();
	while (callback_it != cb_map.end()) {
	    delete callback_it->second;
		callback_it++;
	}

    std::map<int, uv_async_t*>::iterator uvasync_it = async_map.begin();
	while (uvasync_it != async_map.end()) {
		uv_close((uv_handle_s*)uvasync_it->second, NULL);
		uvasync_it++;
	}
	logger_cout("uv_trader object destroyed");
}
int uv_trader::On(int cb_type, void(*callback)(CbRtnField* cbResult)) {
	std::string log = "on function";
	std::map<int, uv_async_t*>::iterator it = async_map.find(cb_type);
	if (it != async_map.end()) {
		logger_cout(log.append(" event id").append(to_string(cb_type)).append(" register repeat").c_str());
		return 1;//Callback is defined before
	}
	
	uv_async_t* s_async = new uv_async_t();//析构函数中需要销毁
	uv_async_init(uv_default_loop(), s_async, completeCb);
	async_map[cb_type] = s_async;

	CbWrap* cb_wrap = new CbWrap();//析构函数中需要销毁
	cb_wrap->callback = callback;
	cb_map[cb_type] = cb_wrap;
	logger_cout(log.append(" event id ").append(to_string(cb_type)).append(" register").c_str());
	return 0;
}
void uv_trader::Connect(UVConnectField* pConnectField, void(*callback)(int, void*), int uuid) {
	UVConnectField* _pConnectField = new UVConnectField();
	memcpy(_pConnectField, pConnectField, sizeof(UVConnectField));
	this->invoke(_pConnectField, T_CONNECT_RE, callback, uuid);//pConnectField函数外部销毁
}
void uv_trader::ReqUserLogin(CThostFtdcReqUserLoginField *pReqUserLoginField, void(*callback)(int, void*), int uuid) {
	CThostFtdcReqUserLoginField *_pReqUserLoginField = new CThostFtdcReqUserLoginField();
	memcpy(_pReqUserLoginField, pReqUserLoginField, sizeof(CThostFtdcReqUserLoginField));
	this->invoke(_pReqUserLoginField, T_LOGIN_RE, callback, uuid);
}
void uv_trader::ReqUserLogout(CThostFtdcUserLogoutField *pUserLogout, void(*callback)(int, void*), int uuid) {
	CThostFtdcUserLogoutField* _pUserLogout = new CThostFtdcUserLogoutField();
	memcpy(_pUserLogout, pUserLogout, sizeof(CThostFtdcUserLogoutField));
	this->invoke(_pUserLogout, T_LOGOUT_RE, callback, uuid);
}
void uv_trader::ReqSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, void(*callback)(int, void*), int uuid) {
	CThostFtdcSettlementInfoConfirmField* _pSettlementInfoConfirm = new CThostFtdcSettlementInfoConfirmField();
	memcpy(_pSettlementInfoConfirm, pSettlementInfoConfirm, sizeof(CThostFtdcSettlementInfoConfirmField));
	this->invoke(_pSettlementInfoConfirm, T_CONFIRM_RE, callback, uuid);
}
void uv_trader::ReqQryInstrument(CThostFtdcQryInstrumentField *pQryInstrument, void(*callback)(int, void*), int uuid) {
	CThostFtdcQryInstrumentField *_pQryInstrument = new CThostFtdcQryInstrumentField();
	memcpy(_pQryInstrument, pQryInstrument, sizeof(CThostFtdcQryInstrumentField));
	this->invoke(_pQryInstrument, T_INSTRUMENT_RE, callback, uuid);
}
void uv_trader::ReqQryTradingAccount(CThostFtdcQryTradingAccountField *pQryTradingAccount, void(*callback)(int, void*), int uuid) {
	CThostFtdcQryTradingAccountField *_pQryTradingAccount = new CThostFtdcQryTradingAccountField();
	memcpy(_pQryTradingAccount, pQryTradingAccount, sizeof(CThostFtdcQryTradingAccountField));
	this->invoke(_pQryTradingAccount, T_TRADINGACCOUNT_RE, callback, uuid);
}
void uv_trader::ReqQryInvestorPosition(CThostFtdcQryInvestorPositionField *pQryInvestorPosition, void(*callback)(int, void*), int uuid) {
	CThostFtdcQryInvestorPositionField *_pQryInvestorPosition = new CThostFtdcQryInvestorPositionField();
	memcpy(_pQryInvestorPosition, pQryInvestorPosition, sizeof(CThostFtdcQryInvestorPositionField));
	this->invoke(_pQryInvestorPosition, T_INVESTORPOSITION_RE, callback, uuid);
}
void uv_trader::ReqQryInvestorPositionDetail(CThostFtdcQryInvestorPositionDetailField *pQryInvestorPositionDetail, void(*callback)(int, void*), int uuid) {
	CThostFtdcQryInvestorPositionDetailField *_pQryInvestorPositionDetail = new CThostFtdcQryInvestorPositionDetailField();
	memcpy(_pQryInvestorPositionDetail, pQryInvestorPositionDetail, sizeof(CThostFtdcQryInvestorPositionDetailField));
	this->invoke(_pQryInvestorPositionDetail, T_INVESTORPOSITIONDETAIL_RE, callback, uuid);
}
void uv_trader::ReqOrderInsert(CThostFtdcInputOrderField *pInputOrder, void(*callback)(int, void*), int uuid) {
	CThostFtdcInputOrderField *_pInputOrder = new CThostFtdcInputOrderField();
	memcpy(_pInputOrder, pInputOrder, sizeof(CThostFtdcInputOrderField));
	this->invoke(_pInputOrder, T_INSERT_RE, callback, uuid);
}
void uv_trader::ReqOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, void(*callback)(int, void*), int uuid) {
	CThostFtdcInputOrderActionField *_pInputOrderAction = new CThostFtdcInputOrderActionField();
	memcpy(_pInputOrderAction, pInputOrderAction, sizeof(CThostFtdcInputOrderActionField));
	this->invoke(_pInputOrderAction, T_ACTION_RE, callback, uuid);
}
void uv_trader::ReqQryInstrumentMarginRate(CThostFtdcQryInstrumentMarginRateField *pQryInstrumentMarginRate, void(*callback)(int, void*), int uuid) {
	CThostFtdcQryInstrumentMarginRateField *_pQryInstrumentMarginRate = new CThostFtdcQryInstrumentMarginRateField();
	memcpy(_pQryInstrumentMarginRate, pQryInstrumentMarginRate, sizeof(CThostFtdcQryInstrumentMarginRateField));
	this->invoke(_pQryInstrumentMarginRate, T_MARGINRATE_RE, callback, uuid);
}
void uv_trader::ReqQryDepthMarketData(CThostFtdcQryDepthMarketDataField *pQryDepthMarketData, void(*callback)(int, void*), int uuid) {
	CThostFtdcQryDepthMarketDataField *_pQryDepthMarketData = new CThostFtdcQryDepthMarketDataField();
	memcpy(_pQryDepthMarketData, pQryDepthMarketData, sizeof(CThostFtdcQryDepthMarketDataField));
	this->invoke(_pQryDepthMarketData, T_DEPTHMARKETDATA_RE, callback, uuid);
}
void uv_trader::ReqQrySettlementInfo(CThostFtdcQrySettlementInfoField *pQrySettlementInfo, void(*callback)(int, void*), int uuid) {
	CThostFtdcQrySettlementInfoField *_pQrySettlementInfo = new CThostFtdcQrySettlementInfoField();
	memcpy(_pQrySettlementInfo, pQrySettlementInfo, sizeof(CThostFtdcQrySettlementInfoField));
	this->invoke(_pQrySettlementInfo, T_SETTLEMENTINFO_RE, callback, uuid);
}

void uv_trader::OnFrontConnected() {		
	std::map<int, uv_async_t*>::iterator it = async_map.find(T_ON_CONNECT);
	if (it != async_map.end()) {		
		CbRtnField* field = new CbRtnField();//调用完毕后需要销毁
		field->eFlag = T_ON_CONNECT;//FrontConnected
		it->second->data = field;//对象销毁后，指针清空
		uv_async_send(it->second);
	}
}
void uv_trader::OnFrontDisconnected(int nReason) {
	std::map<int, uv_async_t*>::iterator it = async_map.find(T_ON_DISCONNECTED);
	if (it != async_map.end()) {
		CbRtnField* field = new CbRtnField();//调用完毕后需要销毁
		field->eFlag = T_ON_DISCONNECTED;//FrontConnected
		field->nReason = nReason;
		it->second->data = field;//对象销毁后，指针清空
		uv_async_send(it->second);
	}
}
void uv_trader::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	CThostFtdcRspUserLoginField* _pRspUserLogin = NULL;
	if (pRspUserLogin) {
		_pRspUserLogin = new CThostFtdcRspUserLoginField();
		memcpy(_pRspUserLogin, pRspUserLogin, sizeof(CThostFtdcRspUserLoginField));
	}
	pkg_senduv(T_ON_RSPUSERLOGIN, _pRspUserLogin, pRspInfo, nRequestID, bIsLast);
}
void uv_trader::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	CThostFtdcUserLogoutField* _pUserLogout = NULL;
	if (pUserLogout) {
		_pUserLogout = new CThostFtdcUserLogoutField();
		memcpy(_pUserLogout, pUserLogout, sizeof(CThostFtdcUserLogoutField));
	}
	pkg_senduv(T_ON_RSPUSERLOGOUT, _pUserLogout, pRspInfo, nRequestID, bIsLast);
}
void uv_trader::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	CThostFtdcUserLogoutField* _pSettlementInfoConfirm = NULL;
	if (pSettlementInfoConfirm) {
		_pSettlementInfoConfirm = new CThostFtdcUserLogoutField();
		memcpy(_pSettlementInfoConfirm, pSettlementInfoConfirm, sizeof(CThostFtdcSettlementInfoConfirmField));
	}
	pkg_senduv(T_ON_RSPINFOCONFIRM, _pSettlementInfoConfirm, pRspInfo, nRequestID, bIsLast);
}
void uv_trader::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	CThostFtdcInputOrderField* _pInputOrder = NULL;
	if (pInputOrder) {
		_pInputOrder = new CThostFtdcInputOrderField();
		memcpy(_pInputOrder, pInputOrder, sizeof(CThostFtdcInputOrderField));
	}
	pkg_senduv(T_ON_RSPINSERT, _pInputOrder, pRspInfo, nRequestID, bIsLast);
}
void uv_trader::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo) {
	CThostFtdcInputOrderField* _pInputOrder = NULL;
	if (pInputOrder) {
		_pInputOrder = new CThostFtdcInputOrderField();
		memcpy(_pInputOrder, pInputOrder, sizeof(CThostFtdcInputOrderField));
	}
	pkg_senduv(T_ON_ERRINSERT, _pInputOrder, pRspInfo, 0, 0);
}
void uv_trader::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	CThostFtdcInputOrderActionField* _pInputOrderAction = NULL;
	if (pInputOrderAction) {
		_pInputOrderAction = new CThostFtdcInputOrderActionField();
		memcpy(_pInputOrderAction, pInputOrderAction, sizeof(CThostFtdcInputOrderActionField));
	}
	pkg_senduv(T_ON_RSPACTION, _pInputOrderAction, pRspInfo, nRequestID, bIsLast);
}
void uv_trader::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo) {
	CThostFtdcOrderActionField* _pOrderAction = NULL;
	if (pOrderAction) {
		_pOrderAction = new CThostFtdcOrderActionField();
		memcpy(_pOrderAction, pOrderAction, sizeof(CThostFtdcOrderActionField));
	}
	pkg_senduv(T_ON_ERRACTION, _pOrderAction, pRspInfo, 0, 0);
}
void uv_trader::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	CThostFtdcOrderField* _pOrder = NULL;
	if (pOrder) {
		_pOrder = new CThostFtdcOrderField();
		memcpy(_pOrder, pOrder, sizeof(CThostFtdcOrderField));
	}
	pkg_senduv(T_ON_RQORDER, _pOrder, pRspInfo, nRequestID, bIsLast);
}
void uv_trader::OnRtnOrder(CThostFtdcOrderField *pOrder) {
	CThostFtdcOrderField* _pOrder = NULL;
	if (pOrder) {
		_pOrder = new CThostFtdcOrderField();
		memcpy(_pOrder, pOrder, sizeof(CThostFtdcOrderField));
	}
	pkg_senduv(T_ON_RTNORDER, _pOrder, new CThostFtdcRspInfoField(), 0, 0);
}
void uv_trader::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	CThostFtdcTradeField* _pTrade = NULL;
	if (pTrade) {
		_pTrade = new CThostFtdcTradeField();
		memcpy(_pTrade, pTrade, sizeof(CThostFtdcTradeField));
	}
	pkg_senduv(T_ON_RQTRADE, _pTrade, pRspInfo, nRequestID, bIsLast);
}
void uv_trader::OnRtnTrade(CThostFtdcTradeField *pTrade) {
	CThostFtdcTradeField* _pTrade = NULL;
	if (pTrade) {
		_pTrade = new CThostFtdcTradeField();
		memcpy(_pTrade, pTrade, sizeof(CThostFtdcTradeField));
	}
	pkg_senduv(T_ON_RTNTRADE, _pTrade, new CThostFtdcRspInfoField(), 0, 0);
}
void uv_trader::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	CThostFtdcInvestorPositionField* _pInvestorPosition = NULL;
	if (pInvestorPosition) {
		_pInvestorPosition = new CThostFtdcInvestorPositionField();
		memcpy(_pInvestorPosition, pInvestorPosition, sizeof(CThostFtdcInvestorPositionField));
	}
	pkg_senduv(T_ON_RQINVESTORPOSITION, _pInvestorPosition, pRspInfo, nRequestID, bIsLast);
}
void uv_trader::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	CThostFtdcInvestorPositionDetailField* _pInvestorPositionDetail = NULL;
	if (pInvestorPositionDetail) {
		_pInvestorPositionDetail = new CThostFtdcInvestorPositionDetailField();
		memcpy(_pInvestorPositionDetail, pInvestorPositionDetail, sizeof(CThostFtdcInvestorPositionDetailField));
	}
	pkg_senduv(T_ON_RQINVESTORPOSITIONDETAIL, _pInvestorPositionDetail, pRspInfo, nRequestID, bIsLast);
}
void uv_trader::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	CThostFtdcTradingAccountField *_pTradingAccount = NULL;
	if (pTradingAccount) {
		_pTradingAccount = new CThostFtdcTradingAccountField();
		memcpy(_pTradingAccount, pTradingAccount, sizeof(CThostFtdcTradingAccountField));
	}
	pkg_senduv(T_ON_RQTRADINGACCOUNT, _pTradingAccount, pRspInfo, nRequestID, bIsLast);
}
void uv_trader::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	CThostFtdcInstrumentField *_pInstrument = NULL;
	if (pInstrument) {
		_pInstrument = new CThostFtdcInstrumentField();
		memcpy(_pInstrument, pInstrument, sizeof(CThostFtdcInstrumentField));
	}
	pkg_senduv(T_ON_RQINSTRUMENT, _pInstrument, pRspInfo, nRequestID, bIsLast);
}
void uv_trader::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	CThostFtdcDepthMarketDataField* _pDepthMarketData = NULL;
	if (pDepthMarketData) {
		_pDepthMarketData = new CThostFtdcDepthMarketDataField();
		memcpy(pDepthMarketData, _pDepthMarketData, sizeof(CThostFtdcDepthMarketDataField));
	}
	pkg_senduv(T_ON_RQDEPTHMARKETDATA, _pDepthMarketData, pRspInfo, nRequestID, bIsLast);
}
void uv_trader::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	CThostFtdcSettlementInfoField* _pSettlementInfo = NULL;
	if (pSettlementInfo) {
		_pSettlementInfo = new CThostFtdcSettlementInfoField();
		memcpy(_pSettlementInfo, pSettlementInfo, sizeof(CThostFtdcSettlementInfoField));
	}
	pkg_senduv(T_ON_RQSETTLEMENTINFO, _pSettlementInfo, pRspInfo, nRequestID, bIsLast);
}
void uv_trader::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	CThostFtdcRspInfoField* _pRspInfo = NULL;
	if (pRspInfo) {
		_pRspInfo = new CThostFtdcRspInfoField();
		memcpy(_pRspInfo, pRspInfo, sizeof(CThostFtdcRspInfoField));
	}
	pkg_senduv(T_ON_RSPERROR, _pRspInfo, pRspInfo, nRequestID, bIsLast);
}
void uv_trader::_async(uv_work_t * work) {
	LookupCtpApiBaton* baton = static_cast<LookupCtpApiBaton*>(work->data);
	uv_trader* uv_trader_obj = static_cast<uv_trader*>(baton->uv_trader_obj);
	std::string log = "uv_trader uv_queue async run,";
	switch (baton->fun) {
	case T_CONNECT_RE:
	{
						 UVConnectField* _pConnectF = static_cast<UVConnectField*>(baton->args);
						 uv_trader_obj->m_pApi = CThostFtdcTraderApi::CreateFtdcTraderApi(_pConnectF->szPath);
						 uv_trader_obj->m_pApi->RegisterSpi(uv_trader_obj);			
						 uv_trader_obj->m_pApi->SubscribePublicTopic(static_cast<THOST_TE_RESUME_TYPE>(_pConnectF->public_topic_type));
						 uv_trader_obj->m_pApi->SubscribePrivateTopic(static_cast<THOST_TE_RESUME_TYPE>(_pConnectF->private_topic_type));
						 uv_trader_obj->m_pApi->RegisterFront(_pConnectF->front_addr);
						 uv_trader_obj->m_pApi->Init();
						 logger_cout(log.append("ftdc_trader_api init successed").c_str());
						 break;
	}
	case T_LOGIN_RE:
	{
					   CThostFtdcReqUserLoginField *_pReqUserLoginField = static_cast<CThostFtdcReqUserLoginField*>(baton->args);
					   baton->nResult = uv_trader_obj->m_pApi->ReqUserLogin(_pReqUserLoginField, baton->iRequestID);
					   logger_cout(log.append("ftdc_trader_api ReqUserLogin run,the result:").append(to_string(baton->nResult)).c_str());
					   break;
	}
	case T_LOGOUT_RE:
	{
						CThostFtdcUserLogoutField* _pUserLogout = static_cast<CThostFtdcUserLogoutField*>(baton->args);
						baton->nResult = uv_trader_obj->m_pApi->ReqUserLogout(_pUserLogout, baton->iRequestID);
						logger_cout(log.append("ftdc_trader_api ReqUserLogout run,the result:").append(to_string(baton->nResult)).c_str());

						break;
	}
	case T_CONFIRM_RE:
	{
						 CThostFtdcSettlementInfoConfirmField* _pSettlementInfoConfirm = static_cast<CThostFtdcSettlementInfoConfirmField*>(baton->args);
						 baton->nResult = uv_trader_obj->m_pApi->ReqSettlementInfoConfirm(_pSettlementInfoConfirm, baton->iRequestID);
						 logger_cout(log.append("ftdc_trader_api ReqSettlementInfoConfirm run,the result:").append(to_string(baton->nResult)).c_str());

						 break;
	}
	case T_INSTRUMENT_RE:
	{
							CThostFtdcQryInstrumentField *_pQryInstrument = static_cast<CThostFtdcQryInstrumentField*>(baton->args);
							baton->nResult = uv_trader_obj->m_pApi->ReqQryInstrument(_pQryInstrument, baton->iRequestID);
							logger_cout(log.append("ftdc_trader_api ReqQryInstrument run,the result:").append(to_string(baton->nResult)).c_str());

							break;
	}
	case T_TRADINGACCOUNT_RE:
	{
								CThostFtdcQryTradingAccountField *_pQryTradingAccount = static_cast<CThostFtdcQryTradingAccountField*>(baton->args);
								baton->nResult = uv_trader_obj->m_pApi->ReqQryTradingAccount(_pQryTradingAccount, baton->iRequestID);
								logger_cout(log.append("ftdc_trader_api ReqQryTradingAccount run,the result:").append(to_string(baton->nResult)).c_str());

								break;
	}
	case T_INVESTORPOSITION_RE:
	{
								  CThostFtdcQryInvestorPositionField *_pQryInvestorPosition = static_cast<CThostFtdcQryInvestorPositionField*>(baton->args);
								  baton->nResult = uv_trader_obj->m_pApi->ReqQryInvestorPosition(_pQryInvestorPosition, baton->iRequestID);
								  logger_cout(log.append("ftdc_trader_api ReqQryInvestorPosition run,the result:").append(to_string(baton->nResult)).c_str());

								  break;
	}
	case T_INVESTORPOSITIONDETAIL_RE:
	{
										CThostFtdcQryInvestorPositionDetailField *_pQryInvestorPositionDetail = static_cast<CThostFtdcQryInvestorPositionDetailField*>(baton->args);
										baton->nResult = uv_trader_obj->m_pApi->ReqQryInvestorPositionDetail(_pQryInvestorPositionDetail, baton->iRequestID);
										logger_cout(log.append("ftdc_trader_api ReqQryInvestorPositionDetail run,the result:").append(to_string(baton->nResult)).c_str());

										break;
	}
	case T_INSERT_RE:
	{
						CThostFtdcInputOrderField *_pInputOrder = static_cast<CThostFtdcInputOrderField*>(baton->args);
						baton->nResult = uv_trader_obj->m_pApi->ReqOrderInsert(_pInputOrder, baton->iRequestID);
						logger_cout(log.append("ftdc_trader_api ReqOrderInsert run,the result:").append(to_string(baton->nResult)).c_str());

						break;
	}
	case T_ACTION_RE:
	{
						CThostFtdcInputOrderActionField *_pInputOrderAction = static_cast<CThostFtdcInputOrderActionField*>(baton->args);
						baton->nResult = uv_trader_obj->m_pApi->ReqOrderAction(_pInputOrderAction, baton->iRequestID);
						logger_cout(log.append("ftdc_trader_api ReqOrderAction run,the result:").append(to_string(baton->nResult)).c_str());

						break;
	}
	case T_MARGINRATE_RE:
	{
							CThostFtdcQryInstrumentMarginRateField *_pQryInstrumentMarginRate = static_cast<CThostFtdcQryInstrumentMarginRateField*>(baton->args);
							baton->nResult = uv_trader_obj->m_pApi->ReqQryInstrumentMarginRate(_pQryInstrumentMarginRate, baton->iRequestID);
							logger_cout(log.append("ftdc_trader_api ReqQryInstrumentMarginRate run,the result:").append(to_string(baton->nResult)).c_str());

							break;
	}
	case T_DEPTHMARKETDATA_RE:
	{
								 CThostFtdcQryDepthMarketDataField *_pQryDepthMarketData = static_cast<CThostFtdcQryDepthMarketDataField*>(baton->args);
								 baton->nResult = uv_trader_obj->m_pApi->ReqQryDepthMarketData(_pQryDepthMarketData, baton->iRequestID);
								 logger_cout(log.append("ftdc_trader_api ReqQryDepthMarketData run,the result:").append(to_string(baton->nResult)).c_str());

								 break;
	}
	case T_SETTLEMENTINFO_RE:
	{
								CThostFtdcQrySettlementInfoField *_pQrySettlementInfo = static_cast<CThostFtdcQrySettlementInfoField*>(baton->args);
								baton->nResult = uv_trader_obj->m_pApi->ReqQrySettlementInfo(_pQrySettlementInfo, baton->iRequestID);
								logger_cout(log.append("ftdc_trader_api ReqQrySettlementInfo run,the result:").append(to_string(baton->nResult)).c_str());

								break;
	}
	}
}
///uv_queue_work 方法完成回调
void uv_trader::_completed(uv_work_t * work, int) {
	LookupCtpApiBaton* baton = static_cast<LookupCtpApiBaton*>(work->data);
	baton->callback(baton->nResult, baton);
	delete baton->args;
	delete baton;
}
///uv_async_init 服务器消息回调
void uv_trader::completeCb(uv_async_t* handle,int) {	
	CbRtnField* cbTrnField = (CbRtnField*)handle->data;
	cb_map[cbTrnField->eFlag]->callback(cbTrnField);	
	if (cbTrnField->rtnField)
		delete cbTrnField->rtnField;
	if (cbTrnField->rspInfo)
		delete cbTrnField->rspInfo;
	delete cbTrnField;
	handle->data = NULL;
}

void uv_trader::invoke(void* field, int ret, void(*callback)(int, void*), int uuid) {
	LookupCtpApiBaton* baton = new LookupCtpApiBaton();//完成函数中需要销毁
	baton->work.data = baton;
	baton->uv_trader_obj = this;
	baton->callback = callback;
	baton->args = field;
	baton->fun = ret;
	baton->uuid = uuid;
    __sync_fetch_and_add(&iRequestID,1);
    baton->iRequestID = iRequestID;
	std::string head = "uv_trader invoke function uuid:";
	logger_cout(head.append(to_string(uuid)).append(",requestid:").append(to_string(baton->iRequestID)).c_str());
	uv_queue_work(uv_default_loop(), &baton->work, _async, _completed);
}

void uv_trader::pkg_senduv(int event_type, void* _stru, CThostFtdcRspInfoField *pRspInfo_org, int nRequestID, bool bIsLast) {
	std::string log = "ftdc_trade_api callback,event type:";
	logger_cout(log.append(to_string(event_type)).append(",requestid:").append(to_string(nRequestID)).append(",islast:").append(to_string(bIsLast)).c_str());
	std::map<int, uv_async_t*>::iterator it = async_map.find(event_type);
	if (it != async_map.end()) {
		CThostFtdcRspInfoField* _pRspInfo = NULL;
		if (pRspInfo_org) {	  		
			_pRspInfo = new CThostFtdcRspInfoField();
			memcpy(_pRspInfo, pRspInfo_org, sizeof(CThostFtdcRspInfoField));
		}  
		CbRtnField* field = new CbRtnField();
		field->eFlag = event_type;
		field->rtnField = _stru;
		field->rspInfo = (void*)_pRspInfo;
		field->nRequestID = nRequestID;
		field->bIsLast = bIsLast;
		it->second->data = field;
		uv_async_send(it->second);
	}
}


