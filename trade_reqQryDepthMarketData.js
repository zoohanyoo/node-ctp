ctp = require('bindings')('shifctp');
ctp.settings({ log: true});
var trader = ctp.createTrader();

trader.on("connect",function(result){
    console.log("on connected");
    trader.reqUserLogin('0292','0000001000','122015',function(result){
        console.log('login return val is '+result);
    });

});

trader.on('rspUserLogin',function(requestId, isLast, field, info){
    
    console.log(JSON.stringify(field));
    console.log(info);

    trader.reqQryDepthMarketData('IF1412',function(result){
        console.log('rqDdpthmarketData return val is '+result);

    });
});

trader.on('rqDdpthmarketData',function(requestId, isLast, field, info){
    console.log('rqDdpthmarketData callback');
    console.log(field);
    console.log(info);

});

trader.on('rtnOrder',function(field){
    console.log(field);
});

trader.on('rspError',function(requestId, isLast, field){
    console.log(JSON.stringify(field));

});

trader.connect('tcp://222.240.130.30:41205',undefined,0,1,function(result){
    console.log('connect return val is '+result);
});

console.log('continute');
