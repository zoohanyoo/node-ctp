{
  "targets": [
    {
      "target_name": "shifctp",
      "sources": [ "src/shifctp.cc","src/tools.cc","src/stdafx.cpp","src/uv_mduser.cpp","src/uv_trader.cpp","src/wrap_mduser.cpp","src/wrap_trader.cpp" ],
      "libraries":["~/node-ctp/6.2.5_20140811_apitraderapi_linux64/thostmduserapi.so","~/node-ctp/6.2.5_20140811_apitraderapi_linux64/thosttraderapi.so"],
      "include_dirs":["6.2.5_20140811_apitraderapi_linux64/"]
    }
  ],
}


