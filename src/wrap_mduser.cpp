#include <node.h>
#include "wrap_mduser.h"

Persistent<Function> WrapMdUser::constructor;
int WrapMdUser::s_uuid;
std::map<const char*, int,ptrCmp> WrapMdUser::event_map;
std::map<int, Persistent<Function> > WrapMdUser::callback_map;
std::map<int, Persistent<Function> > WrapMdUser::fun_rtncb_map;

WrapMdUser::WrapMdUser() {
	logger_cout("wrap_mduser------>object start init");
	uvMdUser = new uv_mduser();
	logger_cout("wrap_mduser------>object init successed");
}

WrapMdUser::~WrapMdUser() {
    if(uvMdUser){
	    delete uvMdUser;
    }
	logger_cout("wrape_mduser------>object destroyed");
}

void WrapMdUser::Init(int args) {
	// Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
	tpl->SetClassName(String::NewSymbol("WrapMdUser"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	// Prototype
	tpl->PrototypeTemplate()->Set(String::NewSymbol("on"),
		FunctionTemplate::New(On)->GetFunction());

	tpl->PrototypeTemplate()->Set(String::NewSymbol("connect"),
		FunctionTemplate::New(Connect)->GetFunction());

	tpl->PrototypeTemplate()->Set(String::NewSymbol("reqUserLogin"),
		FunctionTemplate::New(ReqUserLogin)->GetFunction());

	tpl->PrototypeTemplate()->Set(String::NewSymbol("reqUserLogout"),
		FunctionTemplate::New(ReqUserLogout)->GetFunction());

	tpl->PrototypeTemplate()->Set(String::NewSymbol("subscribeMarketData"),
		FunctionTemplate::New(SubscribeMarketData)->GetFunction());

	tpl->PrototypeTemplate()->Set(String::NewSymbol("unSubscribeMarketData"),
		FunctionTemplate::New(UnSubscribeMarketData)->GetFunction());	

	tpl->PrototypeTemplate()->Set(String::NewSymbol("disconnect"),
		FunctionTemplate::New(Disposed)->GetFunction());

	constructor = Persistent<Function>::New(tpl->GetFunction());
}

void WrapMdUser::initEventMap() {
	event_map["connect"] = T_ON_CONNECT;
	event_map["disconnected"] = T_ON_DISCONNECTED;
	event_map["rspUserLogin"] = T_ON_RSPUSERLOGIN;
	event_map["rspUserLogout"] = T_ON_RSPUSERLOGOUT;
	event_map["rspSubMarketData"] = T_ON_RSPSUBMARKETDATA;
	event_map["rspUnSubMarketData"] = T_ON_RSPUNSUBMARKETDATA;
	event_map["rtnDepthMarketData"] = T_ON_RTNDEPTHMARKETDATA;
	event_map["rspError"] = T_ON_RSPERROR;
}

Handle<Value> WrapMdUser::New(const Arguments& args) {
	HandleScope scope;

	if (event_map.size() == 0)
		initEventMap();
	WrapMdUser* obj = new WrapMdUser();
	obj->Wrap(args.This());
	return args.This();
}

Handle<Value> WrapMdUser::NewInstance(const Arguments& args) {
	HandleScope scope;

	const unsigned argc = 1;
	Handle<Value> argv[argc] = { args[0] };
	Local<Object> instance = constructor->NewInstance(argc, argv);
	return scope.Close(instance);
}

Handle<Value> WrapMdUser::On(const Arguments& args) {
	HandleScope scope;
	if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
		logger_cout("Wrong arguments->event name or function");
		ThrowException(Exception::TypeError(String::New("Wrong arguments->event name or function")));
		return scope.Close(Undefined());
	}

	WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());

	Local<String> eventName = args[0]->ToString();
	Local<Function> cb = Local<Function>::Cast(args[1]);
	Persistent<Function> unRecoveryCb = Persistent<Function>::New(cb);
	String::AsciiValue eNameAscii(eventName);

	std::map<const char*, int>::iterator eIt = event_map.find(*eNameAscii);
	if (eIt == event_map.end()) {
		ThrowException(Exception::TypeError(String::New("System has no register this event")));
		return scope.Close(Undefined());
	}
	std::map<int, Persistent<Function> >::iterator cIt = callback_map.find(eIt->second);
	if (cIt != callback_map.end()) {
		logger_cout("Callback is defined before");
		ThrowException(Exception::TypeError(String::New("Callback is defined before")));
		return scope.Close(Undefined());
	}

	callback_map[eIt->second] = unRecoveryCb;
	obj->uvMdUser->On(*eNameAscii,eIt->second, FunCallback);
	return scope.Close(Int32::New(0));
}

Handle<Value> WrapMdUser::Connect(const Arguments& args) {
	HandleScope scope;
	std::string log = "wrap_mduser Connect------>";
	if (args[0]->IsUndefined()) {
		logger_cout("Wrong arguments->front addr");
		ThrowException(Exception::TypeError(String::New("Wrong arguments->front addr")));
		return scope.Close(Undefined());
	}	
	int uuid = -1;
	WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
	if (!args[2]->IsUndefined() && args[2]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[2]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	Local<String> frontAddr = args[0]->ToString();
	Local<String> szPath = args[1]->IsUndefined() ? String::New("m") : args[0]->ToString();
	String::AsciiValue addrAscii(frontAddr);
	String::AsciiValue pathAscii(szPath);

	UVConnectField pConnectField;
	memset(&pConnectField, 0, sizeof(pConnectField));
	strcpy(pConnectField.front_addr, ((std::string)*addrAscii).c_str());
	strcpy(pConnectField.szPath, ((std::string)*pathAscii).c_str());  
	logger_cout(log.append(" ").append((std::string)*addrAscii).append("|").append((std::string)*pathAscii).append("|").c_str());
	obj->uvMdUser->Connect(&pConnectField, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapMdUser::ReqUserLogin(const Arguments& args) {
	HandleScope scope;
	std::string log = "wrap_mduser ReqUserLogin------>";
	if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}

	int uuid = -1;
	WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
	if (!args[3]->IsUndefined() && args[3]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[3]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	Local<String> broker = args[0]->ToString();
	Local<String> userId = args[1]->ToString();
	Local<String> pwd = args[2]->ToString();
	String::AsciiValue brokerAscii(broker);
	String::AsciiValue userIdAscii(userId);
	String::AsciiValue pwdAscii(pwd);

	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	strcpy(req.UserID, ((std::string)*userIdAscii).c_str());
	strcpy(req.Password, ((std::string)*pwdAscii).c_str());
	logger_cout(log.append(" ").append((std::string)*brokerAscii).append("|").append((std::string)*userIdAscii).append("|").append((std::string)*pwdAscii).c_str());
	obj->uvMdUser->ReqUserLogin(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapMdUser::ReqUserLogout(const Arguments& args) {
	HandleScope scope;
	std::string log = "wrap_mduser ReqUserLogout------>";

	if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;
	WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
	if (!args[2]->IsUndefined() && args[2]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[2]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	Local<String> broker = args[0]->ToString();
	Local<String> userId = args[1]->ToString();
	String::AsciiValue brokerAscii(broker);
	String::AsciiValue userIdAscii(userId);

	CThostFtdcUserLogoutField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	strcpy(req.UserID, ((std::string)*userIdAscii).c_str());
	logger_cout(log.append(" ").append((std::string)*brokerAscii).append("|").append((std::string)*userIdAscii).c_str());
	obj->uvMdUser->ReqUserLogout(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapMdUser::SubscribeMarketData(const Arguments& args) {
	HandleScope scope;
	std::string log = "wrap_mduser SubscribeMarketData------>";

	if (args[0]->IsUndefined() || !args[0]->IsArray()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;
	WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
	if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[1]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	} 
	Local<v8::Array> instrumentIDs = Local<v8::Array>::Cast(args[0]);
	char** idArray = new char*[instrumentIDs->Length()];
	
	for (uint32_t i = 0; i < instrumentIDs->Length(); i++) {
		Local<String> instrumentId = instrumentIDs->Get(i)->ToString();
		String::AsciiValue idAscii(instrumentId);  		 
		char* id = new char[instrumentId->Length() + 1];
		strcpy(id, *idAscii);
		idArray[i] = id;
		log.append(*idAscii).append("|");
	}
	logger_cout(log.c_str());
	obj->uvMdUser->SubscribeMarketData(idArray, instrumentIDs->Length(), FunRtnCallback, uuid);
	delete idArray;
	return scope.Close(Undefined());
}

Handle<Value> WrapMdUser::UnSubscribeMarketData(const Arguments& args) {
	HandleScope scope;
	std::string log = "wrap_mduser UnSubscribeMarketData------>";

	if (args[0]->IsUndefined() || !args[0]->IsArray()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;
	WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
	if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[1]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}
	Local<v8::Array> instrumentIDs = Local<v8::Array>::Cast(args[0]);
	char** idArray = new char*[instrumentIDs->Length()];

	for (uint32_t i = 0; i < instrumentIDs->Length(); i++) {
		Local<String> instrumentId = instrumentIDs->Get(i)->ToString();
		String::AsciiValue idAscii(instrumentId);
		char* id = new char[instrumentId->Length() + 1];
		strcpy(id, *idAscii);
		idArray[i] = id;
		log.append(*idAscii).append("|");
	}
	logger_cout(log.c_str());
	obj->uvMdUser->UnSubscribeMarketData(idArray, instrumentIDs->Length(), FunRtnCallback, uuid);
	return scope.Close(Undefined());	 
}

Handle<Value> WrapMdUser::Disposed(const Arguments& args) {
	HandleScope scope;
	WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
	obj->uvMdUser->Disposed();
	std::map<int, Persistent<Function> >::iterator callback_it = callback_map.begin();
	while (callback_it != callback_map.end()) {
		callback_it->second.Dispose();
		callback_it++;
	}
	event_map.clear();
	callback_map.clear();
	fun_rtncb_map.clear();
	delete obj->uvMdUser;
    obj->uvMdUser = NULL;
	logger_cout("wrap_mduser Disposed------>wrap disposed");
	return scope.Close(Undefined());
}


////////////////////////////////////////////////////////////////////////////////////////////////

void WrapMdUser::FunCallback(CbRtnField *data) {
	HandleScope scope;
	std::map<int, Persistent<Function> >::iterator cIt = callback_map.find(data->eFlag);
	if (cIt == callback_map.end())
		return;

	switch (data->eFlag) {
	case T_ON_CONNECT:
	{
						 Local<Value> argv[1] = { Local<Value>::New(Undefined()) };
						 cIt->second->Call(Context::GetCurrent()->Global(), 1, argv);
						 break;
	}
	case T_ON_DISCONNECTED:
	{
							  Local<Value> argv[1] = { Int32::New(data->nReason) };
							  cIt->second->Call(Context::GetCurrent()->Global(), 1, argv);
							  break;
	}
	case T_ON_RSPUSERLOGIN:
	{
							  Local<Value> argv[4];
							  pkg_cb_userlogin(data, argv);
							  cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
							  break;
	}
	case T_ON_RSPUSERLOGOUT:
	{
							   Local<Value> argv[4];
							   pkg_cb_userlogout(data, argv);
							   cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
							   break;
	}
	case T_ON_RSPSUBMARKETDATA:
	{
								  Local<Value> argv[4];
								  pkg_cb_rspsubmarketdata(data, argv);
								  cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
								  break;
	}
	case T_ON_RSPUNSUBMARKETDATA:
	{
									Local<Value> argv[4];
									pkg_cb_unrspsubmarketdata(data, argv);
									cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
									break;
	}
	case T_ON_RTNDEPTHMARKETDATA:
	{
									Local<Value> argv[1];
									pkg_cb_rtndepthmarketdata(data, argv);
									cIt->second->Call(Context::GetCurrent()->Global(), 1, argv);
									break;
	}
	case T_ON_RSPERROR:
	{
						  Local<Value> argv[3];
						  pkg_cb_rsperror(data, argv);
						  cIt->second->Call(Context::GetCurrent()->Global(), 3, argv);

						  break;
	}
	}
	scope.Close(Undefined());
}
void WrapMdUser::FunRtnCallback(int result, void* baton) {
	HandleScope scope;
	LookupCtpApiBaton* tmp = static_cast<LookupCtpApiBaton*>(baton);
	if (tmp->uuid != -1) {
		std::map<const int, Persistent<Function> >::iterator it = fun_rtncb_map.find(tmp->uuid);
		Local<Value> argv[1] = { Local<Value>::New(Int32::New(tmp->nResult)) };
		it->second->Call(Context::GetCurrent()->Global(), 1, argv);
		it->second.Dispose();
		fun_rtncb_map.erase(tmp->uuid);
	}
	scope.Close(Undefined());
}
void WrapMdUser::pkg_cb_userlogin(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
	CThostFtdcRspUserLoginField* pRspUserLogin = static_cast<CThostFtdcRspUserLoginField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pRspUserLogin) {
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("TradingDay"), String::New(pRspUserLogin->TradingDay));
		jsonRtn->Set(String::NewSymbol("LoginTime"), String::New(pRspUserLogin->LoginTime));
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pRspUserLogin->BrokerID));
		jsonRtn->Set(String::NewSymbol("UserID"), String::New(pRspUserLogin->UserID));
		jsonRtn->Set(String::NewSymbol("SystemName"), String::New(pRspUserLogin->SystemName));
		jsonRtn->Set(String::NewSymbol("FrontID"), Int32::New(pRspUserLogin->FrontID));
		jsonRtn->Set(String::NewSymbol("SessionID"), Int32::New(pRspUserLogin->SessionID));
		jsonRtn->Set(String::NewSymbol("MaxOrderRef"), String::New(pRspUserLogin->MaxOrderRef));
		jsonRtn->Set(String::NewSymbol("SHFETime"), String::New(pRspUserLogin->SHFETime));
		jsonRtn->Set(String::NewSymbol("DCETime"), String::New(pRspUserLogin->DCETime));
		jsonRtn->Set(String::NewSymbol("CZCETime"), String::New(pRspUserLogin->CZCETime));
		jsonRtn->Set(String::NewSymbol("FFEXTime"), String::New(pRspUserLogin->FFEXTime));
		jsonRtn->Set(String::NewSymbol("INETime"), String::New(pRspUserLogin->INETime));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}

	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapMdUser::pkg_cb_userlogout(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
	CThostFtdcRspUserLoginField* pRspUserLogin = static_cast<CThostFtdcRspUserLoginField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pRspUserLogin) {
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pRspUserLogin->BrokerID));
		jsonRtn->Set(String::NewSymbol("UserID"), String::New(pRspUserLogin->UserID));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapMdUser::pkg_cb_rspsubmarketdata(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
	CThostFtdcSpecificInstrumentField *pSpecificInstrument = static_cast<CThostFtdcSpecificInstrumentField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pSpecificInstrument) {
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pSpecificInstrument->InstrumentID));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapMdUser::pkg_cb_unrspsubmarketdata(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
	CThostFtdcSpecificInstrumentField *pSpecificInstrument = static_cast<CThostFtdcSpecificInstrumentField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pSpecificInstrument) {
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pSpecificInstrument->InstrumentID));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapMdUser::pkg_cb_rtndepthmarketdata(CbRtnField* data, Local<Value>*cbArray) {
	CThostFtdcDepthMarketDataField *pDepthMarketData = static_cast<CThostFtdcDepthMarketDataField*>(data->rtnField);
	if (pDepthMarketData) {	   		
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("TradingDay"), String::New(pDepthMarketData->TradingDay));
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pDepthMarketData->InstrumentID));
		jsonRtn->Set(String::NewSymbol("ExchangeID"), String::New(pDepthMarketData->ExchangeID));
		jsonRtn->Set(String::NewSymbol("ExchangeInstID"), String::New(pDepthMarketData->ExchangeInstID));
		jsonRtn->Set(String::NewSymbol("LastPrice"), Number::New(pDepthMarketData->LastPrice));
		jsonRtn->Set(String::NewSymbol("PreSettlementPrice"), Number::New(pDepthMarketData->PreSettlementPrice));
		jsonRtn->Set(String::NewSymbol("PreClosePrice"), Number::New(pDepthMarketData->PreClosePrice));
		jsonRtn->Set(String::NewSymbol("PreOpenInterest"), Number::New(pDepthMarketData->PreOpenInterest));
		jsonRtn->Set(String::NewSymbol("OpenPrice"), Number::New(pDepthMarketData->OpenPrice));
		jsonRtn->Set(String::NewSymbol("HighestPrice"), Number::New(pDepthMarketData->HighestPrice));
		jsonRtn->Set(String::NewSymbol("LowestPrice"), Number::New(pDepthMarketData->LowestPrice));
		jsonRtn->Set(String::NewSymbol("Volume"), Int32::New(pDepthMarketData->Volume));
		jsonRtn->Set(String::NewSymbol("Turnover"), Number::New(pDepthMarketData->Turnover));
		jsonRtn->Set(String::NewSymbol("OpenInterest"), Number::New(pDepthMarketData->OpenInterest));
		jsonRtn->Set(String::NewSymbol("ClosePrice"), Number::New(pDepthMarketData->ClosePrice));
		jsonRtn->Set(String::NewSymbol("SettlementPrice"), Number::New(pDepthMarketData->SettlementPrice));
		jsonRtn->Set(String::NewSymbol("UpperLimitPrice"), Number::New(pDepthMarketData->UpperLimitPrice));
		jsonRtn->Set(String::NewSymbol("LowerLimitPrice"), Number::New(pDepthMarketData->LowerLimitPrice));
		jsonRtn->Set(String::NewSymbol("PreDelta"), Number::New(pDepthMarketData->PreDelta));
		jsonRtn->Set(String::NewSymbol("CurrDelta"), Number::New(pDepthMarketData->CurrDelta));
		jsonRtn->Set(String::NewSymbol("UpdateTime"), String::New(pDepthMarketData->UpdateTime));
		jsonRtn->Set(String::NewSymbol("UpdateMillisec"), Int32::New(pDepthMarketData->UpdateMillisec));
		jsonRtn->Set(String::NewSymbol("BidPrice1"), Number::New(pDepthMarketData->BidPrice1));
		jsonRtn->Set(String::NewSymbol("BidVolume1"), Number::New(pDepthMarketData->BidVolume1));
		jsonRtn->Set(String::NewSymbol("AskPrice1"), Number::New(pDepthMarketData->AskPrice1));
		jsonRtn->Set(String::NewSymbol("AskVolume1"), Number::New(pDepthMarketData->AskVolume1));
		jsonRtn->Set(String::NewSymbol("BidPrice2"), Number::New(pDepthMarketData->BidPrice2));
		jsonRtn->Set(String::NewSymbol("BidVolume2"), Number::New(pDepthMarketData->BidVolume2));
		jsonRtn->Set(String::NewSymbol("AskPrice2"), Number::New(pDepthMarketData->AskPrice2));
		jsonRtn->Set(String::NewSymbol("AskVolume2"), Number::New(pDepthMarketData->AskVolume2));
		jsonRtn->Set(String::NewSymbol("BidPrice3"), Number::New(pDepthMarketData->BidPrice3));
		jsonRtn->Set(String::NewSymbol("BidVolume3"), Number::New(pDepthMarketData->BidVolume3));
		jsonRtn->Set(String::NewSymbol("AskPrice3"), Number::New(pDepthMarketData->AskPrice3));
		jsonRtn->Set(String::NewSymbol("AskVolume3"), Number::New(pDepthMarketData->AskVolume3));
		jsonRtn->Set(String::NewSymbol("BidPrice4"), Number::New(pDepthMarketData->BidPrice4));
		jsonRtn->Set(String::NewSymbol("BidVolume4"), Number::New(pDepthMarketData->BidVolume4));
		jsonRtn->Set(String::NewSymbol("AskPrice4"), Number::New(pDepthMarketData->AskPrice4));
		jsonRtn->Set(String::NewSymbol("AskVolume4"), Number::New(pDepthMarketData->AskVolume4));
		jsonRtn->Set(String::NewSymbol("BidPrice5"), Number::New(pDepthMarketData->BidPrice5));
		jsonRtn->Set(String::NewSymbol("BidVolume5"), Number::New(pDepthMarketData->BidVolume5));
		jsonRtn->Set(String::NewSymbol("AskPrice5"), Number::New(pDepthMarketData->AskPrice5));
		jsonRtn->Set(String::NewSymbol("AskVolume5"), Number::New(pDepthMarketData->AskVolume5));
		jsonRtn->Set(String::NewSymbol("AveragePrice"), Number::New(pDepthMarketData->AveragePrice));
		jsonRtn->Set(String::NewSymbol("ActionDay"), String::New(pDepthMarketData->ActionDay));	   	
		*cbArray = jsonRtn;
	}
	else {
		*cbArray = Local<Value>::New(Undefined());
	}
	
	return;
}  
void WrapMdUser::pkg_cb_rsperror(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	*(cbArray + 2) = pkg_rspinfo(pRspInfo);
	return;
}
Local<Value> WrapMdUser::pkg_rspinfo(CThostFtdcRspInfoField *pRspInfo) {
	if (pRspInfo) {
		Local<Object> jsonInfo = Object::New();
		jsonInfo->Set(String::NewSymbol("ErrorID"), Int32::New(pRspInfo->ErrorID));
		jsonInfo->Set(String::NewSymbol("ErrorMsg"), String::New(pRspInfo->ErrorMsg));
		return jsonInfo;
	}
	else {
		return 	Local<Value>::New(Undefined());
	}
}
