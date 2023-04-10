#ifndef LOCAL_H
#define LOCAL_h

const char* WifiHostName = "Temp - Room1";
const char* WifiSSID     = "irHere";
const char* WifiPassword = "IainLauraAndMatthew";
const char* EmonNode = "Room1";
const char* EmonUriFormat = string::format("http://localhost/input/post?node=%s&apikey=b215641cf41c8ed618172ec7911f566c&data=", EmonNode);
const char* EmonDataFormat = "{\"Temperature\":%3.2f,\"Humidity\":%3.2f;\"Pressure\":%3.2f;}";

#endif