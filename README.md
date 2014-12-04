node-ctp
========

Wrap Fetures CTP Client API to Node.js

##Why?

Shif发布的CTP接口是基于C++语言开发的，我们使用CTP开发交易策略软件时，一般也使用C++语言。
我本人觉得这样不是很方便，我想能不能封装成Node.js模块，这样考虑基于以下两点：
    1. 交易策略脚本化，这样给用户提供了极大的方便，我也相信Javascript语言比C++好学。
    2. 自动交易需要一个高性能的并发多线程底层框架，原来都需要使用者自己开发。现在众所周知，
       Node.js使用了Libuv，好处我在这里不赘述。

##Demo

我们可以这样调用CTP接口

```javascript

var ctp = require('bindings')('shifctp');
ctp.settings({log:true});
var mduser = ctp.createMduser();
mduser.on("connect",function(result){
    console.log('on connect!');
    meuser.reqUserLogin('brokerid','userid','password',function(result){
        console.log(result);

    });

});

mduser.on('rspUserLogin',function(requestId,isLast,field,info){
    mduser.subscribeMarketData(['IF1503'],function(result){
        console.log('subscribeMarketData result:' + result);
    });

});

mduser.on('rspSubMarketData',function (requestId, isLast, field, info){
        

});

mduser.on('rspUnSubMarketData', function (requestId, isLast, field, info){
    mduser.disconnect();
});

mduser.on('rtnDepthMarketData', function (field){
    console.log(JSON.stringify(field));

});

mduser.on('rspError',function(requestId,isLast,info){
    //console.log(...);

});

mduser.connect('ctp url', undefined, function (result){
    console.log(result);

});


```
##运行环境

我分别给出了Linux和Windows平台下的工程，由于Shif发布接口文件平台的缘故，Linux版本的运行
在Node.js x64下，Windows运行在Node.js x86下。注意下载不同版本的Node.js,下载地址：[Download Node.js](http://www.nodejs.org/download/).
相应的Shif发布的Linux平台下CTP包选用x64的，而Windows平台的选用x86的。下载地址：[Download tradeapi](http://www.sfit.com.cn/5_2_DocumentDown.htm)

    linux:centos 6.4 x64;
    gcc:v4.4.7 20120313;
    node-gyp:v1.0.2;
    node.js:v0.10.26;
    tradeapi:6.2.5_20140811_apitraderapi_linux64

    windows:win7 x64;
    visual studio 2013;
    node-gyp:v1.0.2;
    node.js:v0.10.26 x86;
    tradeapi:6.2.5_20140811_traderapi_win32


##编译

    1.$npm install node-gyp;
    2.打开binding.gyp文件，把libraries节点的路径修改成自己开发环境的真实路径；
    3.$node-gyp rebuild;

    注意：windows环境下，node-gyp默认生成vs2010版本的工程，如果没有安装此版本的vs，需要设置环境变量
    如：GYP_MSVS_VERSION=2013

##介绍

待续







