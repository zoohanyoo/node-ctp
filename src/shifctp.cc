#include <node.h>
#include <v8.h>
#include "wrap_trader.h"
#include "wrap_mduser.h"

using namespace v8;

bool islog;//log?

Handle<Value> CreateTrader(const Arguments& args) {
  HandleScope scope;
  return scope.Close(WrapTrader::NewInstance(args));
}

Handle<Value> CreateMdUser(const Arguments& args) {
	HandleScope scope;
	return scope.Close(WrapMdUser::NewInstance(args));
}

Handle<Value> Settings(const Arguments& args) {
	HandleScope scope;

	if (!args[0]->IsUndefined() && args[0]->IsObject()) {
		Local<Object> setting = args[0]->ToObject();
		Local<Value> log = setting->Get(v8::String::New("log"));
		if (!log->IsUndefined()) {
			islog = log->BooleanValue();
		}		
	}

	return scope.Close(Undefined());
}

void Init(Handle<Object> exports) {
	WrapTrader::Init(0);
	WrapMdUser::Init(0);
	exports->Set(String::NewSymbol("createTrader"),
		FunctionTemplate::New(CreateTrader)->GetFunction());
	exports->Set(String::NewSymbol("createMdUser"),
		FunctionTemplate::New(CreateMdUser)->GetFunction());
	exports->Set(String::NewSymbol("settings"),
		FunctionTemplate::New(Settings)->GetFunction());
}

NODE_MODULE(shifctp, Init)
