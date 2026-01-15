///* -*- mode: c; tab-width: 4; c-basic-offset: 4; c-file-style: "linux" -*- */
//
// Copyright (c) 2009-2011, Wei Mingzhi <whistler_wmz@users.sf.net>.
// Copyright (c) 2011-2021, SDLPAL development team.
// Copyright (c) 2021-2026, Wu Dong.
// 
// All rights reserved.
//
// This file is part of SDLPAL.
//
// SDLPAL is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <cassert>
//#include <iostream>
#include <filesystem>
#include "cpaldata.h"
#include "cpalevent.h"
#include "Convers.h"
#include <fstream>
typedef struct tagSAVEDGAME_DOS
{
	WORD             wSavedTimes;             // saved times
	WORD             wViewportX, wViewportY;  // viewport location
	WORD             nPartyMember;            // number of members in party
	WORD             wNumScene;               // scene number
	WORD             wPaletteOffset;
	WORD             wPartyDirection;         // party direction
	WORD             wNumMusic;               // music number
	WORD             wNumBattleMusic;         // battle music number
	WORD             wNumBattleField;         // battle field number
	WORD             wScreenWave;             // level of screen waving
	WORD             wBattleSpeed;            // battle speed
	WORD             wCollectValue;           // value of "collected" items
	WORD             wLayer;
	WORD             wChaseRange;
	WORD             wChasespeedChangeCycles;
	WORD             nFollower{};

	WORD	    	 bFinishGameTime{};         //未使用地址，用来记录共完成游戏多少次
	WORD			 rgbReserved[2]{};			// unused

	DWORD            dwCash;                  // amount of cash
	PARTY            rgParty[MAX_PLAYABLE_PLAYER_ROLES];       // player party
	TRAIL            rgTrail[MAX_PLAYABLE_PLAYER_ROLES];       // player trail
	ALLEXPERIENCE    Exp;                       // experience data
	PLAYERROLES      PlayerRoles;
	BYTE			 rgbReserved2[320]{};		// poison status 
	INVENTORY        rgInventory[MAX_INVENTORY];// inventory status
	SCENE            rgScene[MAX_SCENES];
	OBJECT_DOS       rgObject[MAX_OBJECTS];
	EVENTOBJECT      rgEventObject[MAX_EVENT_OBJECTS];
	static const size_t getOffset(int ob, int isWin95 = 0)
	{
		switch (ob)
		{
		case 0://Object
			return sizeof(tagSAVEDGAME_DOS) - sizeof(rgEventObject) - sizeof(rgObject);
		case 1://EventObject
			if (isWin95)
				return sizeof(tagSAVEDGAME_DOS) - sizeof(rgEventObject) + MAX_OBJECTS * sizeof(WORD);
			else
				return sizeof(tagSAVEDGAME_DOS) - sizeof(rgEventObject);
		case 2://Scene
			return sizeof(tagSAVEDGAME_DOS) - sizeof(rgEventObject) - sizeof(rgObject) - sizeof(rgScene);
		default://Error
			return 0;
		}
	};
} SAVEDGAME_DOS, * LPSAVEDGAME_DOS;

typedef struct tagSAVEDGAME
{
	WORD             wSavedTimes;             // saved times
	WORD             wViewportX, wViewportY;  // viewport location
	WORD             nPartyMember;            // number of members in party
	WORD             wNumScene;               // scene number
	WORD             wPaletteOffset;
	WORD             wPartyDirection;         // party direction
	WORD             wNumMusic;               // music number
	WORD             wNumBattleMusic;         // battle music number
	WORD             wNumBattleField;         // battle field number
	WORD             wScreenWave;             // level of screen waving
	WORD             wBattleSpeed;            // battle speed
	WORD             wCollectValue;           // value of "collected" items
	WORD             wLayer;
	WORD             wChaseRange;
	WORD             wChasespeedChangeCycles;
	WORD             nFollower;

	WORD			 bFinishGameTime{};         //未使用地址，用来记录共完成游戏多少次
	WORD			 rgbReserved[2]{};			// unused

	DWORD            dwCash;                  // amount of cash
	PARTY            rgParty[MAX_PLAYABLE_PLAYER_ROLES];       // player party
	TRAIL            rgTrail[MAX_PLAYABLE_PLAYER_ROLES];       // player trail
	ALLEXPERIENCE    Exp;                     // experience data
	PLAYERROLES      PlayerRoles;
	BYTE				rgbReserved2[320];		// poison status // 现在已经弃用
	INVENTORY        rgInventory[MAX_INVENTORY];               // inventory status
	SCENE            rgScene[MAX_SCENES];
	OBJECT           rgObject[MAX_OBJECTS];
	EVENTOBJECT      rgEventObject[MAX_EVENT_OBJECTS];
} SAVEDGAME, * LPSAVEDGAME;

typedef struct tagSAVEDGAME_COMMON
{
	WORD             wSavedTimes;             // saved times
	WORD             wViewportX, wViewportY;  // viewport location
	WORD             nPartyMember;            // number of members in party
	WORD             wNumScene;               // scene number
	WORD             wPaletteOffset;
	WORD             wPartyDirection;         // party direction
	WORD             wNumMusic;               // music number
	WORD             wNumBattleMusic;         // battle music number
	WORD             wNumBattleField;         // battle field number
	WORD             wScreenWave;             // level of screen waving
	WORD             wBattleSpeed;            // battle speed
	WORD             wCollectValue;           // value of "collected" items
	WORD             wLayer;
	WORD             wChaseRange;
	WORD             wChasespeedChangeCycles;
	WORD             nFollower;

	WORD				bFinishGameTime{};         //未使用地址，用来记录共完成游戏多少次
	WORD				rgbReserved[2]{};			// unused

	DWORD            dwCash;                  // amount of cash

	PARTY            rgParty[MAX_PLAYABLE_PLAYER_ROLES];       // player party
	TRAIL            rgTrail[MAX_PLAYABLE_PLAYER_ROLES];       // player trail
	ALLEXPERIENCE    Exp;                     // experience data
	PLAYERROLES      PlayerRoles;
	BYTE			 rgbReserved2[320]{};		// poison status // 现在已经弃用
	INVENTORY        rgInventory[MAX_INVENTORY];               // inventory status
	SCENE            rgScene[MAX_SCENES];
} SAVEDGAME_COMMON, * LPSAVEDGAME_COMMON;

#ifndef DO_BYTESWAP
#define DO_BYTESWAP(buf, size)									\
if(DO_SWAP)                                                     \
for (int i = 0; i < (size) / 2; i++)							\
{																\
	((LPWORD)(buf))[i] = SWAP16(((LPWORD)(buf))[i]);			\
}
#endif // !DO_BYTESWAP


ByteArray CPalData::readAll(const std::string& fileName) {
	std::ifstream file(fileName, std::ios::binary);
	if (!file.is_open())
		return ByteArray();
	file.seekg(0, std::ios::end);
	std::streampos fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	ByteArray fileData(fileSize + (std::streampos)4);
	file.read(reinterpret_cast<char*>(fileData.data()), fileSize);
	file.close();
	return fileData;
}



/*
ByteArray CPalData::readAll(const std::string& fileName) {
	FILE* fd = fopen(fileName.c_str(), "rb");
	if (!fd) {
		//perror("open失败");
		return ByteArray();
	}

	// 获取文件大小
	fseek(fd, 0, SEEK_END);
	off_t fileSize = ftell(fd);
	if (fileSize == (off_t)-1) {
		perror("lseek失败");
		fclose(fd);
		return ByteArray();
	}
	fseek(fd, 0, SEEK_SET);  // 回到文件开头
	ByteArray fileData(fileSize);
	fread(fileData.data(),fileSize,1,fd);
	fclose(fd);
	return fileData;
}

*/
CPalData::CPalData(const std::string& dir,const CPalData* pf) {
	loadAllData(dir, pf);
}

CPalData::~CPalData() {
	PAL_FreeUI();
	PAL_FreeText();
	PAL_FreeGlobals();
}

PalErr CPalData::loadAllData(const std::string& dir, const CPalData* pf)
{
	auto& gConfig = CPalEvent::ggConfig;
	//复制系统目录
	PalDir = dir;
	int err{};
	if (pf)
		f = pf->f;//拷贝不进行物理读取数据，复制
	else
	{
		err += loadData(dir + "abc.mkf", f.fpABC);
		err += loadData(dir + "map.mkf", f.fpMAP);
		err += loadData(dir + "gop.mkf", f.fpGOP);

		err += loadData(dir + "fbp.mkf", f.fpFBP);
		err += loadData(dir + "mgo.mkf", f.fpMGO);
		err += loadData(dir + "ball.mkf", f.fpBALL);
		err += loadData(dir + "data.mkf", f.fpDATA);
		err += loadData(dir + "f.mkf", f.fpF);
		err += loadData(dir + "fire.mkf", f.fpFIRE);
		err += loadData(dir + "rgm.mkf", f.fpRGM);
		err += loadData(dir + "sss.mkf", f.fpSSS);
		err += loadData(dir + "pat.mkf", f.fpPAT);
		err += loadData(dir + "rng.mkf", f.fpRNG);
		err += loadData(dir + "m.msg", f.fpMsg);
		err += loadData(dir + "word.dat", f.fpWord);
		if (IsFileExist(dir + "desc.dat"))
			loadData(dir + "desc.dat", f.fpDesc);
	}
	err += loadConfig();
	err = (CConfig::fisUSEYJ1DeCompress = is_Use_YJ1_Decompress()) == -1;

	CPalEvent::ggConfig->fIsWIN95 = PAL_IsPalWIN();
	PAL_InitGlobalGameData();
	if (err)
	{
		return PalErr(err);
	}
	g.rgObject.resize( MAX_OBJECTS);
	DWORD len = PAL_MKFGetChunkSize(2, f.fpSSS);
	if (gConfig->fIsWIN95)
		g.nObject = len / sizeof(OBJECT);
	else
		g.nObject = len / sizeof(OBJECT_DOS);
	PAL_InitText();
	PAL_InitUI();
	PAL_LoadDefaultGame();
	bIsBig5 = PAL_IsBig5(gConfig->m_Function_Set[49]) == 1;
	//装入说明，如没有则为空
	lpObjectDesc = PAL_LoadObjectDesc();
	//存档文件缓存
	//预装入存档文件
	int maxSaveFile = gConfig->m_Function_Set[47] + 
		CPalEvent::ggConfig->m_Function_Set[53];
	rgSaveData.resize(maxSaveFile);
	if (pf)
		for (int i = 0; i < maxSaveFile; i++)
			rgSaveData.at(i) = pf->rgSaveData.at(i);
	else
		for (size_t i = 0; i < maxSaveFile; i++)
		{
			std::string fileName = CPalEvent::PalDir +
				CPalEvent::va("%d.rpg", i + 1 - 
					CPalEvent::ggConfig->m_Function_Set[53]);
			if (IsFileExist(fileName))
			{
				rgSaveData.at(i) = readAll(fileName);
			}
		}
	return PalErr(err);
}

PalErr CPalData::loadData(std::string filename, ByteArray& s)
{
	s = readAll(filename);
	if (s.empty())
	{
		return 1;
	}
	return 0;
}

int CPalData::PAL_MKFGetChunkCount(const ByteArray& fp)
{
	if (fp.empty())
	{
		return 0;
	}
	return pByteToInt(fp.data()) / 4 - 1;
}

INT CPalData::PAL_MKFGetChunkSize(UINT uiChunkNum, const ByteArray& fp) 
{
	UINT    uiOffset = 0;
	UINT    uiNextOffset = 0;
	UINT    uiChunkCount = 0;
	//
	// Get the total number of chunks.
	//
	uiChunkCount = PAL_MKFGetChunkCount(fp);
	if (uiChunkNum >= uiChunkCount)
	{
		return -1;
	}
	uiNextOffset = pByteToInt(fp.data() + 4 * uiChunkNum + 4);
	uiOffset = pByteToInt(fp.data() + 4 * uiChunkNum);
	return uiNextOffset - uiOffset;
}

INT CPalData::PAL_IsBig5(INT Default_Decision)
{
	/*
	* 功能：测试字符集是否是BIG5
	* 输入缺省定义 1 返回0 2 返回 1，其他测试
	* 返回：1 是，0 GB2312 //其他 -1;
	*/
	if (Default_Decision == 1)return 0;
	if (Default_Decision == 2)return 1;
	int noGb2312 = 0, noBig5 = 0;
	LPCSTR fname[2] = { "word.dat","m.msg" };
	for (size_t m = 0; m < 2; m++)
	{
		auto buf = CPalData::readAll(CPalEvent::PalDir + fname[m]);
		long i = buf.size();
		//buf.resize(i + 10);//防止数据溢出
		for (size_t n = 0; n < i; n++)
		{
			BYTE c, c1;
			c = buf[n];
			c1 = n + 1 < i ? buf[n + 1] : 0;//防止数据溢出
			if (c < 127)
				continue;//ascii
			if (c > 0xa0 && c <= 0xfe)
			{
				if (c1 >= 0x40 && c1 <= 0xfe)
				{
					if (c1 < 0xa1)
					{
						//gb2312编码范围之外
						noGb2312++;
					}
					//big5范围
					n++;
					continue;
				}
			}
			noBig5++;
		}
	}
	//end
	if (noBig5 > 4)
		return -1;
	if (noGb2312 <= 10)
		return 0;
	return  1;
}

PalErr CPalData::PAL_InitText()
{

	//
	// See how many words we have
	//
	auto i = f.fpWord.size();
	//
	// Each word has 10 bytes
	//
	g_TextLib.nWords = (i + (WORD_LENGTH - 1)) / WORD_LENGTH;

	//
	// Read the words
	//
	g_TextLib.lpWordBuf = f.fpWord.data();

	//
	// Read the message offsets. The message offsets are in SSS.MKF #3
	//
	i = PAL_MKFGetChunkSize(3, f.fpSSS) / sizeof(DWORD);
	g_TextLib.nMsgs = i - 1;

	g_TextLib.lpMsgOffset.resize(i);
	PAL_MKFReadChunk((LPBYTE)(g_TextLib.lpMsgOffset.data()), i * sizeof(DWORD), 3,
		f.fpSSS);

	//
	// Read the messages.
	//
	//i = f.fpMsg.size();

	g_TextLib.lpMsgBuf = f.fpMsg.data();

	g_TextLib.bCurrentFontColor = FONT_COLOR_DEFAULT;
	g_TextLib.bIcon = 0;
	g_TextLib.posIcon = 0;
	g_TextLib.nCurrentDialogLine = 0;
	g_TextLib.iDelayTime = 3;
	g_TextLib.posDialogTitle = PAL_XY(12, 8);
	g_TextLib.posDialogText = PAL_XY(44, 26);
	g_TextLib.bDialogPosition = kDialogUpper;
	g_TextLib.fUserSkip = FALSE;

	PAL_MKFReadChunk(g_TextLib.bufDialogIcons, 282, 12, f.fpDATA);

	return 0;
}

PalErr CPalData::PAL_InitUI()
{
	int        iSize;

	//
	// Load the UI sprite.
	//
	iSize = PAL_MKFGetChunkSize(CHUNKNUM_SPRITEUI, f.fpDATA);
	if (iSize < 0)
	{
		return 1;
	}

	gpSpriteUI = (LPSPRITE)calloc(1, iSize);
	if (gpSpriteUI == NULL)
	{
		return -1;
	}

	PAL_MKFReadChunk(gpSpriteUI, iSize, CHUNKNUM_SPRITEUI, f.fpDATA);

	return 0;
}

VOID CPalData::PAL_FreeGlobals()
{
	PAL_FreeObjectDesc(lpObjectDesc);
}

VOID CPalData::PAL_FreeUI()
{

	if (gpSpriteUI != NULL)
	{
		free(gpSpriteUI);
		gpSpriteUI = NULL;
	}
}

VOID CPalData::PAL_FreeText()
{
}

//释放说明
VOID CPalData::PAL_FreeObjectDesc(LPOBJECTDESC lpObjectDesc)
{
	if (lpObjectDesc == NULL)
		return;
	delete (lpObjectDesc);
	lpObjectDesc = nullptr;
}

LPOBJECTDESC CPalData::PAL_LoadObjectDesc()
/*++
Purpose:

Load the object description strings from file.

Parameters:

[IN]  lpszFileName - the filename to be loaded.

Return value:

Pointer to loaded data, in linked list form. NULL if unable to load.

--*/
{

	auto& fp = f.fpDesc;
	if (fp.empty())
	{
		return NULL;
	}

	//
	// Load the description data

	auto lpDesc = new OBJECTDESC;
	size_t pos{};
	char buf[512];
	while (bgets(buf, 510, fp, pos))
	{
		auto p = strchr(buf, '=');
		if (p == nullptr)
		{
			continue;
		}
		*p = '\0';
		p++;
		int i{};
		sscanf_s(buf, "%x", &i);
		lpDesc->p[i] = PAL_TextToUTF8(p);
	}

	return lpDesc;
}


// 判断文件是否存在
BOOL CPalData::IsFileExist(const std::string& filename)
{
	return std::filesystem::exists(filename);
}

// 判断文件是否存在
BOOL CPalData::IsDirExist(const std::string& path)
{
    return std::filesystem::exists(path) && 
		std::filesystem::is_directory(path);
}

int CPalData::PAL_MKFReadChunk(LPBYTE lpBuffer, UINT uiBufferSize, UINT uiChunkNum, const ByteArray& fp)
{
	UINT     uiOffset = 0;
	UINT     uiNextOffset = 0;
	UINT     uiChunkCount = 0;
	UINT     uiChunkLen = 0;

	if (lpBuffer == NULL || fp.empty() || uiBufferSize == 0)
	{
		return -1;
	}

	//
	// Get the total number of chunks.
	//
	uiChunkCount = PAL_MKFGetChunkCount(fp);
	if (uiChunkNum >= uiChunkCount)
	{
		return -1;
	}

	//
	// Get the offset of the chunk.
	//
	uiOffset = pByteToInt(fp.data() + 4 * uiChunkNum);
	uiNextOffset = pByteToInt(fp.data() + 4 * uiChunkNum + 4);
	uiOffset = SWAP32(uiOffset);
	uiNextOffset = SWAP32(uiNextOffset);

	//
	// Get the length of the chunk.
	//
	uiChunkLen = uiNextOffset - uiOffset;

	if (uiChunkLen > uiBufferSize)
	{
		return -2;
	}

	if (uiChunkLen != 0)
	{
		memcpy(lpBuffer, fp.data() + uiOffset, uiChunkLen);
	}
	else
	{
		return -1;
	}

	return (INT)uiChunkLen;
}

ByteArray CPalData::PAL_MKFReadChunk(UINT uiChunkNum, const ByteArray& fp)
{
	if (fp.empty())
		return ByteArray();
	auto uiChunkCount = PAL_MKFGetChunkCount(fp);
	auto size = PAL_MKFGetChunkSize(uiChunkNum, fp);
	if (size <= 0 || uiChunkCount <= uiChunkNum)
		return ByteArray();
	ByteArray buf;
	buf.resize(size);
	auto uiOffset = pByteToInt(fp.data() + 4 * uiChunkNum);
	memcpy(buf.data(), fp.data() + uiOffset, size);
	return buf;
}

PalErr CPalData::PAL_LoadGame(LPCSTR szFileName)
{
	ByteArray fi = readAll(szFileName);
	return PAL_LoadGame(fi);
}

PalErr CPalData::PAL_LoadGame(const ByteArray & fi)
{
	auto &gConfig = CPalEvent::ggConfig;
	if (fi.empty())
		return 1;
	auto s =(const SAVEDGAME_COMMON *)fi.data();
	
	viewport = PAL_XY(s->wViewportX, s->wViewportY);
	wMaxPartyMemberIndex = s->nPartyMember;
	wNumScene = s->wNumScene;
	fNightPalette = (s->wPaletteOffset != 0);
	wPartyDirection = s->wPartyDirection;
	wNumMusic = s->wNumMusic;
	wNumBattleMusic = s->wNumBattleMusic;
	wNumBattleField = s->wNumBattleField;
	wScreenWave = s->wScreenWave;
	sWaveProgression = 0;
	wCollectValue = s->wCollectValue;
	wLayer = s->wLayer;
	wChaseRange = s->wChaseRange;
	wChasespeedChangeCycles = s->wChasespeedChangeCycles;
	nFollower = s->nFollower;
	dwCash = s->dwCash;
	if (!gConfig->fIsWIN95)
		bFinishGameTime = s->bFinishGameTime;
	else
		bFinishGameTime = 0;
	memcpy(rgParty, s->rgParty, sizeof(rgParty));
	
	memcpy(rgTrail, s->rgTrail, sizeof(rgTrail));

	Exp = s->Exp;
	g.PlayerRoles = s->PlayerRoles;
	memset(rgPoisonStatus, 0, sizeof(rgPoisonStatus));
	memcpy(rgInventory, s->rgInventory, sizeof(rgInventory));
	memcpy(g.rgScene.data(), s->rgScene, sizeof(s->rgScene));

	fEnteringScene = FALSE;

	PAL_CompressInventory();
	if (gConfig->fIsWIN95)
	{
		auto p = (const SAVEDGAME*)s;
		memcpy(g.rgObject.data(), p->rgObject, sizeof(p->rgObject));
		memcpy(g.lprgEventObject.data(), p->rgEventObject, sizeof(EVENTOBJECT) * g.nEventObject);
	}
	else
	{
		auto p = (const SAVEDGAME_DOS*)s;
		LPOBJECT op = (LPOBJECT)g.rgObject.data();
		for (int i = 0; i < MAX_OBJECTS; i++)
		{
			memcpy(&op[i], &p->rgObject[i], sizeof(OBJECT_DOS));
			op[i].rgwData[6] = p->rgObject[i].rgwData[5];     // wFlags
			op[i].rgwData[5] = 0;                             // wScriptDesc or wReserved2
		}
		memcpy(g.lprgEventObject.data(), p->rgEventObject, sizeof(EVENTOBJECT) * g.nEventObject);
	}
	bSavedTimes = s->wSavedTimes;
	return 0;
}

VOID CPalData::PAL_SaveGame(int iSlot, WORD wSavedTimes,bool noSave)
{
	bSavedTimes = wSavedTimes;

	ByteArray& fb = rgSaveData.at(iSlot - 1);
	if (iSlot == 1 && CPalEvent::ggConfig->m_Function_Set[53])
		wSavedTimes = 1;
	PAL_SaveGame(fb, wSavedTimes);
	rgSaveDataChaged[iSlot - 1] = 1;//标识改变
	//如果不请允许存盘，返回
	if (noSave) return;
	
	//物理存盘
	auto s = (SAVEDGAME_COMMON*)fb.data();
	//
	// Adjust endianness
	//
	DO_BYTESWAP(s, fb.size());

	//
	// Cash amount is in DWORD, so do a wordswap in Big-Endian.
	//
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	s->dwCash = ((s->dwCash >> 16) | (s->dwCash << 16));
#endif

	FILE* fp{};
	std::string szFileName = CPalEvent::PalDir + 
		CPalEvent::va("%d.rpg", iSlot - CPalEvent::ggConfig->m_Function_Set[53]);
	if (fopen_s(&fp, szFileName.c_str(), "wb"))
		return;
	auto i = getSaveFileLen();
	fwrite(s, i, 1, fp);

	fclose(fp);
}

VOID CPalData::PAL_SaveGame(ByteArray& fp, WORD wSavedTimes)
{
	fp.resize(sizeof(SAVEDGAME));
	auto s = (SAVEDGAME_COMMON*)fp.data();
	if (CPalEvent::ggConfig->fIsWIN95)
	{
		SAVEDGAME* p = (SAVEDGAME*)s;
		memcpy(&p->rgObject, g.rgObject.data(), sizeof(p->rgObject));
		memcpy(&p->rgEventObject, g.lprgEventObject.data(), sizeof(EVENTOBJECT) * g.nEventObject);
	}
	else
	{
		SAVEDGAME_DOS* p = (SAVEDGAME_DOS*)s;
		LPOBJECT op = (LPOBJECT)g.rgObject.data();
		for (int i = 0; i < MAX_OBJECTS; i++)
		{
			memcpy(&p->rgObject[i], &op[i], sizeof(OBJECT_DOS));
			p->rgObject[i].rgwData[5] = op[i].rgwData[6];     // wFlags
		}
		memcpy(p->rgEventObject, g.lprgEventObject.data(), sizeof(EVENTOBJECT) * g.nEventObject);
	}
	s->wSavedTimes = wSavedTimes;
	s->wViewportX = PAL_X(viewport);
	s->wViewportY = PAL_Y(viewport);
	s->nPartyMember = wMaxPartyMemberIndex;
	s->wNumScene = wNumScene;
	s->wPaletteOffset = (fNightPalette ? 0x180 : 0);
	s->wPartyDirection = wPartyDirection;
	s->wNumMusic = wNumMusic;
	s->wNumBattleMusic = wNumBattleMusic;
	s->wNumBattleField = wNumBattleField;
	s->wScreenWave = wScreenWave;
	s->wCollectValue = wCollectValue;
	s->wLayer = wLayer;
	s->wChaseRange = wChaseRange;
	s->wChasespeedChangeCycles = wChasespeedChangeCycles;
	s->nFollower = nFollower;
	s->dwCash = dwCash;

#ifdef FINISH_GAME_MORE_ONE_TIME
	s->bFinishGameTime = bFinishGameTime;
#endif

#ifndef PAL_CLASSIC
	s->wBattleSpeed = bBattleSpeed;
#else
	s->wBattleSpeed = 2;
#endif

	memcpy(s->rgParty, rgParty, sizeof(rgParty));
	memcpy(s->rgTrail, rgTrail, sizeof(rgTrail));
	s->Exp = Exp;
	s->PlayerRoles = g.PlayerRoles;
	//memcpy(s->rgPoisonStatus, rgPoisonStatus, sizeof(rgPoisonStatus));
	memcpy(s->rgInventory, rgInventory, sizeof(rgInventory));
	memcpy(s->rgScene, g.rgScene.data(), sizeof(s->rgScene));
}

VOID CPalData::PAL_LoadDefaultGame()
/*++
Purpose:
Load the default game data.
Parameters:
None.
Return value:
None.
--*/
{

	//
	// Load the default data from the game data files.
	//
	Pal_LoadObject(g.lprgEventObject, g.nEventObject,0,f.fpSSS);
	Pal_LoadObject(g.rgScene, g.nScene,1,f.fpSSS);
	assert( g.nScene <= MAX_SCENES);
	if (CPalEvent::ggConfig->fIsWIN95)
	{
		PAL_MKFReadChunk((LPBYTE)(g.rgObject.data()), sizeof(OBJECT) * MAX_OBJECTS, 2, f.fpSSS);
	}
	else
	{
		std::vector<OBJECT_DOS> objects;
		Pal_LoadObject(objects, g.nObject, 2, f.fpSSS);
		//
		// Convert the DOS-style data structure to WIN-style data structure
		//
		for (int i = 0; i < g.nObject; i++)
		{
			memcpy(&g.rgObject[i], &objects[i], sizeof(OBJECT_DOS));
			g.rgObject[i].rgwData[6] = objects[i].rgwData[5];     // wFlags
			g.rgObject[i].rgwData[5] = 0;                         // wScriptDesc or wReserved2
		}
	}

	PAL_MKFReadChunk((LPBYTE)(&(g.PlayerRoles)), sizeof(PLAYERROLES),
		3, f.fpDATA);
	//DO_BYTESWAP(&(g.PlayerRoles), sizeof(PLAYERROLES));

	//
	// Set some other default data.
	//
	dwCash = 0;
	wNumMusic = 0;
	wNumPalette = 0;
	wNumScene = 1;
	wCollectValue = 0;
	fNightPalette = FALSE;
	wMaxPartyMemberIndex = 0;
	viewport = PAL_XY(0, 0);
	wLayer = 0;
	wChaseRange = 1;

#ifdef FINISH_GAME_MORE_ONE_TIME
	bFinishGameTime = 0;
#endif
	memset(rgInventory, 0, sizeof(rgInventory));
	memset(rgPoisonStatus, 0, sizeof(rgPoisonStatus));
	memset(rgParty, 0, sizeof(rgParty));
	memset(rgTrail, 0, sizeof(rgTrail));
	memset(&(Exp), 0, sizeof(Exp));

	for (int i = 0; i < MAX_PLAYER_ROLES; i++)
	{
		Exp.rgPrimaryExp[i].wLevel = g.PlayerRoles.rgwLevel[i];
		Exp.rgHealthExp[i].wLevel = g.PlayerRoles.rgwLevel[i];
		Exp.rgMagicExp[i].wLevel = g.PlayerRoles.rgwLevel[i];
		Exp.rgAttackExp[i].wLevel = g.PlayerRoles.rgwLevel[i];
		Exp.rgMagicPowerExp[i].wLevel = g.PlayerRoles.rgwLevel[i];
		Exp.rgDefenseExp[i].wLevel = g.PlayerRoles.rgwLevel[i];
		Exp.rgDexterityExp[i].wLevel = g.PlayerRoles.rgwLevel[i];
		Exp.rgFleeExp[i].wLevel = g.PlayerRoles.rgwLevel[i];
	}

	fEnteringScene = TRUE;
}
/*
void CPalData::trim(char* str)
{
	int pos = 0;
	char* dest = str;

	//
	// skip leading blanks
	//
	while (str[pos] <= ' ' && str[pos] > 0)
		pos++;

	while (str[pos])
	{
		*(dest++) = str[pos];
		pos++;
	}
	*(dest--) = '\0'; // store the null
	// remove trailing blanks
	while (dest >= str && *dest <= ' ' && *dest > 0)
		*(dest--) = '\0';
}
*/
std::string CPalData::PAL_TextToUTF8(const std::string& s)
{
	auto& gConfig = CPalEvent::ggConfig;
	if (s.empty()) return std::string();

	Cls_Iconv m{};
	std::string rs;
	if (bIsBig5 && !gConfig->fIsUseBig5)
	{
		rs = m.GBKtoUTF8(m.Big5ToGb2312(s));
	}
	else if (bIsBig5 && gConfig->fIsUseBig5)
	{
		rs = m.BIG5toUTF8(s);
	}
	else if (!bIsBig5 && gConfig->fIsUseBig5)
	{
		rs = m.BIG5toUTF8(m.Gb2312ToBig5(s));
	}
	else
	{
		rs = m.GBKtoUTF8(s);
	}
	//去除尾部空格
	while (!rs.empty() && (UINT)rs.back() <= 32) {
		rs.pop_back();
	}
	return rs;
}

std::string CPalData::PAL_GetWord(WORD wNumWord)
{
	static char buf[WORD_LENGTH + 10];
	static LPCSTR  cw_NewWord[] =
	{
		"信息",
		"情报",
		"灵抗",
		"毒抗",
		"可偷",
		"巫抗",
		"二次攻击",
		"附加攻击",
		"魔法",
		"物抗",
		"灵壶值",//20011
		"进度一",
		"进度二",
		"进度三",
		"进度四",
		"进度五",
		"进度六",
		"进度七",
		"进度八",
		"进度九",
		"进度十",
		"进度",
	};

	if (wNumWord >= PAL_ADDITIONAL_WORD_SECOND && wNumWord <
		PAL_ADDITIONAL_WORD_SECOND + sizeof(cw_NewWord) / sizeof(LPSTR))
	{
		//return cw_NewWord[wNumWord - PAL_ADDITIONAL_WORD_SECOND];
		//由于新加字是简体，需要转换
		if (bIsBig5)
		{
			Cls_Iconv m;
			std::string p = cw_NewWord[wNumWord - PAL_ADDITIONAL_WORD_SECOND];//UTF8
			std::string p1 = m.UTF8toGBK(p);
			p1 = m.Gb2312ToBig5(p1);
			return PAL_TextToUTF8(p1);//返回繁体UTF8
		}
		else
			return cw_NewWord[wNumWord - PAL_ADDITIONAL_WORD_SECOND];//直接返回UTF8简体

	}
	else if (wNumWord >= g_TextLib.nWords)
	{
		assert(false);
		return std::string();
	}

	SDL_zero(buf);
	memcpy(buf, &g_TextLib.lpWordBuf[wNumWord * WORD_LENGTH], WORD_LENGTH);
	buf[WORD_LENGTH] = '\0';

	//
	// Remove the trailing spaces
	//
	trim(buf);

	if ((strlen(buf) & 1) != 0 && buf[strlen(buf) - 1] == '1')
	{
		//buf[strlen(buf) - 1] = '\0';
	}

	return PAL_TextToUTF8(buf);
}

PalErr CPalData::loadConfig()
{
	//auto& gConfig = CPalEvent::ggConfig;
	if (!gConfig)
		gConfig = new CConfig;
	CPalEvent::ggConfig = gConfig;
	if (PAL_MKFGetChunkCount(f.fpDATA) < 16)
		return 0;//没有存入数据
	if (!gConfig)//出错
		return 1;
	int len = PAL_MKFGetChunkSize(15, f.fpDATA);
	BYTE buf[2048]{ 0 };
	PAL_MKFReadChunk(buf, len, 15, f.fpDATA);
	memcpy(gConfig->m_Function_Set, buf, sizeof(gConfig->m_Function_Set));

	RangeValues(gConfig->m_Function_Set[45], 0, 4);
	RangeValues(gConfig->m_Function_Set[46], 0, 1);
	RangeValues(gConfig->m_Function_Set[47], 5, 30);
	RangeValues(gConfig->m_Function_Set[48], -80, 500);//加速
	//装入字库目录
	if (len > sizeof(gConfig->m_Function_Set))
	{
		CPalEvent::ggConfig->m_FontName = (LPCSTR)( buf + sizeof(gConfig->m_Function_Set));
	}
	return 0;
}

INT CPalData::PAL_IsPalWIN()
{
	//测试是否是Win_95系统
	long i = PAL_MKFGetChunkSize(2, f.fpSSS);
	long  wordlen = f.fpWord.size();

	wordlen = (wordlen + WORD_LENGTH - 1) / WORD_LENGTH; //对象数上限
	if (i > 6.5 * wordlen * 2)
		return 1;
	return 0;

}

template<typename T>  
PalErr CPalData::Pal_LoadObject(std::vector<T>& data,int& len, int n, ByteArray& fp) {  
   if (fp.empty())  
       return 1;  
   len = PAL_MKFGetChunkSize(n, fp);  
   if (len <= 0)  
       return 1;  
   data.resize(len / sizeof(T));
   PAL_MKFReadChunk((LPBYTE)data.data(), len, n, fp);
   len = data.size();
   return 0;
}


VOID CPalData::PAL_InitGlobalGameData()
/*++
Purpose:
Initialize global game data.
Parameters:
None.
Return value:
None.
--*/
{
	int        len;

	//
	// If the memory has not been allocated, allocate first.
	//
	if (g.lprgEventObject.empty())
	{
		Pal_LoadObject(g.lprgEventObject, g.nEventObject,0,f.fpSSS);
		Pal_LoadObject(g.lprgScriptEntry, g.nScriptEntry, 4, f.fpSSS);
		Pal_LoadObject(g.lprgStore, g.nStore, 0, f.fpDATA);
		Pal_LoadObject(g.lprgEnemy, g.nEnemy, 1, f.fpDATA);
		Pal_LoadObject(g.lprgEnemyTeam, g.nEnemyTeam, 2, f.fpDATA);
		//sizeof(MAGIC);
		Pal_LoadObject(g.lprgMagic, g.nMagic, 4, f.fpDATA);
		Pal_LoadObject(g.lprgBattleField, g.nBattleField, 5, f.fpDATA);
		Pal_LoadObject(g.lprgLevelUpMagic, g.nLevelUpMagic, 6, f.fpDATA);

		PAL_ReadGlobalGameData();
	}
}

VOID CPalData::PAL_ReadGlobalGameData()
/*++
Purpose:    Read global game data from data files.
Parameters:    None.
Return value:    None.
--*/
{
	SGAMEDATA* p = &g;

	Pal_LoadObject(p->lprgScriptEntry, (int&)p->nScriptEntry, 4, f.fpSSS);
	Pal_LoadObject(p->lprgStore, (int&)p->nStore, 0, f.fpDATA);
	Pal_LoadObject(p->lprgEnemy, (int&)p->nEnemy, 1, f.fpDATA);
	Pal_LoadObject(p->lprgEnemyTeam, (int&)p->nEnemyTeam, 2, f.fpDATA);
	Pal_LoadObject(p->lprgMagic, (int&)p->nMagic, 4, f.fpDATA);
	Pal_LoadObject(p->lprgBattleField, (int&)p->nBattleField, 5, f.fpDATA);
	Pal_LoadObject(p->lprgLevelUpMagic, (int&)p->nLevelUpMagic, 6, f.fpDATA);

	PAL_MKFReadChunk((LPBYTE)(p->rgwBattleEffectIndex), sizeof(p->rgwBattleEffectIndex),
		11, f.fpDATA);
	DO_BYTESWAP(p->rgwBattleEffectIndex, sizeof(p->rgwBattleEffectIndex));
	PAL_MKFReadChunk((LPBYTE) & (p->EnemyPos), sizeof(p->EnemyPos),
		13, f.fpDATA);
	DO_BYTESWAP(&(p->EnemyPos), sizeof(p->EnemyPos));
	PAL_MKFReadChunk((LPBYTE)(p->rgLevelUpExp), sizeof(p->rgLevelUpExp),
		14, f.fpDATA);
	DO_BYTESWAP(p->rgLevelUpExp, sizeof(p->rgLevelUpExp));

}


//返回存档文件结构中的 场景结构指针，输入指向存档结构，返回指向场景结构指针
LPSCENE CPalData::getSecensPoint(const LPVOID p)
{
	return ((SAVEDGAME*)p)->rgScene;
}

VOID CPalData::PAL_New_GoBackAndLoadDefaultGame()
/*++
Purpose:穿越回去，除了主角的各项属性，其余的都恢复到最初的数据
--*/
{
	SGAMEDATA* p = &g;
	UINT32             i;
	PLAYERROLES playerRoles;

	memset(&playerRoles, 0, sizeof(playerRoles));
	//
	// Load the default data from the game data files.
	//
	Pal_LoadObject(p->lprgEventObject,p->nEventObject,0, f.fpSSS);

	Pal_LoadObject(p->rgScene, p->nScene,1,f.fpSSS);

	if (CPalEvent::ggConfig->fIsWIN95)
	{
		PAL_MKFReadChunk((LPBYTE)(p->rgObject.data()), sizeof(OBJECT), 2, f.fpSSS);
		DO_BYTESWAP(p->rgObject.data(), sizeof(OBJECT));
	}
	else
	{
		OBJECT_DOS objects[MAX_OBJECTS]{};
		PAL_MKFReadChunk((LPBYTE)(objects), sizeof(objects), 2, f.fpSSS);
		DO_BYTESWAP(objects, sizeof(objects));
		//
		// Convert the DOS-style data structure to WIN-style data structure
		//转化格式
		for (i = 0; i < MAX_OBJECTS; i++)
		{
			memcpy(&p->rgObject[i], &objects[i], sizeof(OBJECT_DOS));
			p->rgObject[i].rgwData[6] = objects[i].rgwData[5];     // wFlags
			p->rgObject[i].rgwData[5] = 0;                         // wScriptDesc or wReserved2
		}
	}

	PAL_MKFReadChunk((LPBYTE)(&playerRoles), sizeof(PLAYERROLES), 3, f.fpDATA);
	DO_BYTESWAP(&(playerRoles), sizeof(PLAYERROLES));

	// Set some other default data.
	wNumMusic = 0;
	wNumPalette = 0;
	wNumScene = 1;
	fNightPalette = FALSE;
	wMaxPartyMemberIndex = 0;
	viewport = PAL_XY(0, 0);
	wLayer = 0;
	wChaseRange = 1;

#ifndef PAL_CLASSIC
	bBattleSpeed = 2;
#endif

	bFinishGameTime++;

	memset(rgPoisonStatus, 0, sizeof(rgPoisonStatus));
	memset(rgParty, 0, sizeof(rgParty));
	memset(rgTrail, 0, sizeof(rgTrail));
	//memset(&(Exp), 0, sizeof(Exp));

	for (i = 0; i < MAX_PLAYER_ROLES; i++)
	{
		Exp.rgPrimaryExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
		Exp.rgHealthExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
		Exp.rgMagicExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
		Exp.rgAttackExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
		Exp.rgMagicPowerExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
		Exp.rgDefenseExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
		Exp.rgDexterityExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
		Exp.rgFleeExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
	}

#ifdef FINISH_GAME_MORE_ONE_TIME


	switch (bFinishGameTime)
	{
	case 0:
	case 1:
	{
		dwCash = 0;
		wCollectValue = 0;
		memset(rgInventory, 0, sizeof(rgInventory));
		// 此处还应去除每个人的绝技，例如酒神、剑灭、梦蛇、满天花雨、无心咒等
		// 还原佩戴的物品
		for (i = 0; i < MAX_PLAYER_ROLES; i++)
		{
			g.PlayerRoles.rgwEquipment[kBodyPartWear][i] = playerRoles.rgwEquipment[kBodyPartWear][i];
		}
		break;
	}

	default:
	{
		break;
	}
	}
#endif

	PAL_PlayerLevelUp(RoleID_ZhaoLingEr, 4);
	PAL_PlayerLevelUp(RoleID_LinYueRu, 6);
	PAL_PlayerLevelUp(RoleID_ANu, 45);
	PAL_PlayerLevelUp(RoleID_WuHou, 80);
	PAL_PlayerLevelUp(RoleID_GaiLuoJiao, 80);

	for (i = 0; i < MAX_PLAYER_ROLES; i++)
	{
		g.PlayerRoles.rgwAvatar[i] = playerRoles.rgwAvatar[i];
		g.PlayerRoles.rgwSpriteNumInBattle[i] = playerRoles.rgwSpriteNumInBattle[i];
		g.PlayerRoles.rgwSpriteNum[i] = playerRoles.rgwSpriteNum[i];
		g.PlayerRoles.rgwWalkFrames[i] = playerRoles.rgwWalkFrames[i];
		g.PlayerRoles.rgwHP[i] = g.PlayerRoles.rgwMaxHP[i];
		g.PlayerRoles.rgwMP[i] = g.PlayerRoles.rgwMaxMP[i];
	}

	fEnteringScene = TRUE;
}

VOID CPalData::PAL_PlayerLevelUp(WORD wPlayerRole, WORD wNumLevel)
/*++
Purpose:

Increase the player's level by wLevels.

Parameters:

[IN]  wPlayerRole - player role ID.

[IN]  wNumLevel - number of levels to be increased.

Return value:

None.

--*/
{
	WORD          i;

	// Add the level
	g.PlayerRoles.rgwLevel[wPlayerRole] += wNumLevel;
	if (g.PlayerRoles.rgwLevel[wPlayerRole] > MAX_LEVELS
#if 0

#else
#if FINISH_GAME_MORE_ONE_TIME
		&& !bFinishGameTime
#endif
#endif
		)
	{
		g.PlayerRoles.rgwLevel[wPlayerRole] = MAX_LEVELS;
	}

	for (i = 0; i < wNumLevel; i++)
	{
		//
		// Increase player's stats
		//
		auto& RandomLong = CPalEvent::RandomLong;
		g.PlayerRoles.rgwMaxHP[wPlayerRole] += 12 + RandomLong(0, 2);
		g.PlayerRoles.rgwMaxMP[wPlayerRole] += 9 + RandomLong(0, 1);
		g.PlayerRoles.rgwAttackStrength[wPlayerRole] += 4 + RandomLong(0, 1);
		g.PlayerRoles.rgwMagicStrength[wPlayerRole] += 4 + RandomLong(0, 1);
		g.PlayerRoles.rgwDefense[wPlayerRole] += 2 + RandomLong(0, 1);
		g.PlayerRoles.rgwDexterity[wPlayerRole] += 2 + RandomLong(0, 1);
		g.PlayerRoles.rgwFleeRate[wPlayerRole] += 2 + RandomLong(0, 1);
	}


#define STAT_LIMIT(t) { if ((t) > MAX_PARAMETER) (t) = MAX_PARAMETER; }
	STAT_LIMIT(g.PlayerRoles.rgwMaxHP[wPlayerRole]);
	STAT_LIMIT(g.PlayerRoles.rgwMaxMP[wPlayerRole]);
	STAT_LIMIT(g.PlayerRoles.rgwAttackStrength[wPlayerRole]);
	STAT_LIMIT(g.PlayerRoles.rgwMagicStrength[wPlayerRole]);
	STAT_LIMIT(g.PlayerRoles.rgwDefense[wPlayerRole]);
	STAT_LIMIT(g.PlayerRoles.rgwDexterity[wPlayerRole]);
	STAT_LIMIT(g.PlayerRoles.rgwFleeRate[wPlayerRole]);
#undef STAT_LIMIT


	//
	// Reset experience points to zero
	//
	Exp.rgPrimaryExp[wPlayerRole].wExp = 0;
	Exp.rgPrimaryExp[wPlayerRole].wLevel =
		g.PlayerRoles.rgwLevel[wPlayerRole];
}

VOID CPalData::PAL_CompressInventory()
/*++
Purpose:    Remove all the items in inventory which has a number of zero.
Parameters:    None.
Return value:    None.
--*/
{
	int i, j;

	j = 0;

	for (i = 0; i < MAX_INVENTORY; i++)
	{
		if (rgInventory[i].wItem == 0)
		{
			break;
		}

		if (rgInventory[i].nAmount > 0)
		{
			rgInventory[j] = rgInventory[i];
			j++;
		}
	}

	for (; j < MAX_INVENTORY; j++)
	{
		rgInventory[j].nAmount = 0;
		rgInventory[j].nAmountInUse = 0;
		rgInventory[j].wItem = 0;
	}
	PAL_New_SortInventory();
}

VOID CPalData::PAL_New_SortInventory()
{
	int         i, j;
	WORD        ItemID1, ItemID2;
	WORD		ItemNum;
	INVENTORY   TempItem;
	INVENTORY	TempInventory[MAX_INVENTORY];

	memset(TempInventory, 0, sizeof(TempInventory));

	for (i = 0, j = 0; i < MAX_INVENTORY; i++)
	{
		TempItem = rgInventory[i];
		if (TempItem.wItem != 0 && TempItem.nAmount != 0)
		{
			TempInventory[j] = TempItem;
			j++;
		}
	}
	ItemNum = j;

	for (i = 0; i < ItemNum; i++)
	{
		for (j = 0; j < ItemNum - i - 1; j++)
		{
			ItemID1 = TempInventory[j].wItem;
			ItemID2 = TempInventory[j + 1].wItem;


			if (ItemID1 > ItemID2)
			{
				TempItem = TempInventory[j];
				TempInventory[j] = TempInventory[j + 1];
				TempInventory[j + 1] = TempItem;
			}
		}
	}

	for (i = 0; i < MAX_INVENTORY; i++)
	{
		rgInventory[i] = TempInventory[i];
	}
	return;
}

const size_t CPalData::getSaveFileLen()const
{

	if (CPalEvent::ggConfig->fIsWIN95)
		return sizeof(SAVEDGAME) + g.nEventObject * sizeof(EVENTOBJECT) -
			sizeof(EVENTOBJECT) * MAX_EVENT_OBJECTS;
	else
		return sizeof(SAVEDGAME_DOS) + g.nEventObject * sizeof(EVENTOBJECT) -
			sizeof(EVENTOBJECT) * MAX_EVENT_OBJECTS;
}

//get Object displacement
//取对象结构在存储文件中的位移 0 对象，2 场景 1 场景对象
const size_t CPalData::getSaveFileOffset(int ob)const
{
	return tagSAVEDGAME_DOS::getOffset(ob, CPalEvent::ggConfig->fIsWIN95);
}

//从存档文件缓存中返回指定结构指定位置的指针
//ob = 1 OBJECT,ob = 2 scene ob = 3 EventObject
// row 为 行 col 为栏 fp 存档数据
LPWORD CPalData::getSaveFileObject(int ob,int row,int col, ByteArray& fp) {
	if (CPalEvent::ggConfig->fIsWIN95)
	{
		auto s = (SAVEDGAME*)(fp.data());
		switch (ob)
		{
		case 1:
			return &s->rgObject[row].rgwData[col];
			break;
		case 2:
			return &((LPWORD)(&s->rgScene[row]))[col];
			break;
		case 3:
			return &((LPWORD)((LPOBJECTDESC)&s->rgEventObject[row]))[col];
			break;
		default:
			return nullptr;
		}
	}
	else
	{
		auto s = (SAVEDGAME_DOS*)(fp.data());
		switch (ob)
		{
		case 1:
			return &s->rgObject[row].rgwData[col];
			break;
		case 2:
			return &((LPWORD)(&s->rgScene[row]))[col];
			break;
		case 3:
			return &((LPWORD)((LPOBJECTDESC)&s->rgEventObject[row]))[col];
			break;
		default:
			return nullptr;
		}
	}
	return nullptr;
}
 
