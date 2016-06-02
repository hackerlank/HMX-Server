#include "ProcDpHandler.h"
#include "CommonW2S.h"
#include "CommonS2S.h"
#include "LogManager.h"
#include "ServerSession.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "TemporaryScene.h"


ProcDpHandler::ProcDpHandler()
{
}


ProcDpHandler::~ProcDpHandler()
{
}

void ProcDpHandler::RevLoadUserData(BaseSession* pSession, const NetMsgHead* pMsg,int32 nSize)
{

	const D2SLoadCharacter* packet = static_cast<const D2SLoadCharacter*>(pMsg);

	TemporaryScene::Instance()->DbLoadData(packet->nSessionID,&(packet->sUserData));

}


