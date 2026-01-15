#pragma once
///////cpaldata.h///////
#include <string>
#include <vector>
#include "command.h"
#include "palgpgl.h"
#include "cconfig.h"
#include <Windows.h>

#define   FONT_COLOR_DEFAULT        0x4F
#define   FONT_COLOR_YELLOW         0x2D
#define   FONT_COLOR_RED            0x1A
#define   FONT_COLOR_CYAN           0x8D
#define   FONT_COLOR_CYAN_ALT       0x8C
#define   CHUNKNUM_SPRITEUI                  9

class SFILES
{
public:
	ByteArray fpABC;	  //我方战斗图像
	ByteArray fpMAP;      //地图数据文件
	ByteArray fpGOP;	  //地图图像文件
	ByteArray fpFBP;      // battlefield background images
	ByteArray fpMGO;      // sprites in scenes
	ByteArray fpBALL;     // item bitmaps
	ByteArray fpDATA;     // misc data
	ByteArray fpF;        // player sprites during battle
	ByteArray fpFIRE;     // fire effect sprites
	ByteArray fpRGM;      // character face bitmaps
	ByteArray fpSSS;      // script data
	ByteArray fpPAT;	  // 调色版文件
	ByteArray fpRNG;	  //RNG文件
	ByteArray fpMsg;	  //m.msg
	ByteArray fpWord;	  //word.data	 
	ByteArray fpDesc;	  //dest.data 说明文件	 
};

// game data which is available in data files.
struct SGAMEDATA
{
	//LPEVENTOBJECT           lprgEventObject{};
	std::vector<EVENTOBJECT> lprgEventObject{}; //事件对象
	int                     nEventObject{};

	//SCENE                   rgScene[MAX_SCENES]{};
	std::vector<SCENE>      rgScene{}; //场景数据
	int						nScene{};

	//LPOBJECT                rgObject{};
	std::vector<OBJECT>		rgObject{}; //对象数据
	int						nObject{};

	//LPSCRIPTENTRY           lprgScriptEntry{};
	std::vector<SCRIPTENTRY> lprgScriptEntry{}; //脚本数据
	int                     nScriptEntry{};

	//LPSTORE                 lprgStore{};
	std::vector<STORE>		lprgStore{}; //商店数据
	int                     nStore{};

	//LPENEMY                 lprgEnemy{};
	std::vector<ENEMY>		lprgEnemy{}; //敌人数据
	int                     nEnemy{};

	//LPENEMYTEAM             lprgEnemyTeam{};
	std::vector<ENEMYTEAM>  lprgEnemyTeam{}; //敌人队伍数据
	int                     nEnemyTeam{};

	PLAYERROLES             PlayerRoles{};

	//LPMAGIC                 lprgMagic{};
	std::vector<MAGIC>      lprgMagic{}; //魔法数据
	int                     nMagic{};

	//LPBATTLEFIELD           lprgBattleField{};
	std::vector<BATTLEFIELD> lprgBattleField{}; //战斗场景数据
	int                     nBattleField{};

	//LPLEVELUPMAGIC_ALL      lprgLevelUpMagic{};
	std::vector<LEVELUPMAGIC_ALL> lprgLevelUpMagic{}; //升级魔法数据
	int                     nLevelUpMagic{};

	ENEMYPOS                EnemyPos{};
	LEVELUPEXP              rgLevelUpExp[MAX_LEVELS + 1]{};

	WORD                    rgwBattleEffectIndex[10][2]{};
};


//公共数据模块
class CPalData 
{
public:
	SFILES f{};
	SGAMEDATA g{};
	TEXTLIB				g_TextLib{};
	LPSPRITE			gpSpriteUI{};
	class CConfig*		gConfig{}; 
	std::string			PalDir;
	std::vector< ByteArray>	rgSaveData;//0--maxSave
	int					rgSaveDataChaged[MAXSAVEFILES]{};

	int 			 isErr{};
	int              iCurMainMenuItem{};    // current main menu item number
	int              iCurSystemMenuItem{};  // current system menu item number
	int              iCurInvMenuItem{};     // current inventory menu item number
	int              iCurPlayingRNG{};      // current playing RNG animation
	int              bCurrentSaveSlot{};    // current save slot (1-5)
	BOOL             fInMainGame{};         // TRUE if in main game
	BOOL             fEnteringScene{};      // TRUE if entering a new scene
	BOOL             fNeedToFadeIn{};       // TRUE if need to fade in when drawing scene
	BOOL             fInBattle{};           // TRUE if in battle
	BOOL             fAutoBattle{};         // TRUE if auto-battle

	BYTE             bBattleSpeed{};        // Battle Speed (1 = Fastest, 5 = Slowest)

	WORD             wLastUnequippedItem{}; // last unequipped item

	PLAYERROLES      rgEquipmentEffect[MAX_PLAYER_EQUIPMENTS + 1]{}; // equipment effects
	WORD             rgPlayerStatus[MAX_PLAYER_ROLES][kStatusAll]{}; // player status

	PAL_POS          viewport{};            // viewport coordination
	PAL_POS          partyoffset{};
	WORD             wLayer{};
	WORD             wMaxPartyMemberIndex{};// max index of members in party (0 to MAX_PLAYERS_IN_PARTY - 1)
	PARTY            rgParty[MAX_PLAYABLE_PLAYER_ROLES]{}; // player party
	TRAIL            rgTrail[MAX_PLAYABLE_PLAYER_ROLES]{}; // player trail
	WORD             wPartyDirection{};     // direction of the party
	WORD             wNumScene{};           // current scene number
	WORD             wNumPalette{};         // current palette number
	BOOL             fNightPalette{};       // TRUE if use the darker night palette
	WORD             wNumMusic{};           // current music number
	WORD             wNumBattleMusic{};     // current music number in battle
	WORD             wNumBattleField{};     // current battle field number
	WORD             wCollectValue{};       // value of "collected" items
	WORD             wScreenWave{};         // level of screen waving
	SHORT            sWaveProgression{};
	WORD             wChaseRange{};
	WORD             wChasespeedChangeCycles{};
	USHORT           nFollower{};

	DWORD            dwCash{};              // amount of cash

	ALLEXPERIENCE    Exp{};                 // experience status
	POISONSTATUS     rgPoisonStatus[MAX_POISONS][MAX_PLAYABLE_PLAYER_ROLES]{}; // poison status
	INVENTORY        rgInventory[MAX_INVENTORY]{};  // inventory status
	LPOBJECTDESC     lpObjectDesc{};
	DWORD            dwFrameNum{};

	/////////
	BOOL			 fGameStart{};
	INT				 bFinishGameTime{};
	BOOL			 fShowDataInBattle{};
	BOOL			 bIsBig5{};
	WORD			 bSavedTimes{};

public:
	CPalData() = delete;
	CPalData(const std::string& dir,const CPalData* pf = nullptr );
	~CPalData();
	PalErr loadAllData(const std::string& dir, const CPalData* pf);
	
	static ByteArray readAll(const std::string& fileName);
	static PalErr loadData(std::string filename, ByteArray& s);
	static int PAL_MKFGetChunkCount(const ByteArray& fp);
	static INT PAL_MKFGetChunkSize(UINT uiChunkNum, const ByteArray& fp);
	
	// 判断文件是否存在
	static BOOL IsFileExist(const std::string& csFile);
	// 判断文件是否存在
	static BOOL IsDirExist(const std::string& csFile);

	static int pByteToInt(LPCBYTE p) {
		return (int)(p[0] +
			(p[1] << 8) + (p[2] << 16) + (p[3] << 24));
	}
	static int PAL_MKFReadChunk(LPBYTE lpBuffer, UINT  uiBufferSize, UINT  uiChunkNum, const ByteArray& fp);;
	static ByteArray PAL_MKFReadChunk(UINT  uiChunkNum, const ByteArray& fp);

	//压缩解压完整类型测试 
	INT is_Use_YJ1_Decompress();
	static PalErr PAL_DeCompress(LPCVOID Source, LPVOID Destination, INT DestSize);
	static PalErr EnCompress(const void* Source, UINT32 SourceLength, void*& Destination, UINT32& Length, bool bCopatible = 1);
private:
	//压缩函数
	static PalErr EncodeYJ1(const void* Source, UINT32 SourceLength, void*& Destination, UINT32& Length);
	static PalErr EncodeYJ2(const void* Source, UINT32 SourceLength, void*& Destination, UINT32& Length, bool bCompatible);
	//解压
	static PalErr YJ1_Decompress(LPCVOID Source, LPVOID Destination, INT DestSize);
	static PalErr YJ2_Decompress(LPCVOID Source, LPVOID Destination, INT DestSize);

public:	
	#define  RangeValues(val,mix,max) {if(val<(mix))val=(mix);else if(val>(max))val=(max);} 
	
	PalErr loadConfig();
	INT PAL_IsPalWIN();
	VOID PAL_InitGlobalGameData();
	VOID PAL_ReadGlobalGameData();
	PalErr PAL_LoadGame(LPCSTR szFileName);
	PalErr PAL_LoadGame(const ByteArray& fi);
	VOID PAL_SaveGame(int iSlot, WORD  wSavedTimes,bool noSave);
	VOID PAL_SaveGame(ByteArray & fp, WORD  wSavedTimes);
	VOID  PAL_LoadDefaultGame();
	
	//static void trim(char* str);
	std::string PAL_TextToUTF8(const std::string& s);
	std::string PAL_GetWord(WORD wNumWord);

	static char* bgets(char* str, int num, const ByteArray& vec, size_t& position) {
		if (num <= 0) {
			return nullptr;
		}

		if (position >= vec.size()) {
			return nullptr;  // 到达向量末尾
		}

		int i = 0;
		while (i < num - 1 && position < vec.size()) {
			char c = static_cast<char>(vec[position]);
			str[i] = c;
			position++;
			i++;

			// 如果遇到换行符，停止读取
			if (c == '\n') {
				break;
			}
		}

		// 如果没有读取到任何字符
		if (i == 0) {
			return nullptr;
		}

		str[i] = '\0';  // 添加字符串终止符
		return str;
	}
	//返回存档文件结构中的 场景结构指针，输入指向存档结构，返回指向场景结构指针
	LPSCENE getSecensPoint(const LPVOID p);

	VOID PAL_New_GoBackAndLoadDefaultGame();

	VOID PAL_PlayerLevelUp(WORD wPlayerRole,WORD wNumLevel);

	VOID PAL_CompressInventory();

	VOID PAL_New_SortInventory();

	const size_t getSaveFileLen()const;

	//get Object displacement
	//取对象结构在存储文件中的位移 0 对象，2 场景 1 场景对象
	const size_t getSaveFileOffset(int ob)const;

	LPOBJECTDESC PAL_LoadObjectDesc();

	LPWORD getSaveFileObject(int ob, int row, int col, ByteArray& fp);

private:
		INT  PAL_IsBig5(INT Default_Decision);

		PalErr PAL_InitText();

		PalErr PAL_InitUI();

		VOID PAL_FreeGlobals();

		VOID PAL_FreeUI();

		VOID PAL_FreeText();
		//释放说明
		VOID PAL_FreeObjectDesc(LPOBJECTDESC   lpObjectDesc);
		//加载对象
		template<typename T>
		PalErr Pal_LoadObject(std::vector<T>& data,int &len, int n, ByteArray& fp);


};