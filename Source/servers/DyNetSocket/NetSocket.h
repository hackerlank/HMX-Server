#ifndef __NET_SOCKET_H_
#define __NET_SOCKET_H_


#include "NetIOBuffer.h"

/*
 *
 *	Detail: Socket 类 
 *
 *  Created by hzd 2013-1-1
 *
 */
enum ERecvType
{
	REVC_FSRS_NULL = 0,
	REVC_FSRS_HEAD,
	REVC_FSRS_BODY,
};
enum  EMsgRead
{
	MSG_READ_INVALID,
	MSG_READ_WAITTING,
	MSG_READ_OK,
	MSG_READ_REVCING,
};

// SCOKET 断开原因 
enum ESocketExist
{
	ESOCKET_EXIST_NULL = -1,			//-1:无 
	ESOCKET_EXIST_LOCAL = 0,			//0:本地主动断开-------服务端调用了SetWellColse 
	ESOCKET_EXIST_REMOTE,				//1:远端主动断开-------用户直接 
	ESOCKET_EXIST_TIMEOUT,				//2:超时断开-----------该值由启动服务器端设定 
	ESOCKET_EXIST_PACK_LENGTH_ERROR,	//3:包头长度值非法-----小于sizeof(NetMsgHead)或大于设置接收最大包大小 
	ESOCKET_EXIST_WRITE_TOO_DATA,		//4:写入数据到buffer中失败--收到的数据过多，而服务器处理包速度过慢，或者设定的buffer容量太小 
	ESCOKET_EXIST_SEND_CONNECT_INVAILD, //5:发送数据时失败------连接失效  
};

// Socket配置 
struct SocketDefine
{
private:

	int32 nMaxRecivedSize;/* 最大收到单包大小 */
	int32 nMaxSendoutSize;/* 最大一次发送大小(1+包) */
	int32 nMaxRecivedCacheSize;/* 最大接收缓存大小 */
	int32 nMaxSendoutCacheSize;/* 最大发送缓存大小 */
	int32 nBaseIncreseID;/* SocketID增长基数 */

private:

	SocketDefine()
	{
		memset(this,0,sizeof(SocketDefine));
		Define4x64();
	}

	/*
	单包收发 10k
	缓存收发 64k
	*/ 
	const SocketDefine& Define10x64()
	{
		nMaxRecivedSize = 10 * 1024;
		nMaxSendoutSize = 10 * 1024;
		nMaxRecivedCacheSize = 64 * 1024;
		nMaxSendoutCacheSize = 64 * 1024;
		return *this;
	}

	/*
	单包收发 10k
	缓存收发 64k
	*/
	const SocketDefine& Define4x64()
	{
		nMaxRecivedSize = 10 * 1024;
		nMaxSendoutSize = 10 * 1024;
		nMaxRecivedCacheSize = 64 * 1024;
		nMaxSendoutCacheSize = 64 * 1024;
		return *this;
	}

public:

	static SocketDefine g_sDefine;

};

// socket excute callback base  
struct SocketCallbackBase
{
public:
	SocketCallbackBase(int32 arg)
	{
		static int32 nIncraseID = 1000;
		nIncraseID++;
		m_nCallbackID = nIncraseID;
	}

	virtual void Finished(int32 nCallbackID)
	{

	}

	int32 GetCallbackID()
	{
		return m_nCallbackID;
	}

private:

	int32 m_nCallbackID;

};


enum 
{
	// 100-700+ 协议是即时协议(检查收在不停地进行) 
	EVENT_LOCAL_REVC_CLOSE = 100,
	EVENT_LOCAL_REVC_PRE_MSG = 200,
	EVENT_LOCAL_REVC_AFTER_MSG = 201,
	EVENT_LOCAL_REVC_PRE_ONLY_MSG = 300,
	EVENT_LOCAL_REVC_AFTER_ONLY_MSG = 301,

	// 远程协议NetServer内部的回调函数(只本地收到协议中处理) 
	EVENT_REMOTE_REVC_CLOSE = 500,
	EVENT_REMOTE_REVC_PRE_MSG = 600,
	EVENT_REMOTE_REVC_AFTER_MSG = 601,
	EVENT_REMOTE_REVC_PRE_ONLY_MSG = 700,
	EVENT_REMOTE_REVC_AFTER_ONLY_MSG = 701,

	//-----------------------发协议处理------------------------------
	EVENT_REMOTE_SEND_PRE_ONLY_MSG = 800,
	EVENT_REMOTE_SEND_AFTER_ONLY_MSE = 801,

};

const int32 PRO_REMOTE = 10;
struct stRemoteMsg : public NetMsgHead
{
	stRemoteMsg() :NetMsgHead(PRO_REMOTE)
	{

	}
	SocketEvent stEvent;
	inline int32 GetPackLength()const
	{
		return sizeof(*this);
	}
};



// 回调协议 
const int32 PRO_CALLBACK = 111;
struct stCallbackMsg : public NetMsgHead
{
	stCallbackMsg():NetMsgHead(PRO_CALLBACK)
	{
		nRepCallbackID = 0;
	}
	
	inline int32 GetPackLength()const
	{
		return sizeof(*this);
	}

	int32 nRepCallbackID;
};


class NetSocket : public tcp::socket
{
	friend class NetServer;
	friend class NetClient;
public:
	NetSocket(io_service& rIo_service,int32 nMaxRecivedSize,int32 nMaxSendoutSize,int32 nMaxRecivedCacheSize,int32 nMaxSendoutCacheSize);
	virtual ~NetSocket();

	/*
	 *
	 *	Detail:  获得SocketID，从0开始自增 
	 *
	 *  Created by hzd 2013/01/21
	 *
	 */
	int32 SID() { return m_nID; }

	/*
	 *
	 *	Detail:  压入要准备发送的数据 
	 *
	 *  Created by hzd 2013-1-21
	 *
	 */
	void ParkMsg(NetMsgHead* pMsg,int32 nLength);

	/*
	 *
	 *	Detail:  发送数据 
	 *
	 *  Created by hzd 2013-1-21
	 *
	 */
	void SendMsg();

	/*
	 *
	 *	Detail:  读取缓存中的数据 
	 *
	 *  Created by hzd 2013-1-21
	 *
	 */
	EMsgRead ReadMsg(NetMsgHead** pMsg,int32& nSize);

	/*
	 *
	 *	Detail: 移除缓存中的数据 
	 *
	 *  Created by hzd 2013-1-21
	 *
	 */
	void RemoveMsg(uint32 nLen);

	/*
	 *
	 *	Detail: 启动Socket，可以进入收发数据 
	 *
	 *  Created by hzd 2013-1-21
	 *
	 */
	void Run();

	/*
	 *
	 *	Detail: 获得Socket客户端IP地址 
	 *
	 *  Created by hzd 2013-1-21
	 *
	 */
	const string GetIp();

	/*
	 *
	 *	Detail: 将关闭Socket(主动断开客户端) 
	 *   
	 * Copyright (c) Created by hzd 2013-4-29.All rights reserved
	 *
	 */
	void OnEventColse();

	/*
	 *
	 *	Detail: 超过断开 
	 *   
	 *  Created by hzd 2013-6-2
	 *
	 */
	void SetTimeout(int32 nTimeout);

	/*
	 *
	 *	Detail: 定时启动 
	 *   
	 *  Created by hzd 2013-6-2
	 *
	 */
	void TimeoutStart();

	/*
	 *
	 *	Detail: 定时关闭 
	 *   
	 *  Created by hzd 2013-6-2
	 *
	 */
	void TimeoutCancel();

	/*-------------------------------------------------------------------
	 * @Brief :
	 *			-1:无
	 *			0:本地主动断开-------服务端调用了SetWellColse
	 *			1:远端主动断开-------用户直接
	 *			2:超时断开-----------该值由启动服务器端设定
	 *			3:包头长度值非法-----小于sizeof(NetMsgHead)或大于设置接收最大包大小
	 *			4:写入数据到buffer中失败--收到的数据过多，而服务器处理包速度过慢，或者设定的buffer容量太小 
	 *			5:发送数据时失败------连接失效 
	 *
	 * @Author:hzd 2013:11:19
	 *------------------------------------------------------------------*/
	int32 ErrorCode(std::string& error);

	// 自定义事件 
	void AddEvent(const SocketEvent& nEvent);

	void AddEventLocalClose();
	void AddEventLocalPreMsg();
	void AddEventLocalAfterMsg();
	void AddEventLocalPreOnlyMsg(int32 nProtocol);
	void AddEventLocalAfterOnlyMsg(int32 nProtocol);

	void AddEventRemotePreClose();
	void AddEventRemotePreMsg();
	void AddEventRemoteAfterMsg();
	void AddEventRemotePreOnlyMsg(int32 nProtocol);
	void AddEventRemoteAfterOnlyMsg(int32 nProtocol);

private:

	NetSocket();

	/*
	 *
	 *	Detail: 断开 socket
	 *
	 *  Created by hzd 2013-1-21
	 *
	 */
	void Disconnect();

	/*
	 *
	 *	Detail: 关闭 socket
	 *
	 *  Created by hzd 2013-1-21
	 *
	 */
	void HandleClose(const boost::system::error_code& error);
	
	/*
	 *
	 *	Detail: 恢复到构造函数那种状态 
	 *
	 *  Created by hzd 2013-1-20
	 *
	 */
	void Clear();

	/*
	 *
	 *	Detail: 收到指定长度数据回调函数 
	 *
	 *  Created by hzd 2013-1-21
	 *
	 */
	void RecvMsg(const boost::system::error_code& ec, size_t bytes_transferred);

	/*
	 *
	 *	Detail: 发送数据回调函数，发送后发现有新数据，则会选择断续发送（一般不会发生这种情况）
	 *
	 *  Created by hzd 2013-1-21
	 *
	 */
	void SendMsg(const boost::system::error_code& ec, size_t bytes_transferred);

	/*
	 *
	 *	Detail: 读消息头 
	 *
	 *  Created by hzd 2013-1-21
	 *
	 */
	void ReadHead();

	/*
	 *
	 *	Detail: 读消息主体 
	 *
	 *  Created by hzd 2013-1-21
	 *
	 */
	void ReadBody();

	/*
	 *
	 *	Detail: 超时回调函数，将会关闭 socket 
	 *
	 *  Created by hzd 2013-1-21
	 *
	 */
	void HandleWait(const boost::system::error_code& error);

private:

	/*
	 * 获得事件(获得一次后，记录事件会被清掉)	
	 */
	bool GetEvents(std::vector<SocketEvent>& o_vecEvents);

	void RemoveEvents(const SocketEvent& stEvent);

	// 当一个用户退出时，应该将它的所有事件删除，这个由逻辑去控制，不该提供这样的接口 
	void RemoveEventsBySessionID(int32 nSessionID) {}

private:

	int32				m_nID;				// socketID， 一个进程所的 Socket 唯一ID从0开始 
	int32				m_nIndexID;			// 自定义ID, 可以用数组下标，方便管理 

	deadline_timer		m_cTimer;			// 收发定时器，用于检测用户多久没有与服务器通信，否则会角色 HandleWait 函数 
	deadline_timer		m_cCloseTimer;		// 关闭连接定时器 

	NetIOBuffer			m_cIBuffer;			// 管理收到的字节管理器  
	NetIOBuffer			m_cOBuffer;			// 管理发出的字节管理器 

	mutable_buffers_1	m_cHeadBuffer;		// 收到头数据缓存绑定类 
	mutable_buffers_1	m_cBodyBuffer;      // 收到的主体数据绑在绑定类 
	mutable_buffers_1	m_cSendBuffer;      // 发送的数据缓存绑定类 

	ERecvType			m_eRecvStage;
	int32				m_nBodyLen;			// 主体数据长度（不包括包头） 
	char				m_arrRecvBuffer[65536];

	int32				m_bSending;
	char				m_arrSendBuffer[65536];

	int32				m_nMaxRecivedSize;
	int32				m_nMaxSendoutSize;

	int32				m_nTimeout;			// 超时断开,0不,>0指定秒数 

	/*-------------------------------------------------------------------
	 * @Brief :
	 *			-1:无
	 *			0:本地主动断开-------服务端调用了SetWellColse
	 *			1:远端主动断开-------用户直接
	 *			2:超时断开-----------该值由启动服务器端设定
	 *			3:包头长度值非法-----小于sizeof(NetMsgHead)或大于设置接收最大包大小
	 *			4:写入数据到buffer中失败--收到的数据过多，而服务器处理包速度过慢，或者设定的buffer容量太小 
	 *			5:发送数据时失败------连接失效 
	 *
	 * @Author:hzd 2013:11:19
	 *------------------------------------------------------------------*/
	int32				m_errorCode;		// 错误代码  

	std::map<int32,SocketCallbackBase*> m_mapCallback; // 自动完成后回调

	/*-------------------------------------------------------------------
	* @Brief :
	*			本地协议
	*			100:断开连接前回调退出接口
	*			200:业务逻辑前事件回调(即时1次)
	*			201:业务逻辑后事件回调(即时1次)
	*			300:协议监听xxx前回调(即时1次)
	*			301:协议监听xxx后回调(即时1次)
	*			
	*			远程协议
	*			500:断开连接前回调远程接口
	*			600:业务逻辑前事件回调远程接口(即时1次)
	*			601:业务逻辑后事件回调远程接口(即时1次)
	*			700:协议监听xxx前回调远程接口(即时1次)
	*			701:协议监听xxx后回调远程接口(即时1次)
	* @Author:hzd 2013:11:19
	*------------------------------------------------------------------*/
	std::vector< SocketEvent >	m_vecEvents;	// 普通事件 

};

typedef boost::function<void(NetSocket& pSocket)>	PNetServerEnterHandler;
typedef boost::function<void(NetSocket& pSocket)>	PNetServerExistHandler;
typedef boost::function<void(NetSocket& pSocket, NetMsgHead* pMsg,int32 nSize)>	PNetServerOnMsgHandler;


typedef list<NetSocket*> SocketList;
typedef list<NetSocket*>::iterator SocketListIter;
typedef vector<NetSocket*> SocketVector;
typedef vector<NetSocket*>::iterator SocketVectorIter;


// 事件回调接口定义 
typedef boost::function<void(NetSocket& pSocket, const SocketEvent& stEvent)> PNetEventLocalPreMsg;
typedef boost::function<void(NetSocket& pSocket, const SocketEvent& stEvent)> PNetEventLocalAfterMsg;
typedef boost::function<void(NetSocket& pSocket, const SocketEvent& stEvent)> PNetEventLocalPreOnlyMsg;
typedef boost::function<void(NetSocket& pSocket, const SocketEvent& stEvent)> PNetEventLocalAfterOnlyMsg;

// 本地远程回调 
typedef boost::function<void(NetSocket& pSocket, const SocketEvent& stEvent)> PNetEventRemoteClose;
typedef boost::function<void(NetSocket& pSocket, const SocketEvent& stEvent)> PNetEventRemotePreMsg;
typedef boost::function<void(NetSocket& pSocket, const SocketEvent& stEvent)> PNetEventRemoteAfterMsg;
typedef boost::function<void(NetSocket& pSocket, const SocketEvent& stEvent)> PNetEventRemotePreOnlyMsg;
typedef boost::function<void(NetSocket& pSocket, const SocketEvent& stEvent)> PNetEventRemoteAfterOnlyMsg;

#endif

