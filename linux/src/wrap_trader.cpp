#include <node.h>
#include "wrap_trader.h"

Persistent<Function> WrapTrader::constructor;
int WrapTrader::s_uuid;
std::map<const char*, int,ptrCmp> WrapTrader::event_map;
std::map<int, Persistent<Function> > WrapTrader::callback_map;
std::map<int, Persistent<Function> > WrapTrader::fun_rtncb_map;

WrapTrader::WrapTrader() {	
	logger_cout("object start init");
	uvTrader = new uv_trader();	
	logger_cout("object init successed");
}

WrapTrader::~WrapTrader(void) {
	delete uvTrader;
	logger_cout("wraper object destroyed");
}

void WrapTrader::Init(int args) {
	// Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
	tpl->SetClassName(String::NewSymbol("WrapTrader"));
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

	tpl->PrototypeTemplate()->Set(String::NewSymbol("reqSettlementInfoConfirm"),
		FunctionTemplate::New(ReqSettlementInfoConfirm)->GetFunction());

	tpl->PrototypeTemplate()->Set(String::NewSymbol("reqQryInstrument"),
		FunctionTemplate::New(ReqQryInstrument)->GetFunction());

	tpl->PrototypeTemplate()->Set(String::NewSymbol("reqQryTradingAccount"),
		FunctionTemplate::New(ReqQryTradingAccount)->GetFunction());

	tpl->PrototypeTemplate()->Set(String::NewSymbol("reqQryInvestorPosition"),
		FunctionTemplate::New(ReqQryInvestorPosition)->GetFunction());

	tpl->PrototypeTemplate()->Set(String::NewSymbol("reqQryInvestorPositionDetail"),
		FunctionTemplate::New(ReqQryInvestorPositionDetail)->GetFunction());

	tpl->PrototypeTemplate()->Set(String::NewSymbol("reqOrderInsert"),
		FunctionTemplate::New(ReqOrderInsert)->GetFunction());

	tpl->PrototypeTemplate()->Set(String::NewSymbol("reqOrderAction"),
		FunctionTemplate::New(ReqOrderAction)->GetFunction());

	tpl->PrototypeTemplate()->Set(String::NewSymbol("reqQryInstrumentMarginRate"),
		FunctionTemplate::New(ReqQryInstrumentMarginRate)->GetFunction());

	tpl->PrototypeTemplate()->Set(String::NewSymbol("reqQryDepthMarketData"),
		FunctionTemplate::New(ReqQryDepthMarketData)->GetFunction());

	tpl->PrototypeTemplate()->Set(String::NewSymbol("ReqQrySettlementInfo"),
		FunctionTemplate::New(ReqQrySettlementInfo)->GetFunction());

	tpl->PrototypeTemplate()->Set(String::NewSymbol("disconnect"),
		FunctionTemplate::New(Disposed)->GetFunction());

	constructor = Persistent<Function>::New(tpl->GetFunction());
}

void WrapTrader::initEventMap() {
	event_map["connect"] = T_ON_CONNECT;
	event_map["disconnected"] = T_ON_DISCONNECTED;
	event_map["rspUserLogin"] = T_ON_RSPUSERLOGIN;
	event_map["rspUserLogout"] = T_ON_RSPUSERLOGOUT;
	event_map["rspInfoconfirm"] = T_ON_RSPINFOCONFIRM;
	event_map["rspInsert"] = T_ON_RSPINSERT;
	event_map["errInsert"] = T_ON_ERRINSERT;
	event_map["rspAction"] = T_ON_RSPACTION;
	event_map["errAction"] = T_ON_ERRACTION;
	event_map["rqOrder"] = T_ON_RQORDER;
	event_map["rtnOrder"] = T_ON_RTNORDER;
	event_map["rqTrade"] = T_ON_RQTRADE;
	event_map["rtnTrade"] = T_ON_RTNTRADE;
	event_map["rqInvestorPosition"] = T_ON_RQINVESTORPOSITION;
	event_map["rqInvestorPositionDetail"] = T_ON_RQINVESTORPOSITIONDETAIL;
	event_map["rqTradingAccount"] = T_ON_RQTRADINGACCOUNT;
	event_map["rqInstrument"] = T_ON_RQINSTRUMENT;
	event_map["rqDdpthmarketData"] = T_ON_RQDEPTHMARKETDATA;
	event_map["rqSettlementInfo"] = T_ON_RQSETTLEMENTINFO;
	event_map["rspError"] = T_ON_RSPERROR;
}

Handle<Value> WrapTrader::New(const Arguments& args) {
	HandleScope scope;

	if (event_map.size() == 0)
		initEventMap();
	WrapTrader* obj = new WrapTrader();
	obj->Wrap(args.This());
	return args.This();
}

Handle<Value> WrapTrader::NewInstance(const Arguments& args) {
	HandleScope scope;

	const unsigned argc = 1;
	Handle<Value> argv[argc] = { args[0] };
	Local<Object> instance = constructor->NewInstance(argc, argv);
	return scope.Close(instance);
}

Handle<Value> WrapTrader::On(const Arguments& args) {
	HandleScope scope;
	if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
		logger_cout("Wrong arguments->event name or function");
		ThrowException(Exception::TypeError(String::New("Wrong arguments->event name or function")));
		return scope.Close(Undefined());
	}
	
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	
	Local<String> eventName = args[0]->ToString();
	Local<Function> cb = Local<Function>::Cast(args[1]);
	Persistent<Function> unRecoveryCb = Persistent<Function>::New(cb);
	String::AsciiValue eNameAscii(eventName);

	std::map<const char*, int>::iterator eIt = event_map.find(*eNameAscii);
	if (eIt == event_map.end()) {
		logger_cout("System has not register this event");
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
	obj->uvTrader->On(eIt->second, FunCallback);
	logger_cout(((std::string)*eNameAscii).append(" event register successed").c_str());
	return scope.Close(Int32::New(0));
}

Handle<Value> WrapTrader::Connect(const Arguments& args) {
	HandleScope scope;	
	std::string log = "function connect";
	if (args[0]->IsUndefined()) {
		logger_cout("Wrong arguments->front addr");
		ThrowException(Exception::TypeError(String::New("Wrong arguments->front addr")));
		return scope.Close(Undefined());
	}
	if (!args[2]->IsNumber() || !args[3]->IsNumber()) {
		logger_cout("Wrong arguments->public or private topic type");
		ThrowException(Exception::TypeError(String::New("Wrong arguments->public or private topic type")));
		return scope.Close(Undefined());
	}  
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[4]->IsUndefined() && args[4]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[4]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	Local<String> frontAddr = args[0]->ToString();
	Local<String> szPath = args[1]->IsUndefined() ? String::New("t") : args[0]->ToString();
	String::AsciiValue addrAscii(frontAddr);
	String::AsciiValue pathAscii(szPath);
	int publicTopicType = args[2]->Int32Value();
	int privateTopicType = args[3]->Int32Value();	 
	
	UVConnectField pConnectField; 
	memset(&pConnectField, 0, sizeof(pConnectField));		
	strcpy(pConnectField.front_addr, ((std::string)*addrAscii).c_str());
	strcpy(pConnectField.szPath, ((std::string)*pathAscii).c_str());
	pConnectField.public_topic_type = publicTopicType;
	pConnectField.private_topic_type = privateTopicType;	
	logger_cout(log.append(" ").append((std::string)*addrAscii).append("|").append((std::string)*pathAscii).append("|").append(to_string(publicTopicType)).append("|").append(to_string(privateTopicType)).c_str());
	obj->uvTrader->Connect(&pConnectField, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::ReqUserLogin(const Arguments& args) {
	HandleScope scope;
	std::string log = "function reqUserLogin";
	if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}

	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
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
	obj->uvTrader->ReqUserLogin(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::ReqUserLogout(const Arguments& args) {
	HandleScope scope;
	std::string log = "function reqUserLogout";

	if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
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
	obj->uvTrader->ReqUserLogout(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::ReqSettlementInfoConfirm(const Arguments& args) {
	HandleScope scope;
	std::string log = "function reqSettlementInfoConfirm";

	if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;	
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[2]->IsUndefined() && args[2]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[2]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}	 

	Local<String> broker = args[0]->ToString();
	Local<String> investorId = args[1]->ToString();
	String::AsciiValue brokerAscii(broker);
	String::AsciiValue investorIdAscii(investorId);

	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());
	logger_cout(log.append(" ").append((std::string)*brokerAscii).append("|").append((std::string)*investorIdAscii).c_str());
	obj->uvTrader->ReqSettlementInfoConfirm(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::ReqQryInstrument(const Arguments& args) {
	HandleScope scope;
	std::string log = "function reqQryInstrument";

	if (args[0]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[1]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	Local<String> instrumentId = args[0]->ToString();
	String::AsciiValue instrumentIdAscii(instrumentId);

	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.InstrumentID, ((std::string)*instrumentIdAscii).c_str());
	logger_cout(log.append(" ").append((std::string)*instrumentIdAscii).c_str());
	obj->uvTrader->ReqQryInstrument(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::ReqQryTradingAccount(const Arguments& args) {
	HandleScope scope;
	std::string log = "function reqQryTradingAccount";

	if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[2]->IsUndefined() && args[2]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[2]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}
	Local<String> broker = args[0]->ToString();
	Local<String> investorId = args[1]->ToString();
	String::AsciiValue brokerAscii(broker);
	String::AsciiValue investorIdAscii(investorId);

	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());
	logger_cout(log.append(" ").append((std::string)*brokerAscii).append("|").append((std::string)*investorIdAscii).c_str());
	obj->uvTrader->ReqQryTradingAccount(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::ReqQryInvestorPosition(const Arguments& args) {
	HandleScope scope;
	std::string log = "function reqQryInvestorPosition";

	if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[3]->IsUndefined() && args[3]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[3]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}
	Local<String> broker = args[0]->ToString();
	Local<String> investorId = args[1]->ToString();
	Local<String> instrumentId = args[2]->ToString();
	String::AsciiValue brokerAscii(broker);
	String::AsciiValue investorIdAscii(investorId);
	String::AsciiValue instrumentIdAscii(instrumentId);

	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());
	strcpy(req.InstrumentID, ((std::string)*instrumentIdAscii).c_str());

	logger_cout(log.append(" ").append((std::string)*brokerAscii).append("|").append((std::string)*investorIdAscii).append("|").append((std::string)*instrumentIdAscii).c_str());
	obj->uvTrader->ReqQryInvestorPosition(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::ReqQryInvestorPositionDetail(const Arguments& args) {
	HandleScope scope;

	//WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());

	const unsigned val = 0;
	return scope.Close(Number::New(val));
}

Handle<Value> WrapTrader::ReqOrderInsert(const Arguments& args) {
	HandleScope scope;
	std::string log = "function reqOrderInsert";

	if (args[0]->IsUndefined() || !args[0]->IsObject()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[1]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}
	Local<Object> jsonObj = args[0]->ToObject();
	Local<Value> brokerId = jsonObj->Get(v8::String::New("brokerId"));
	if (brokerId->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->brokerId")));
		return scope.Close(Undefined());
	}
	String::AsciiValue brokerId_(brokerId->ToString());
	Local<Value> investorId = jsonObj->Get(v8::String::New("investorId"));
	if (investorId->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->investorId")));
		return scope.Close(Undefined());
	}
	String::AsciiValue investorId_(investorId->ToString());
	Local<Value> instrunmentId = jsonObj->Get(v8::String::New("instrunmentId"));
	if (instrunmentId->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->instrunmentId")));
		return scope.Close(Undefined());
	}
	String::AsciiValue instrunmentId_(instrunmentId->ToString());
	Local<Value> orderRef = jsonObj->Get(v8::String::New("orderRef"));
	if (orderRef->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->orderRef")));
		return scope.Close(Undefined());
	}
	String::AsciiValue orderRef_(orderRef->ToString());
	Local<Value> priceType = jsonObj->Get(v8::String::New("priceType"));
	if (priceType->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->priceType")));
		return scope.Close(Undefined());
	}
	String::AsciiValue priceType_(priceType->ToString());
	Local<Value> direction = jsonObj->Get(v8::String::New("direction"));
	if (direction->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->direction")));
		return scope.Close(Undefined());
	}
	String::AsciiValue direction_(direction->ToString());
	Local<Value> combOffsetFlag = jsonObj->Get(v8::String::New("combOffsetFlag"));
	if (combOffsetFlag->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->combOffsetFlag")));
		return scope.Close(Undefined());
	}
	String::AsciiValue combOffsetFlag_(combOffsetFlag->ToString());
	Local<Value> combHedgeFlag = jsonObj->Get(v8::String::New("combHedgeFlag"));
	if (combHedgeFlag->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->combHedgeFlag")));
		return scope.Close(Undefined());
	}
	String::AsciiValue combHedgeFlag_(combHedgeFlag->ToString());
	Local<Value> vlimitPrice = jsonObj->Get(v8::String::New("limitPrice"));
	if (vlimitPrice->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->limitPrice")));
		return scope.Close(Undefined());
	}
	double limitPrice = vlimitPrice->NumberValue();
	Local<Value> vvolumeTotalOriginal = jsonObj->Get(v8::String::New("volumeTotalOriginal"));
	if (vvolumeTotalOriginal->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->volumeTotalOriginal")));
		return scope.Close(Undefined());
	}
	int32_t volumeTotalOriginal = vvolumeTotalOriginal->Int32Value();
	Local<Value> timeCondition = jsonObj->Get(v8::String::New("timeCondition"));
	if (timeCondition->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->timeCondition")));
		return scope.Close(Undefined());
	}
	String::AsciiValue timeCondition_(timeCondition->ToString());
	Local<Value> volumeCondition = jsonObj->Get(v8::String::New("volumeCondition"));
	if (volumeCondition->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->volumeCondition")));
		return scope.Close(Undefined());
	}
	String::AsciiValue volumeCondition_(volumeCondition->ToString());
	Local<Value> vminVolume = jsonObj->Get(v8::String::New("minVolume"));
	if (vminVolume->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->minVolume")));
		return scope.Close(Undefined());
	}
	int32_t minVolume = vminVolume->Int32Value();
	Local<Value> contingentCondition = jsonObj->Get(v8::String::New("contingentCondition"));
	if (contingentCondition->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->contingentCondition")));
		return scope.Close(Undefined());
	}
	String::AsciiValue contingentCondition_(contingentCondition->ToString());
	Local<Value> forceCloseReason = jsonObj->Get(v8::String::New("forceCloseReason"));
	if (forceCloseReason->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->forceCloseReason")));
		return scope.Close(Undefined());
	}
	String::AsciiValue forceCloseReason_(forceCloseReason->ToString());
	Local<Value> visAutoSuspend = jsonObj->Get(v8::String::New("isAutoSuspend"));
	if (visAutoSuspend->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->isAutoSuspend")));
		return scope.Close(Undefined());
	}
	int32_t isAutoSuspend = visAutoSuspend->Int32Value();
	Local<Value> vuserForceClose = jsonObj->Get(v8::String::New("userForceClose"));
	if (vuserForceClose->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->userForceClose")));
		return scope.Close(Undefined());
	}
	int32_t userForceClose = vuserForceClose->Int32Value();

	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));

	///经纪公司代码
	strcpy(req.BrokerID, ((std::string)*brokerId_).c_str());
	///投资者代码
	strcpy(req.InvestorID, ((std::string)*investorId_).c_str());
	///合约代码
	strcpy(req.InstrumentID, ((std::string)*instrunmentId_).c_str());
	///报单引用
	strcpy(req.OrderRef, ((std::string)*orderRef_).c_str());
	///报单价格条件
	req.OrderPriceType = ((std::string)*priceType_)[0];
	///买卖方向: 
	req.Direction = ((std::string)*direction_)[0];
	///组合开平标志
	req.CombOffsetFlag[0] = ((std::string)*combOffsetFlag_)[0];
	///组合投机套保标志
	req.CombHedgeFlag[0] = ((std::string)*combHedgeFlag_)[0];
	///价格
	req.LimitPrice = limitPrice;
	///数量: 
	req.VolumeTotalOriginal = volumeTotalOriginal;
	///有效期类型
	req.TimeCondition = ((std::string)*timeCondition_)[0];
	///成交量类型
	req.VolumeCondition = ((std::string)*volumeCondition_)[0];
	///最小成交量: 1
	req.MinVolume = minVolume;
	///触发条件
	req.ContingentCondition = ((std::string)*contingentCondition_)[0];
	///强平原因
	req.ForceCloseReason = ((std::string)*forceCloseReason_)[0];
	///自动挂起标志
	req.IsAutoSuspend = isAutoSuspend;
	///用户强评标志
	req.UserForceClose = userForceClose;
	logger_cout(log.append(" ").
		append((std::string)*brokerId_).append("|").
		append((std::string)*investorId_).append("|").
		append((std::string)*instrunmentId_).append("|").
		append((std::string)*orderRef_).append("|").
		append((std::string)*priceType_).append("|").
		append((std::string)*direction_).append("|").
		append((std::string)*combOffsetFlag_).append("|").
		append((std::string)*combHedgeFlag_).append("|").
		append(to_string(limitPrice)).append("|").
		append(to_string(volumeTotalOriginal)).append("|").
		append((std::string)*timeCondition_).append("|").
		append((std::string)*volumeCondition_).append("|").
		append(to_string(minVolume)).append("|").
		append((std::string)*contingentCondition_).append("|").
		append((std::string)*forceCloseReason_).append("|").
		append(to_string(isAutoSuspend)).append("|").
		append(to_string(userForceClose)).append("|").c_str());
	obj->uvTrader->ReqOrderInsert(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::ReqOrderAction(const Arguments& args) {
	HandleScope scope;
	std::string log = "function reqOrderInsert";

	if (args[0]->IsUndefined() || !args[0]->IsObject()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());

	if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[1]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	Local<Object> jsonObj = args[0]->ToObject();
	Local<Value> vbrokerId = jsonObj->Get(v8::String::New("brokerId"));
	if (vbrokerId->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->brokerId")));
		return scope.Close(Undefined());
	}
	String::AsciiValue brokerId_(vbrokerId->ToString());
	Local<Value> vinvestorId = jsonObj->Get(v8::String::New("investorId"));
	if (vinvestorId->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->investorId")));
		return scope.Close(Undefined());
	}
	String::AsciiValue investorId_(vinvestorId->ToString());
	Local<Value> vinstrunmentId = jsonObj->Get(v8::String::New("instrunmentId"));
	if (vinstrunmentId->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->instrunmentId")));
		return scope.Close(Undefined());
	}
	String::AsciiValue instrunmentId_(vinstrunmentId->ToString());
	Local<Value> vorderRef = jsonObj->Get(v8::String::New("orderRef"));
	if (vorderRef->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->orderRef")));
		return scope.Close(Undefined());
	}
	String::AsciiValue orderRef_(vorderRef->ToString());
	Local<Value> vfrontId = jsonObj->Get(v8::String::New("frontId"));
	if (vfrontId->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->frontId")));
		return scope.Close(Undefined());
	}
	int32_t frontId = vfrontId->Int32Value();
	Local<Value> vsessionId = jsonObj->Get(v8::String::New("sessionId"));
	if (vsessionId->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->sessionId")));
		return scope.Close(Undefined());
	}
	int32_t sessionId = vsessionId->Int32Value();
	Local<Value> vexchangeID = jsonObj->Get(v8::String::New("exchangeID"));
	if (vexchangeID->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->exchangeID")));
		return scope.Close(Undefined());
	}
	String::AsciiValue exchangeID_(vexchangeID->ToString());
	Local<Value> vorderSysID = jsonObj->Get(v8::String::New("orderSysID"));
	if (vorderSysID->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->orderSysID")));
		return scope.Close(Undefined());
	}
	String::AsciiValue orderSysID_(vorderSysID->ToString());
	Local<Value> vactionFlag = jsonObj->Get(v8::String::New("actionFlag"));
	if (vactionFlag->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->actionFlag")));
		return scope.Close(Undefined());
	}
	int32_t actionFlag = vactionFlag->Int32Value();

	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));
	///经纪公司代码
	strcpy(req.BrokerID, ((std::string)*brokerId_).c_str());
	///投资者代码
	strcpy(req.InvestorID, ((std::string)*investorId_).c_str());
	///报单引用
	strcpy(req.OrderRef, ((std::string)*orderRef_).c_str());
	///前置编号
	req.FrontID = frontId;
	///会话编号
	req.SessionID = sessionId;
	///交易所代码
	strcpy(req.ExchangeID, ((std::string)*exchangeID_).c_str());
	///报单编号
	strcpy(req.OrderSysID, ((std::string)*orderSysID_).c_str());
	///操作标志
	req.ActionFlag = actionFlag;
	///合约代码
	strcpy(req.InstrumentID, ((std::string)*instrunmentId_).c_str());
	logger_cout(log.append(" ").
		append((std::string)*brokerId_).append("|").
		append((std::string)*investorId_).append("|").
		append((std::string)*instrunmentId_).append("|").
		append((std::string)*orderRef_).append("|").
		append(to_string(frontId)).append("|").
		append(to_string(sessionId)).append("|").
		append((std::string)*exchangeID_).append("|").
		append((std::string)*orderSysID_).append("|").
		append(to_string(actionFlag)).append("|").c_str());

	obj->uvTrader->ReqOrderAction(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::ReqQryInstrumentMarginRate(const Arguments& args) {
	HandleScope scope;
	std::string log = "function reqQryInstrumentMarginRate";

	if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined() || args[3]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());

	if (!args[4]->IsUndefined() && args[4]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[4]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	Local<String> broker = args[0]->ToString();
	Local<String> investorId = args[1]->ToString();
	Local<String> instrumentId = args[2]->ToString();
	int32_t hedgeFlag = args[3]->Int32Value();
	String::AsciiValue brokerAscii(broker);
	String::AsciiValue investorIdAscii(investorId);
	String::AsciiValue instrumentIdAscii(instrumentId);

	CThostFtdcQryInstrumentMarginRateField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());
	strcpy(req.InstrumentID, ((std::string)*instrumentIdAscii).c_str());
	req.HedgeFlag = hedgeFlag;
	logger_cout(log.append(" ").
		append((std::string)*brokerAscii).append("|").
		append((std::string)*investorIdAscii).append("|").
		append((std::string)*instrumentIdAscii).append("|").
		append(to_string(hedgeFlag)).append("|").c_str());	 

	obj->uvTrader->ReqQryInstrumentMarginRate(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::ReqQryDepthMarketData(const Arguments& args) {
	HandleScope scope;
	std::string log = "function reqQryInstrumentMarginRate";

	if (args[0]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[1]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	Local<String> instrumentId = args[0]->ToString();
	String::AsciiValue instrumentIdAscii(instrumentId);

	CThostFtdcQryDepthMarketDataField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.InstrumentID, ((std::string)*instrumentIdAscii).c_str());
	logger_cout(log.append(" ").
		append((std::string)*instrumentIdAscii).append("|").c_str());
	obj->uvTrader->ReqQryDepthMarketData(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::ReqQrySettlementInfo(const Arguments& args) {
	HandleScope scope;
	std::string log = "function reqQryInstrumentMarginRate";

	if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[3]->IsUndefined() && args[3]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[3]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	Local<String> broker = args[0]->ToString();
	Local<String> investorId = args[1]->ToString();
	Local<String> tradingDay = args[2]->ToString();
	String::AsciiValue brokerAscii(broker);
	String::AsciiValue investorIdAscii(investorId);
	String::AsciiValue tradingDayAscii(tradingDay);

	CThostFtdcQrySettlementInfoField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());
	strcpy(req.TradingDay, ((std::string)*tradingDayAscii).c_str());
	logger_cout(log.append(" ").
		append((std::string)*brokerAscii).append("|").
		append((std::string)*investorIdAscii).append("|").
		append((std::string)*tradingDayAscii).append("|").c_str());

	obj->uvTrader->ReqQrySettlementInfo(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::Disposed(const Arguments& args) {
	HandleScope scope;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	obj->uvTrader->Disconnect();	
	std::map<int, Persistent<Function> >::iterator callback_it = callback_map.begin();
	while (callback_it != callback_map.end()) {
		callback_it->second.Dispose();
		callback_it++;
	}
	event_map.clear();
	callback_map.clear();
	fun_rtncb_map.clear();
	delete obj->uvTrader;
	logger_cout("wrap disposed");
	return scope.Close(Undefined());
}

void WrapTrader::FunCallback(CbRtnField *data) {
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
	case T_ON_RSPINFOCONFIRM:
	{
								Local<Value> argv[4];
								pkg_cb_confirm(data, argv);
								cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
								break;
	}
	case T_ON_RSPINSERT:
	{
						   Local<Value> argv[4];
						   pkg_cb_orderinsert(data, argv);
						   cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
						   break;
	}
	case T_ON_ERRINSERT:
	{
						   Local<Value> argv[2];
						   pkg_cb_errorderinsert(data, argv);
						   cIt->second->Call(Context::GetCurrent()->Global(), 2, argv);
						   break;
	}
	case T_ON_RSPACTION:
	{
						   Local<Value> argv[4];
						   pkg_cb_orderaction(data, argv);
						   cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
						   break;
	}
	case T_ON_ERRACTION:
	{
						   Local<Value> argv[2];
						   pkg_cb_errorderaction(data, argv);
						   cIt->second->Call(Context::GetCurrent()->Global(), 2, argv);

						   break;
	}
	case T_ON_RQORDER:
	{
						 Local<Value> argv[4];
						 pkg_cb_rspqryorder(data, argv);
						 cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
						 break;
	}
	case T_ON_RTNORDER:
	{
						  Local<Value> argv[1];
						  pkg_cb_rtnorder(data, argv);
						  cIt->second->Call(Context::GetCurrent()->Global(), 1, argv);

						  break;
	}
	case T_ON_RQTRADE:
	{
						 Local<Value> argv[4];
						 pkg_cb_rqtrade(data, argv);
						 cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

						 break;
	}
	case T_ON_RTNTRADE:
	{
						  Local<Value> argv[1];
						  pkg_cb_rtntrade(data, argv);
						  cIt->second->Call(Context::GetCurrent()->Global(), 1, argv);

						  break;
	}
	case T_ON_RQINVESTORPOSITION:
	{
									Local<Value> argv[4];
									pkg_cb_rqinvestorposition(data, argv);
									cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

									break;
	}
	case T_ON_RQINVESTORPOSITIONDETAIL:
	{
										  Local<Value> argv[4];
										  pkg_cb_rqinvestorpositiondetail(data, argv);
										  cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

										  break;
	}
	case T_ON_RQTRADINGACCOUNT:
	{
								  Local<Value> argv[4];
								  pkg_cb_rqtradingaccount(data, argv);
								  cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

								  break;
	}
	case T_ON_RQINSTRUMENT:
	{
							  Local<Value> argv[4];
							  pkg_cb_rqinstrument(data, argv);
							  cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

							  break;
	}
	case T_ON_RQDEPTHMARKETDATA:
	{
								   Local<Value> argv[4];
								   pkg_cb_rqdepthmarketdata(data, argv);
								   cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

								   break;
	}
	case T_ON_RQSETTLEMENTINFO:
	{
								  Local<Value> argv[4];
								  pkg_cb_rqsettlementinfo(data, argv);
								  cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
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

void WrapTrader::FunRtnCallback(int result, void* baton) {
	HandleScope scope;
	LookupCtpApiBaton* tmp = static_cast<LookupCtpApiBaton*>(baton);	 
	if (tmp->uuid != -1) {
		logger_cout(to_string(tmp->uuid).append(":uuid,").append("function return callback").c_str());
		std::map<int, Persistent<Function> >::iterator it = fun_rtncb_map.find(tmp->uuid);
		Local<Value> argv[1] = { Local<Value>::New(Int32::New(tmp->nResult)) };
		it->second->Call(Context::GetCurrent()->Global(), 1, argv);
		it->second.Dispose();
		fun_rtncb_map.erase(tmp->uuid);	  		
	}
	scope.Close(Undefined());
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void WrapTrader::pkg_cb_userlogin(CbRtnField* data, Local<Value>*cbArray) {
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
		*(cbArray + 2) = Undefined()->ToObject();
	}
	
	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapTrader::pkg_cb_userlogout(CbRtnField* data, Local<Value>*cbArray) {
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
		*(cbArray + 2) = Undefined()->ToObject();
	}
	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapTrader::pkg_cb_confirm(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
	CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm = static_cast<CThostFtdcSettlementInfoConfirmField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pSettlementInfoConfirm) {
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pSettlementInfoConfirm->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(pSettlementInfoConfirm->InvestorID));
		jsonRtn->Set(String::NewSymbol("ConfirmDate"), String::New(pSettlementInfoConfirm->ConfirmDate));
		jsonRtn->Set(String::NewSymbol("ConfirmTime"), String::New(pSettlementInfoConfirm->ConfirmTime));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined()->ToObject();
	}
	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapTrader::pkg_cb_orderinsert(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
	CThostFtdcInputOrderField* pInputOrder = static_cast<CThostFtdcInputOrderField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pInputOrder) {
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pInputOrder->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(pInputOrder->InvestorID));
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pInputOrder->InstrumentID));
		jsonRtn->Set(String::NewSymbol("OrderRef"), String::New(pInputOrder->OrderRef));
		jsonRtn->Set(String::NewSymbol("UserID"), String::New(pInputOrder->UserID));
		jsonRtn->Set(String::NewSymbol("OrderPriceType"), Int32::New(pInputOrder->OrderPriceType));
		jsonRtn->Set(String::NewSymbol("Direction"), Int32::New(pInputOrder->Direction));  //var charval = String.fromCharCode(asciival);
		jsonRtn->Set(String::NewSymbol("CombOffsetFlag"), String::New(pInputOrder->CombOffsetFlag));
		jsonRtn->Set(String::NewSymbol("CombHedgeFlag"), String::New(pInputOrder->CombHedgeFlag));
		jsonRtn->Set(String::NewSymbol("LimitPrice"), Number::New(pInputOrder->LimitPrice));
		jsonRtn->Set(String::NewSymbol("VolumeTotalOriginal"), Int32::New(pInputOrder->VolumeTotalOriginal));
		jsonRtn->Set(String::NewSymbol("TimeCondition"), Int32::New(pInputOrder->TimeCondition));
		jsonRtn->Set(String::NewSymbol("GTDDate"), String::New(pInputOrder->GTDDate));
		jsonRtn->Set(String::NewSymbol("VolumeCondition"), Int32::New(pInputOrder->VolumeCondition));
		jsonRtn->Set(String::NewSymbol("MinVolume"), Int32::New(pInputOrder->MinVolume));
		jsonRtn->Set(String::NewSymbol("ContingentCondition"), Int32::New(pInputOrder->ContingentCondition));
		jsonRtn->Set(String::NewSymbol("StopPrice"), Number::New(pInputOrder->StopPrice));
		jsonRtn->Set(String::NewSymbol("ForceCloseReason"), Int32::New(pInputOrder->ForceCloseReason));
		jsonRtn->Set(String::NewSymbol("IsAutoSuspend"), Int32::New(pInputOrder->IsAutoSuspend));
		jsonRtn->Set(String::NewSymbol("BusinessUnit"), String::New(pInputOrder->BusinessUnit));
		jsonRtn->Set(String::NewSymbol("RequestID"), Int32::New(pInputOrder->RequestID));
		jsonRtn->Set(String::NewSymbol("UserForceClose"), Int32::New(pInputOrder->UserForceClose));
		jsonRtn->Set(String::NewSymbol("IsSwapOrder"), Int32::New(pInputOrder->IsSwapOrder));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined()->ToObject();
	}
	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapTrader::pkg_cb_errorderinsert(CbRtnField* data, Local<Value>*cbArray) {
	CThostFtdcInputOrderField* pInputOrder = static_cast<CThostFtdcInputOrderField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pInputOrder) {
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pInputOrder->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(pInputOrder->InvestorID));
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pInputOrder->InstrumentID));
		jsonRtn->Set(String::NewSymbol("OrderRef"), String::New(pInputOrder->OrderRef));
		jsonRtn->Set(String::NewSymbol("UserID"), String::New(pInputOrder->UserID));
		jsonRtn->Set(String::NewSymbol("OrderPriceType"), Int32::New(pInputOrder->OrderPriceType));
		jsonRtn->Set(String::NewSymbol("Direction"), Int32::New(pInputOrder->Direction));  //var charval = String.fromCharCode(asciival);
		jsonRtn->Set(String::NewSymbol("CombOffsetFlag"), String::New(pInputOrder->CombOffsetFlag));
		jsonRtn->Set(String::NewSymbol("CombHedgeFlag"), String::New(pInputOrder->CombHedgeFlag));
		jsonRtn->Set(String::NewSymbol("LimitPrice"), Number::New(pInputOrder->LimitPrice));
		jsonRtn->Set(String::NewSymbol("VolumeTotalOriginal"), Int32::New(pInputOrder->VolumeTotalOriginal));
		jsonRtn->Set(String::NewSymbol("TimeCondition"), Int32::New(pInputOrder->TimeCondition));
		jsonRtn->Set(String::NewSymbol("GTDDate"), String::New(pInputOrder->GTDDate));
		jsonRtn->Set(String::NewSymbol("VolumeCondition"), Int32::New(pInputOrder->VolumeCondition));
		jsonRtn->Set(String::NewSymbol("MinVolume"), Int32::New(pInputOrder->MinVolume));
		jsonRtn->Set(String::NewSymbol("ContingentCondition"), Int32::New(pInputOrder->ContingentCondition));
		jsonRtn->Set(String::NewSymbol("StopPrice"), Number::New(pInputOrder->StopPrice));
		jsonRtn->Set(String::NewSymbol("ForceCloseReason"), Int32::New(pInputOrder->ForceCloseReason));
		jsonRtn->Set(String::NewSymbol("IsAutoSuspend"), Int32::New(pInputOrder->IsAutoSuspend));
		jsonRtn->Set(String::NewSymbol("BusinessUnit"), String::New(pInputOrder->BusinessUnit));
		jsonRtn->Set(String::NewSymbol("RequestID"), Int32::New(pInputOrder->RequestID));
		jsonRtn->Set(String::NewSymbol("UserForceClose"), Int32::New(pInputOrder->UserForceClose));
		jsonRtn->Set(String::NewSymbol("IsSwapOrder"), Int32::New(pInputOrder->IsSwapOrder));
		*cbArray = jsonRtn;
	}
	else {
		*cbArray = Undefined()->ToObject();
	}
	*(cbArray + 1) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapTrader::pkg_cb_orderaction(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
	CThostFtdcInputOrderActionField* pInputOrderAction = static_cast<CThostFtdcInputOrderActionField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pInputOrderAction) {
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pInputOrderAction->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(pInputOrderAction->InvestorID));
		jsonRtn->Set(String::NewSymbol("OrderActionRef"), Int32::New(pInputOrderAction->OrderActionRef));
		jsonRtn->Set(String::NewSymbol("OrderRef"), String::New(pInputOrderAction->OrderRef));
		jsonRtn->Set(String::NewSymbol("RequestID"), Int32::New(pInputOrderAction->RequestID));
		jsonRtn->Set(String::NewSymbol("FrontID"), Int32::New(pInputOrderAction->FrontID));
		jsonRtn->Set(String::NewSymbol("SessionID"), Int32::New(pInputOrderAction->SessionID));
		jsonRtn->Set(String::NewSymbol("ExchangeID"), String::New(pInputOrderAction->ExchangeID));
		jsonRtn->Set(String::NewSymbol("OrderSysID"), String::New(pInputOrderAction->OrderSysID));
		jsonRtn->Set(String::NewSymbol("ActionFlag"), Int32::New(pInputOrderAction->ActionFlag));
		jsonRtn->Set(String::NewSymbol("LimitPrice"), Number::New(pInputOrderAction->LimitPrice));
		jsonRtn->Set(String::NewSymbol("VolumeChange"), Int32::New(pInputOrderAction->VolumeChange));
		jsonRtn->Set(String::NewSymbol("UserID"), String::New(pInputOrderAction->UserID));
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pInputOrderAction->InstrumentID));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined()->ToObject();
	}
	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapTrader::pkg_cb_errorderaction(CbRtnField* data, Local<Value>*cbArray) {
	CThostFtdcOrderActionField* pOrderAction = static_cast<CThostFtdcOrderActionField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pOrderAction) {
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pOrderAction->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(pOrderAction->InvestorID));
		jsonRtn->Set(String::NewSymbol("OrderActionRef"), Int32::New(pOrderAction->OrderActionRef));
		jsonRtn->Set(String::NewSymbol("OrderRef"), String::New(pOrderAction->OrderRef));
		jsonRtn->Set(String::NewSymbol("RequestID"), Int32::New(pOrderAction->RequestID));
		jsonRtn->Set(String::NewSymbol("FrontID"), Int32::New(pOrderAction->FrontID));
		jsonRtn->Set(String::NewSymbol("SessionID"), Int32::New(pOrderAction->SessionID));
		jsonRtn->Set(String::NewSymbol("ExchangeID"), String::New(pOrderAction->ExchangeID));
		jsonRtn->Set(String::NewSymbol("OrderSysID"), String::New(pOrderAction->OrderSysID));
		jsonRtn->Set(String::NewSymbol("ActionFlag"), Int32::New(pOrderAction->ActionFlag));
		jsonRtn->Set(String::NewSymbol("LimitPrice"), Number::New(pOrderAction->LimitPrice));
		jsonRtn->Set(String::NewSymbol("VolumeChange"), Int32::New(pOrderAction->VolumeChange));
		jsonRtn->Set(String::NewSymbol("ActionDate"), String::New(pOrderAction->ActionDate));
		jsonRtn->Set(String::NewSymbol("TraderID"), String::New(pOrderAction->TraderID));
		jsonRtn->Set(String::NewSymbol("InstallID"), Int32::New(pOrderAction->InstallID));
		jsonRtn->Set(String::NewSymbol("OrderLocalID"), String::New(pOrderAction->OrderLocalID));
		jsonRtn->Set(String::NewSymbol("ActionLocalID"), String::New(pOrderAction->ActionLocalID));
		jsonRtn->Set(String::NewSymbol("ParticipantID"), String::New(pOrderAction->ParticipantID));
		jsonRtn->Set(String::NewSymbol("ClientID"), String::New(pOrderAction->ClientID));
		jsonRtn->Set(String::NewSymbol("BusinessUnit"), String::New(pOrderAction->BusinessUnit));
		jsonRtn->Set(String::NewSymbol("OrderActionStatus"), Int32::New(pOrderAction->OrderActionStatus));
		jsonRtn->Set(String::NewSymbol("UserID"), String::New(pOrderAction->UserID));
		jsonRtn->Set(String::NewSymbol("StatusMsg"), String::New(pOrderAction->StatusMsg));
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pOrderAction->InstrumentID));
		*cbArray = jsonRtn;
	}
	else {
		*cbArray = Undefined()->ToObject();
	}
	*(cbArray + 1) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapTrader::pkg_cb_rspqryorder(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
	CThostFtdcOrderField* pOrder = static_cast<CThostFtdcOrderField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pOrder) {
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pOrder->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(pOrder->InvestorID));
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pOrder->InstrumentID));
		jsonRtn->Set(String::NewSymbol("OrderRef"), String::New(pOrder->OrderRef));
		jsonRtn->Set(String::NewSymbol("UserID"), String::New(pOrder->UserID));
		jsonRtn->Set(String::NewSymbol("OrderPriceType"), Int32::New(pOrder->OrderPriceType));
		jsonRtn->Set(String::NewSymbol("Direction"), Int32::New(pOrder->Direction));  //var charval = String.fromCharCode(asciival);
		jsonRtn->Set(String::NewSymbol("CombOffsetFlag"), String::New(pOrder->CombOffsetFlag));
		jsonRtn->Set(String::NewSymbol("CombHedgeFlag"), String::New(pOrder->CombHedgeFlag));
		jsonRtn->Set(String::NewSymbol("LimitPrice"), Number::New(pOrder->LimitPrice));
		jsonRtn->Set(String::NewSymbol("VolumeTotalOriginal"), Int32::New(pOrder->VolumeTotalOriginal));
		jsonRtn->Set(String::NewSymbol("TimeCondition"), Int32::New(pOrder->TimeCondition));
		jsonRtn->Set(String::NewSymbol("GTDDate"), String::New(pOrder->GTDDate));
		jsonRtn->Set(String::NewSymbol("VolumeCondition"), Int32::New(pOrder->VolumeCondition));
		jsonRtn->Set(String::NewSymbol("MinVolume"), Int32::New(pOrder->MinVolume));
		jsonRtn->Set(String::NewSymbol("ContingentCondition"), Int32::New(pOrder->ContingentCondition));
		jsonRtn->Set(String::NewSymbol("StopPrice"), Number::New(pOrder->StopPrice));
		jsonRtn->Set(String::NewSymbol("ForceCloseReason"), Int32::New(pOrder->ForceCloseReason));
		jsonRtn->Set(String::NewSymbol("IsAutoSuspend"), Int32::New(pOrder->IsAutoSuspend));
		jsonRtn->Set(String::NewSymbol("BusinessUnit"), String::New(pOrder->BusinessUnit));
		jsonRtn->Set(String::NewSymbol("RequestID"), Int32::New(pOrder->RequestID));
		jsonRtn->Set(String::NewSymbol("OrderLocalID"), String::New(pOrder->OrderLocalID));
		jsonRtn->Set(String::NewSymbol("ExchangeID"), String::New(pOrder->ExchangeID));
		jsonRtn->Set(String::NewSymbol("ParticipantID"), String::New(pOrder->ParticipantID));
		jsonRtn->Set(String::NewSymbol("ClientID"), String::New(pOrder->ClientID));
		jsonRtn->Set(String::NewSymbol("ExchangeInstID"), String::New(pOrder->ExchangeInstID));
		jsonRtn->Set(String::NewSymbol("TraderID"), String::New(pOrder->TraderID));
		jsonRtn->Set(String::NewSymbol("InstallID"), Int32::New(pOrder->InstallID));
		jsonRtn->Set(String::NewSymbol("OrderSubmitStatus"), Int32::New(pOrder->OrderSubmitStatus));
		jsonRtn->Set(String::NewSymbol("NotifySequence"), Int32::New(pOrder->NotifySequence));
		jsonRtn->Set(String::NewSymbol("TradingDay"), String::New(pOrder->TradingDay));
		jsonRtn->Set(String::NewSymbol("SettlementID"), Int32::New(pOrder->SettlementID));
		jsonRtn->Set(String::NewSymbol("OrderSysID"), String::New(pOrder->OrderSysID));
		jsonRtn->Set(String::NewSymbol("OrderSource"), Int32::New(pOrder->OrderSource));
		jsonRtn->Set(String::NewSymbol("OrderStatus"), Int32::New(pOrder->OrderStatus));
		jsonRtn->Set(String::NewSymbol("OrderType"), Int32::New(pOrder->OrderType));
		jsonRtn->Set(String::NewSymbol("VolumeTraded"), Int32::New(pOrder->VolumeTraded));
		jsonRtn->Set(String::NewSymbol("VolumeTotal"), Int32::New(pOrder->VolumeTotal));
		jsonRtn->Set(String::NewSymbol("InsertDate"), String::New(pOrder->InsertDate));
		jsonRtn->Set(String::NewSymbol("InsertTime"), String::New(pOrder->InsertTime));
		jsonRtn->Set(String::NewSymbol("ActiveTime"), String::New(pOrder->ActiveTime));
		jsonRtn->Set(String::NewSymbol("SuspendTime"), String::New(pOrder->SuspendTime));
		jsonRtn->Set(String::NewSymbol("UpdateTime"), String::New(pOrder->UpdateTime));
		jsonRtn->Set(String::NewSymbol("CancelTime"), String::New(pOrder->CancelTime));
		jsonRtn->Set(String::NewSymbol("ActiveTraderID"), String::New(pOrder->ActiveTraderID));
		jsonRtn->Set(String::NewSymbol("ClearingPartID"), String::New(pOrder->ClearingPartID));
		jsonRtn->Set(String::NewSymbol("SequenceNo"), Int32::New(pOrder->SequenceNo));
		jsonRtn->Set(String::NewSymbol("FrontID"), Int32::New(pOrder->FrontID));
		jsonRtn->Set(String::NewSymbol("SessionID"), Int32::New(pOrder->SessionID));
		jsonRtn->Set(String::NewSymbol("UserProductInfo"), String::New(pOrder->UserProductInfo));
		jsonRtn->Set(String::NewSymbol("StatusMsg"), String::New(pOrder->StatusMsg));
		jsonRtn->Set(String::NewSymbol("UserForceClose"), Int32::New(pOrder->UserForceClose));
		jsonRtn->Set(String::NewSymbol("ActiveUserID"), String::New(pOrder->ActiveUserID));
		jsonRtn->Set(String::NewSymbol("BrokerOrderSeq"), Int32::New(pOrder->BrokerOrderSeq));
		jsonRtn->Set(String::NewSymbol("RelativeOrderSysID"), String::New(pOrder->RelativeOrderSysID));
		jsonRtn->Set(String::NewSymbol("ZCETotalTradedVolume"), Int32::New(pOrder->ZCETotalTradedVolume));
		jsonRtn->Set(String::NewSymbol("IsSwapOrder"), Int32::New(pOrder->IsSwapOrder));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined()->ToObject();
	}
	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapTrader::pkg_cb_rtnorder(CbRtnField* data, Local<Value>*cbArray) {
	CThostFtdcOrderField* pOrder = static_cast<CThostFtdcOrderField*>(data->rtnField);
	if (pOrder) {
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pOrder->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(pOrder->InvestorID));
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pOrder->InstrumentID));
		jsonRtn->Set(String::NewSymbol("OrderRef"), String::New(pOrder->OrderRef));
		jsonRtn->Set(String::NewSymbol("UserID"), String::New(pOrder->UserID));
		jsonRtn->Set(String::NewSymbol("OrderPriceType"), Int32::New(pOrder->OrderPriceType));
		jsonRtn->Set(String::NewSymbol("Direction"), Int32::New(pOrder->Direction));  //var charval = String.fromCharCode(asciival);
		jsonRtn->Set(String::NewSymbol("CombOffsetFlag"), String::New(pOrder->CombOffsetFlag));
		jsonRtn->Set(String::NewSymbol("CombHedgeFlag"), String::New(pOrder->CombHedgeFlag));
		jsonRtn->Set(String::NewSymbol("LimitPrice"), Number::New(pOrder->LimitPrice));
		jsonRtn->Set(String::NewSymbol("VolumeTotalOriginal"), Int32::New(pOrder->VolumeTotalOriginal));
		jsonRtn->Set(String::NewSymbol("TimeCondition"), Int32::New(pOrder->TimeCondition));
		jsonRtn->Set(String::NewSymbol("GTDDate"), String::New(pOrder->GTDDate));
		jsonRtn->Set(String::NewSymbol("VolumeCondition"), Int32::New(pOrder->VolumeCondition));
		jsonRtn->Set(String::NewSymbol("MinVolume"), Int32::New(pOrder->MinVolume));
		jsonRtn->Set(String::NewSymbol("ContingentCondition"), Int32::New(pOrder->ContingentCondition));
		jsonRtn->Set(String::NewSymbol("StopPrice"), Number::New(pOrder->StopPrice));
		jsonRtn->Set(String::NewSymbol("ForceCloseReason"), Int32::New(pOrder->ForceCloseReason));
		jsonRtn->Set(String::NewSymbol("IsAutoSuspend"), Int32::New(pOrder->IsAutoSuspend));
		jsonRtn->Set(String::NewSymbol("BusinessUnit"), String::New(pOrder->BusinessUnit));
		jsonRtn->Set(String::NewSymbol("RequestID"), Int32::New(pOrder->RequestID));
		jsonRtn->Set(String::NewSymbol("OrderLocalID"), String::New(pOrder->OrderLocalID));
		jsonRtn->Set(String::NewSymbol("ExchangeID"), String::New(pOrder->ExchangeID));
		jsonRtn->Set(String::NewSymbol("ParticipantID"), String::New(pOrder->ParticipantID));
		jsonRtn->Set(String::NewSymbol("ClientID"), String::New(pOrder->ClientID));
		jsonRtn->Set(String::NewSymbol("ExchangeInstID"), String::New(pOrder->ExchangeInstID));
		jsonRtn->Set(String::NewSymbol("TraderID"), String::New(pOrder->TraderID));
		jsonRtn->Set(String::NewSymbol("InstallID"), Int32::New(pOrder->InstallID));
		jsonRtn->Set(String::NewSymbol("OrderSubmitStatus"), Int32::New(pOrder->OrderSubmitStatus));
		jsonRtn->Set(String::NewSymbol("NotifySequence"), Int32::New(pOrder->NotifySequence));
		jsonRtn->Set(String::NewSymbol("TradingDay"), String::New(pOrder->TradingDay));
		jsonRtn->Set(String::NewSymbol("SettlementID"), Int32::New(pOrder->SettlementID));
		jsonRtn->Set(String::NewSymbol("OrderSysID"), String::New(pOrder->OrderSysID));
		jsonRtn->Set(String::NewSymbol("OrderSource"), Int32::New(pOrder->OrderSource));
		jsonRtn->Set(String::NewSymbol("OrderStatus"), Int32::New(pOrder->OrderStatus));
		jsonRtn->Set(String::NewSymbol("OrderType"), Int32::New(pOrder->OrderType));
		jsonRtn->Set(String::NewSymbol("VolumeTraded"), Int32::New(pOrder->VolumeTraded));
		jsonRtn->Set(String::NewSymbol("VolumeTotal"), Int32::New(pOrder->VolumeTotal));
		jsonRtn->Set(String::NewSymbol("InsertDate"), String::New(pOrder->InsertDate));
		jsonRtn->Set(String::NewSymbol("InsertTime"), String::New(pOrder->InsertTime));
		jsonRtn->Set(String::NewSymbol("ActiveTime"), String::New(pOrder->ActiveTime));
		jsonRtn->Set(String::NewSymbol("SuspendTime"), String::New(pOrder->SuspendTime));
		jsonRtn->Set(String::NewSymbol("UpdateTime"), String::New(pOrder->UpdateTime));
		jsonRtn->Set(String::NewSymbol("CancelTime"), String::New(pOrder->CancelTime));
		jsonRtn->Set(String::NewSymbol("ActiveTraderID"), String::New(pOrder->ActiveTraderID));
		jsonRtn->Set(String::NewSymbol("ClearingPartID"), String::New(pOrder->ClearingPartID));
		jsonRtn->Set(String::NewSymbol("SequenceNo"), Int32::New(pOrder->SequenceNo));
		jsonRtn->Set(String::NewSymbol("FrontID"), Int32::New(pOrder->FrontID));
		jsonRtn->Set(String::NewSymbol("SessionID"), Int32::New(pOrder->SessionID));
		jsonRtn->Set(String::NewSymbol("UserProductInfo"), String::New(pOrder->UserProductInfo));
		jsonRtn->Set(String::NewSymbol("StatusMsg"), String::New(pOrder->StatusMsg));
		jsonRtn->Set(String::NewSymbol("UserForceClose"), Int32::New(pOrder->UserForceClose));
		jsonRtn->Set(String::NewSymbol("ActiveUserID"), String::New(pOrder->ActiveUserID));
		jsonRtn->Set(String::NewSymbol("BrokerOrderSeq"), Int32::New(pOrder->BrokerOrderSeq));
		jsonRtn->Set(String::NewSymbol("RelativeOrderSysID"), String::New(pOrder->RelativeOrderSysID));
		jsonRtn->Set(String::NewSymbol("ZCETotalTradedVolume"), Int32::New(pOrder->ZCETotalTradedVolume));
		jsonRtn->Set(String::NewSymbol("IsSwapOrder"), Int32::New(pOrder->IsSwapOrder));
		*cbArray = jsonRtn;
	}
	else {
		*cbArray = Undefined()->ToObject();
	}
	return;
}
void WrapTrader::pkg_cb_rqtrade(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
	CThostFtdcTradeField* pTrade = static_cast<CThostFtdcTradeField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pTrade) {
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pTrade->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(pTrade->InvestorID));
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pTrade->InstrumentID));
		jsonRtn->Set(String::NewSymbol("OrderRef"), String::New(pTrade->OrderRef));
		jsonRtn->Set(String::NewSymbol("UserID"), String::New(pTrade->UserID));
		jsonRtn->Set(String::NewSymbol("ExchangeID"), String::New(pTrade->ExchangeID));
		jsonRtn->Set(String::NewSymbol("TradeID"), String::New(pTrade->TradeID));
		jsonRtn->Set(String::NewSymbol("Direction"), Int32::New(pTrade->Direction));  //var charval = String.fromCharCode(asciival);
		jsonRtn->Set(String::NewSymbol("OrderSysID"), String::New(pTrade->OrderSysID));
		jsonRtn->Set(String::NewSymbol("ParticipantID"), String::New(pTrade->ParticipantID));
		jsonRtn->Set(String::NewSymbol("ClientID"), String::New(pTrade->ClientID));
		jsonRtn->Set(String::NewSymbol("TradingRole"), Int32::New(pTrade->TradingRole));
		jsonRtn->Set(String::NewSymbol("ExchangeInstID"), String::New(pTrade->ExchangeInstID));
		jsonRtn->Set(String::NewSymbol("OffsetFlag"), Int32::New(pTrade->OffsetFlag));
		jsonRtn->Set(String::NewSymbol("HedgeFlag"), Int32::New(pTrade->HedgeFlag));
		jsonRtn->Set(String::NewSymbol("Price"), Number::New(pTrade->Price));
		jsonRtn->Set(String::NewSymbol("Volume"), Int32::New(pTrade->Volume));
		jsonRtn->Set(String::NewSymbol("TradeDate"), String::New(pTrade->TradeDate));
		jsonRtn->Set(String::NewSymbol("TradeTime"), String::New(pTrade->TradeTime));
		jsonRtn->Set(String::NewSymbol("TradeType"), Int32::New(pTrade->TradeType));
		jsonRtn->Set(String::NewSymbol("PriceSource"), Int32::New(pTrade->PriceSource));
		jsonRtn->Set(String::NewSymbol("TraderID"), String::New(pTrade->TraderID));
		jsonRtn->Set(String::NewSymbol("OrderLocalID"), String::New(pTrade->OrderLocalID));
		jsonRtn->Set(String::NewSymbol("ClearingPartID"), String::New(pTrade->ClearingPartID));
		jsonRtn->Set(String::NewSymbol("BusinessUnit"), String::New(pTrade->BusinessUnit));
		jsonRtn->Set(String::NewSymbol("SequenceNo"), Int32::New(pTrade->SequenceNo));
		jsonRtn->Set(String::NewSymbol("TradingDay"), String::New(pTrade->TradingDay));
		jsonRtn->Set(String::NewSymbol("SettlementID"), Int32::New(pTrade->SettlementID));
		jsonRtn->Set(String::NewSymbol("BrokerOrderSeq"), Int32::New(pTrade->BrokerOrderSeq));
		jsonRtn->Set(String::NewSymbol("TradeSource"), Int32::New(pTrade->TradeSource));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined()->ToObject();
	}
	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapTrader::pkg_cb_rtntrade(CbRtnField* data, Local<Value>*cbArray) {
	CThostFtdcTradeField* pTrade = static_cast<CThostFtdcTradeField*>(data->rtnField);
	if (pTrade) {
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pTrade->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(pTrade->InvestorID));
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pTrade->InstrumentID));
		jsonRtn->Set(String::NewSymbol("OrderRef"), String::New(pTrade->OrderRef));
		jsonRtn->Set(String::NewSymbol("UserID"), String::New(pTrade->UserID));
		jsonRtn->Set(String::NewSymbol("ExchangeID"), String::New(pTrade->ExchangeID));
		jsonRtn->Set(String::NewSymbol("TradeID"), String::New(pTrade->TradeID));
		jsonRtn->Set(String::NewSymbol("Direction"), Int32::New(pTrade->Direction));  //var charval = String.fromCharCode(asciival);
		jsonRtn->Set(String::NewSymbol("OrderSysID"), String::New(pTrade->OrderSysID));
		jsonRtn->Set(String::NewSymbol("ParticipantID"), String::New(pTrade->ParticipantID));
		jsonRtn->Set(String::NewSymbol("ClientID"), String::New(pTrade->ClientID));
		jsonRtn->Set(String::NewSymbol("TradingRole"), Int32::New(pTrade->TradingRole));
		jsonRtn->Set(String::NewSymbol("ExchangeInstID"), String::New(pTrade->ExchangeInstID));
		jsonRtn->Set(String::NewSymbol("OffsetFlag"), Int32::New(pTrade->OffsetFlag));
		jsonRtn->Set(String::NewSymbol("HedgeFlag"), Int32::New(pTrade->HedgeFlag));
		jsonRtn->Set(String::NewSymbol("Price"), Number::New(pTrade->Price));
		jsonRtn->Set(String::NewSymbol("Volume"), Int32::New(pTrade->Volume));
		jsonRtn->Set(String::NewSymbol("TradeDate"), String::New(pTrade->TradeDate));
		jsonRtn->Set(String::NewSymbol("TradeTime"), String::New(pTrade->TradeTime));
		jsonRtn->Set(String::NewSymbol("TradeType"), Int32::New(pTrade->TradeType));
		jsonRtn->Set(String::NewSymbol("PriceSource"), Int32::New(pTrade->PriceSource));
		jsonRtn->Set(String::NewSymbol("TraderID"), String::New(pTrade->TraderID));
		jsonRtn->Set(String::NewSymbol("OrderLocalID"), String::New(pTrade->OrderLocalID));
		jsonRtn->Set(String::NewSymbol("ClearingPartID"), String::New(pTrade->ClearingPartID));
		jsonRtn->Set(String::NewSymbol("BusinessUnit"), String::New(pTrade->BusinessUnit));
		jsonRtn->Set(String::NewSymbol("SequenceNo"), Int32::New(pTrade->SequenceNo));
		jsonRtn->Set(String::NewSymbol("TradingDay"), String::New(pTrade->TradingDay));
		jsonRtn->Set(String::NewSymbol("SettlementID"), Int32::New(pTrade->SettlementID));
		jsonRtn->Set(String::NewSymbol("BrokerOrderSeq"), Int32::New(pTrade->BrokerOrderSeq));
		jsonRtn->Set(String::NewSymbol("TradeSource"), Int32::New(pTrade->TradeSource));
		*cbArray = jsonRtn;
	}
	else {
		*cbArray = Undefined()->ToObject();
	}
	 
	return;
}
void WrapTrader::pkg_cb_rqinvestorposition(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
	CThostFtdcInvestorPositionField* _pInvestorPosition = static_cast<CThostFtdcInvestorPositionField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (_pInvestorPosition) {
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(_pInvestorPosition->InstrumentID));
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(_pInvestorPosition->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(_pInvestorPosition->InvestorID));
		jsonRtn->Set(String::NewSymbol("PosiDirection"), Int32::New(_pInvestorPosition->PosiDirection));
		jsonRtn->Set(String::NewSymbol("HedgeFlag"), Int32::New(_pInvestorPosition->HedgeFlag));
		jsonRtn->Set(String::NewSymbol("PositionDate"), Int32::New(_pInvestorPosition->PositionDate));
		jsonRtn->Set(String::NewSymbol("YdPosition"), Int32::New(_pInvestorPosition->YdPosition));
		jsonRtn->Set(String::NewSymbol("Position"), Int32::New(_pInvestorPosition->Position));
		jsonRtn->Set(String::NewSymbol("LongFrozen"), Int32::New(_pInvestorPosition->LongFrozen));
		jsonRtn->Set(String::NewSymbol("ShortFrozen"), Int32::New(_pInvestorPosition->ShortFrozen));
		jsonRtn->Set(String::NewSymbol("LongFrozenAmount"), Number::New(_pInvestorPosition->LongFrozenAmount));
		jsonRtn->Set(String::NewSymbol("ShortFrozenAmount"), Number::New(_pInvestorPosition->ShortFrozenAmount));
		jsonRtn->Set(String::NewSymbol("OpenVolume"), Int32::New(_pInvestorPosition->OpenVolume));
		jsonRtn->Set(String::NewSymbol("CloseVolume"), Int32::New(_pInvestorPosition->CloseVolume));
		jsonRtn->Set(String::NewSymbol("OpenAmount"), Number::New(_pInvestorPosition->OpenAmount));
		jsonRtn->Set(String::NewSymbol("CloseAmount"), Number::New(_pInvestorPosition->CloseAmount));
		jsonRtn->Set(String::NewSymbol("PositionCost"), Number::New(_pInvestorPosition->PositionCost));
		jsonRtn->Set(String::NewSymbol("PreMargin"), Number::New(_pInvestorPosition->PreMargin));
		jsonRtn->Set(String::NewSymbol("UseMargin"), Number::New(_pInvestorPosition->UseMargin));
		jsonRtn->Set(String::NewSymbol("FrozenMargin"), Number::New(_pInvestorPosition->FrozenMargin));
		jsonRtn->Set(String::NewSymbol("FrozenCash"), Number::New(_pInvestorPosition->FrozenCash));
		jsonRtn->Set(String::NewSymbol("FrozenCommission"), Number::New(_pInvestorPosition->FrozenCommission));
		jsonRtn->Set(String::NewSymbol("CashIn"), Number::New(_pInvestorPosition->CashIn));
		jsonRtn->Set(String::NewSymbol("Commission"), Number::New(_pInvestorPosition->Commission));
		jsonRtn->Set(String::NewSymbol("CloseProfit"), Number::New(_pInvestorPosition->CloseProfit));
		jsonRtn->Set(String::NewSymbol("PositionProfit"), Number::New(_pInvestorPosition->PositionProfit));
		jsonRtn->Set(String::NewSymbol("PreSettlementPrice"), Number::New(_pInvestorPosition->PreSettlementPrice));
		jsonRtn->Set(String::NewSymbol("SettlementPrice"), Number::New(_pInvestorPosition->SettlementPrice));
		jsonRtn->Set(String::NewSymbol("TradingDay"), String::New(_pInvestorPosition->TradingDay));
		jsonRtn->Set(String::NewSymbol("SettlementID"), Int32::New(_pInvestorPosition->SettlementID));
		jsonRtn->Set(String::NewSymbol("OpenCost"), Number::New(_pInvestorPosition->OpenCost));
		jsonRtn->Set(String::NewSymbol("ExchangeMargin"), Number::New(_pInvestorPosition->ExchangeMargin));
		jsonRtn->Set(String::NewSymbol("CombPosition"), Int32::New(_pInvestorPosition->CombPosition));
		jsonRtn->Set(String::NewSymbol("CombLongFrozen"), Int32::New(_pInvestorPosition->CombLongFrozen));
		jsonRtn->Set(String::NewSymbol("CombShortFrozen"), Int32::New(_pInvestorPosition->CombShortFrozen));
		jsonRtn->Set(String::NewSymbol("CloseProfitByDate"), Number::New(_pInvestorPosition->CloseProfitByDate));
		jsonRtn->Set(String::NewSymbol("CloseProfitByTrade"), Number::New(_pInvestorPosition->CloseProfitByTrade));
		jsonRtn->Set(String::NewSymbol("TodayPosition"), Int32::New(_pInvestorPosition->TodayPosition));
		jsonRtn->Set(String::NewSymbol("MarginRateByMoney"), Number::New(_pInvestorPosition->MarginRateByMoney));
		jsonRtn->Set(String::NewSymbol("MarginRateByVolume"), Number::New(_pInvestorPosition->MarginRateByVolume));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined()->ToObject();
	}
	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapTrader::pkg_cb_rqinvestorpositiondetail(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
	CThostFtdcInvestorPositionDetailField* pInvestorPositionDetail = static_cast<CThostFtdcInvestorPositionDetailField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pInvestorPositionDetail) {
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pInvestorPositionDetail->InstrumentID));
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pInvestorPositionDetail->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(pInvestorPositionDetail->InvestorID));
		jsonRtn->Set(String::NewSymbol("HedgeFlag"), Int32::New(pInvestorPositionDetail->HedgeFlag));
		jsonRtn->Set(String::NewSymbol("Direction"), Int32::New(pInvestorPositionDetail->Direction));  //var charval = String.fromCharCode(asciival);
		jsonRtn->Set(String::NewSymbol("OpenDate"), String::New(pInvestorPositionDetail->OpenDate));
		jsonRtn->Set(String::NewSymbol("TradeID"), String::New(pInvestorPositionDetail->TradeID));
		jsonRtn->Set(String::NewSymbol("Volume"), Int32::New(pInvestorPositionDetail->Volume));
		jsonRtn->Set(String::NewSymbol("OpenPrice"), Number::New(pInvestorPositionDetail->OpenPrice));
		jsonRtn->Set(String::NewSymbol("TradingDay"), String::New(pInvestorPositionDetail->TradingDay));
		jsonRtn->Set(String::NewSymbol("SettlementID"), Int32::New(pInvestorPositionDetail->SettlementID));
		jsonRtn->Set(String::NewSymbol("TradeType"), Int32::New(pInvestorPositionDetail->TradeType));
		jsonRtn->Set(String::NewSymbol("CombInstrumentID"), String::New(pInvestorPositionDetail->CombInstrumentID));
		jsonRtn->Set(String::NewSymbol("ExchangeID"), String::New(pInvestorPositionDetail->ExchangeID));
		jsonRtn->Set(String::NewSymbol("CloseProfitByDate"), Number::New(pInvestorPositionDetail->CloseProfitByDate));
		jsonRtn->Set(String::NewSymbol("CloseProfitByTrade"), Number::New(pInvestorPositionDetail->CloseProfitByTrade));
		jsonRtn->Set(String::NewSymbol("PositionProfitByDate"), Number::New(pInvestorPositionDetail->PositionProfitByDate));
		jsonRtn->Set(String::NewSymbol("PositionProfitByTrade"), Number::New(pInvestorPositionDetail->PositionProfitByTrade));
		jsonRtn->Set(String::NewSymbol("Margin"), Number::New(pInvestorPositionDetail->Margin));
		jsonRtn->Set(String::NewSymbol("ExchMargin"), Number::New(pInvestorPositionDetail->ExchMargin));
		jsonRtn->Set(String::NewSymbol("MarginRateByMoney"), Number::New(pInvestorPositionDetail->MarginRateByMoney));
		jsonRtn->Set(String::NewSymbol("MarginRateByVolume"), Number::New(pInvestorPositionDetail->MarginRateByVolume));
		jsonRtn->Set(String::NewSymbol("LastSettlementPrice"), Number::New(pInvestorPositionDetail->LastSettlementPrice));
		jsonRtn->Set(String::NewSymbol("SettlementPrice"), Number::New(pInvestorPositionDetail->SettlementPrice));
		jsonRtn->Set(String::NewSymbol("CloseVolume"), Int32::New(pInvestorPositionDetail->CloseVolume));
		jsonRtn->Set(String::NewSymbol("CloseAmount"), Number::New(pInvestorPositionDetail->CloseAmount));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined()->ToObject();
	}
	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapTrader::pkg_cb_rqtradingaccount(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
	CThostFtdcTradingAccountField *pTradingAccount = static_cast<CThostFtdcTradingAccountField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pTradingAccount) {
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pTradingAccount->BrokerID));
		jsonRtn->Set(String::NewSymbol("AccountID"), String::New(pTradingAccount->AccountID));
		jsonRtn->Set(String::NewSymbol("PreMortgage"), Number::New(pTradingAccount->PreMortgage));
		jsonRtn->Set(String::NewSymbol("PreCredit"), Number::New(pTradingAccount->PreCredit));
		jsonRtn->Set(String::NewSymbol("PreDeposit"), Number::New(pTradingAccount->PreDeposit));
		jsonRtn->Set(String::NewSymbol("PreBalance"), Number::New(pTradingAccount->PreBalance));
		jsonRtn->Set(String::NewSymbol("PreMargin"), Number::New(pTradingAccount->PreMargin));
		jsonRtn->Set(String::NewSymbol("InterestBase"), Number::New(pTradingAccount->InterestBase));
		jsonRtn->Set(String::NewSymbol("Interest"), Number::New(pTradingAccount->Interest));
		jsonRtn->Set(String::NewSymbol("Deposit"), Number::New(pTradingAccount->Deposit));
		jsonRtn->Set(String::NewSymbol("Withdraw"), Number::New(pTradingAccount->Withdraw));
		jsonRtn->Set(String::NewSymbol("FrozenMargin"), Number::New(pTradingAccount->FrozenMargin));
		jsonRtn->Set(String::NewSymbol("FrozenCash"), Number::New(pTradingAccount->FrozenCash));
		jsonRtn->Set(String::NewSymbol("FrozenCommission"), Number::New(pTradingAccount->FrozenCommission));
		jsonRtn->Set(String::NewSymbol("CurrMargin"), Number::New(pTradingAccount->CurrMargin));
		jsonRtn->Set(String::NewSymbol("CashIn"), Number::New(pTradingAccount->CashIn));
		jsonRtn->Set(String::NewSymbol("Commission"), Number::New(pTradingAccount->Commission));
		jsonRtn->Set(String::NewSymbol("CloseProfit"), Number::New(pTradingAccount->CloseProfit));
		jsonRtn->Set(String::NewSymbol("PositionProfit"), Number::New(pTradingAccount->PositionProfit));
		jsonRtn->Set(String::NewSymbol("Balance"), Number::New(pTradingAccount->Balance));
		jsonRtn->Set(String::NewSymbol("Available"), Number::New(pTradingAccount->Available));
		jsonRtn->Set(String::NewSymbol("WithdrawQuota"), Number::New(pTradingAccount->WithdrawQuota));
		jsonRtn->Set(String::NewSymbol("Reserve"), Number::New(pTradingAccount->Reserve));
		jsonRtn->Set(String::NewSymbol("TradingDay"), String::New(pTradingAccount->TradingDay));
		jsonRtn->Set(String::NewSymbol("SettlementID"), Int32::New(pTradingAccount->SettlementID));
		jsonRtn->Set(String::NewSymbol("Credit"), Number::New(pTradingAccount->Credit));
		jsonRtn->Set(String::NewSymbol("Mortgage"), Number::New(pTradingAccount->Mortgage));
		jsonRtn->Set(String::NewSymbol("ExchangeMargin"), Number::New(pTradingAccount->ExchangeMargin));
		jsonRtn->Set(String::NewSymbol("DeliveryMargin"), Number::New(pTradingAccount->DeliveryMargin));
		jsonRtn->Set(String::NewSymbol("ExchangeDeliveryMargin"), Number::New(pTradingAccount->ExchangeDeliveryMargin));
		jsonRtn->Set(String::NewSymbol("ReserveBalance"), Number::New(pTradingAccount->ReserveBalance));
		jsonRtn->Set(String::NewSymbol("CurrencyID"), String::New(pTradingAccount->CurrencyID));
		jsonRtn->Set(String::NewSymbol("PreFundMortgageIn"), Number::New(pTradingAccount->PreFundMortgageIn));
		jsonRtn->Set(String::NewSymbol("PreFundMortgageOut"), Number::New(pTradingAccount->PreFundMortgageOut));
		jsonRtn->Set(String::NewSymbol("FundMortgageIn"), Number::New(pTradingAccount->FundMortgageIn));
		jsonRtn->Set(String::NewSymbol("FundMortgageOut"), Number::New(pTradingAccount->FundMortgageOut));
		jsonRtn->Set(String::NewSymbol("FundMortgageAvailable"), Number::New(pTradingAccount->FundMortgageAvailable));
		jsonRtn->Set(String::NewSymbol("MortgageableFund"), Number::New(pTradingAccount->MortgageableFund));
		jsonRtn->Set(String::NewSymbol("SpecProductMargin"), Number::New(pTradingAccount->SpecProductMargin));
		jsonRtn->Set(String::NewSymbol("SpecProductFrozenMargin"), Number::New(pTradingAccount->SpecProductFrozenMargin));
		jsonRtn->Set(String::NewSymbol("SpecProductCommission"), Number::New(pTradingAccount->SpecProductCommission));
		jsonRtn->Set(String::NewSymbol("SpecProductFrozenCommission"), Number::New(pTradingAccount->SpecProductFrozenCommission));
		jsonRtn->Set(String::NewSymbol("SpecProductPositionProfit"), Number::New(pTradingAccount->SpecProductPositionProfit));
		jsonRtn->Set(String::NewSymbol("SpecProductCloseProfit"), Number::New(pTradingAccount->SpecProductCloseProfit));
		jsonRtn->Set(String::NewSymbol("SpecProductPositionProfitByAlg"), Number::New(pTradingAccount->SpecProductPositionProfitByAlg));
		jsonRtn->Set(String::NewSymbol("SpecProductExchangeMargin"), Number::New(pTradingAccount->SpecProductExchangeMargin));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined()->ToObject();
	}
	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapTrader::pkg_cb_rqinstrument(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
	CThostFtdcInstrumentField *pInstrument = static_cast<CThostFtdcInstrumentField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pInstrument) {
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pInstrument->InstrumentID));
		jsonRtn->Set(String::NewSymbol("ExchangeID"), String::New(pInstrument->ExchangeID));
		jsonRtn->Set(String::NewSymbol("InstrumentName"), String::New(pInstrument->InstrumentName));
		jsonRtn->Set(String::NewSymbol("ExchangeInstID"), String::New(pInstrument->ExchangeInstID));
		jsonRtn->Set(String::NewSymbol("ProductID"), String::New(pInstrument->ProductID));
		jsonRtn->Set(String::NewSymbol("ProductClass"), Int32::New(pInstrument->ProductClass));
		jsonRtn->Set(String::NewSymbol("DeliveryYear"), Int32::New(pInstrument->DeliveryYear));
		jsonRtn->Set(String::NewSymbol("DeliveryMonth"), Int32::New(pInstrument->DeliveryMonth));
		jsonRtn->Set(String::NewSymbol("MaxMarketOrderVolume"), Int32::New(pInstrument->MaxMarketOrderVolume));
		jsonRtn->Set(String::NewSymbol("MinMarketOrderVolume"), Int32::New(pInstrument->MinMarketOrderVolume));
		jsonRtn->Set(String::NewSymbol("MaxLimitOrderVolume"), Int32::New(pInstrument->MaxLimitOrderVolume));
		jsonRtn->Set(String::NewSymbol("MinLimitOrderVolume"), Int32::New(pInstrument->MinLimitOrderVolume));
		jsonRtn->Set(String::NewSymbol("VolumeMultiple"), Int32::New(pInstrument->VolumeMultiple));
		jsonRtn->Set(String::NewSymbol("PriceTick"), Number::New(pInstrument->PriceTick));
		jsonRtn->Set(String::NewSymbol("CreateDate"), String::New(pInstrument->CreateDate));
		jsonRtn->Set(String::NewSymbol("OpenDate"), String::New(pInstrument->OpenDate));
		jsonRtn->Set(String::NewSymbol("ExpireDate"), String::New(pInstrument->ExpireDate));
		jsonRtn->Set(String::NewSymbol("StartDelivDate"), String::New(pInstrument->StartDelivDate));
		jsonRtn->Set(String::NewSymbol("EndDelivDate"), String::New(pInstrument->EndDelivDate));
		jsonRtn->Set(String::NewSymbol("InstLifePhase"), Int32::New(pInstrument->InstLifePhase));
		jsonRtn->Set(String::NewSymbol("IsTrading"), Int32::New(pInstrument->IsTrading));
		jsonRtn->Set(String::NewSymbol("PositionType"), Int32::New(pInstrument->PositionType));
		jsonRtn->Set(String::NewSymbol("PositionDateType"), Int32::New(pInstrument->PositionDateType));
		jsonRtn->Set(String::NewSymbol("LongMarginRatio"), Number::New(pInstrument->LongMarginRatio));
		jsonRtn->Set(String::NewSymbol("ShortMarginRatio"), Number::New(pInstrument->ShortMarginRatio));
		jsonRtn->Set(String::NewSymbol("MaxMarginSideAlgorithm"), Int32::New(pInstrument->MaxMarginSideAlgorithm));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined()->ToObject();
	}
	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapTrader::pkg_cb_rqdepthmarketdata(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
	CThostFtdcDepthMarketDataField *pDepthMarketData = static_cast<CThostFtdcDepthMarketDataField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
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
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined()->ToObject();
	}
	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapTrader::pkg_cb_rqsettlementinfo(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
	CThostFtdcSettlementInfoField *pSettlementInfo = static_cast<CThostFtdcSettlementInfoField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pSettlementInfo) {
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("TradingDay"), String::New(pSettlementInfo->TradingDay));
		jsonRtn->Set(String::NewSymbol("SettlementID"), Int32::New(pSettlementInfo->SettlementID));
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pSettlementInfo->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(pSettlementInfo->InvestorID));
		jsonRtn->Set(String::NewSymbol("SequenceNo"), Int32::New(pSettlementInfo->SequenceNo));
		jsonRtn->Set(String::NewSymbol("Content"), String::New(pSettlementInfo->Content));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Undefined()->ToObject();
	}
	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapTrader::pkg_cb_rsperror(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	*(cbArray + 2) = pkg_rspinfo(pRspInfo);
	return;
}
Local<Value> WrapTrader::pkg_rspinfo(CThostFtdcRspInfoField *pRspInfo) {
	if (pRspInfo) {
		Local<Object> jsonInfo = Object::New();
		jsonInfo->Set(String::NewSymbol("ErrorID"), Int32::New(pRspInfo->ErrorID));
		jsonInfo->Set(String::NewSymbol("ErrorMsg"), String::New(pRspInfo->ErrorMsg));
		return jsonInfo;
	}
	else {
		return 	Undefined()->ToObject();
	}
}
