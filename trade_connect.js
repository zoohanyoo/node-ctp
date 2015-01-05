ctp = require('bindings')('shifctp');
ctp.settings({ log: true});
var trader = ctp.createTrader();

trader.on("connect",function(result){
    console.log("on connected");
        
});

trader.connect('',undefined,0,1,function(result){
    console.log('connect return val is '+result);
});

console.log('continute');
