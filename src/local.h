#ifndef LOCAL_H
#define LOCAL_H

const char* EmonNode = "AtomS3";
const char* EmonApiKey = "abc123";
const char* EmonUriFormat = "http://emonpi/input/post?node=%s&apikey=%s&data={\"Temperature\":%3.2f,\"Humidity\":%3.2f,\"Pressure\":%3.2f}";

const char* WifiHostName = EmonNode;
const char* WifiSSID     = "ssid";
const char* WifiPassword = "password";

#endif