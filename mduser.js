var ctp = require('bindings')('shifctp');
ctp.settings({ log: true});
var mduser = ctp.createMdUser();
mduser.on("connect",function(result){
    console.log("on connected");
    mduser.reqUserLogin('0292', '0000001000', '122015',function(result){
        
    
    });

});

mduser.on("rspUserLogin", function (requestId, isLast, field, info) {
 
    mduser.subscribeMarketData(['IF1503'], function (result) {
             console.log('subscribeMarketData result:' + result);
    });
});

mduser.on('rspSubMarketData', function (requestId, isLast, field, info) {

    console.log("rspSubMarketData");
});

mduser.on('rspUnSubMarketData', function (requestId, isLast, field, info) {
    
    mduser.disconnect();

});

mduser.on('rtnDepthMarketData', function (field) {
    
    console.log(JSON.stringify(field));

});

mduser.on('rspError', function (requestId, isLast, info) {
    
    console.log("repError");

});

mduser.connect('tcp://222.240.130.30:41213', undefined, function (f) {

    console.log(f);
});
