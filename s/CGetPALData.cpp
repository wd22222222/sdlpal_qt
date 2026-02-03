///* -*- mode: c; tab-width: 4; c-basic-offset: 4; c-file-style: "linux" -*- */
//
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

#include "cgetpaldata.h"
#include <thread>
#include "main/Convers.h"
#include <qdir.h>
#include <qfiledialog.h>
#include <qdialog.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
std::string  p_Script(int i)
{
	//返回脚本说明
	std::string s = "";
	switch (i)
	{
	case 0:
		s = "停止运行";
		break;
	case 0x0001:
		s = "停止运行，用下行替换，用于下一轮运行";
		break;
	case 0x0002:
		s = "停止运行，用参数1替换，用于下一轮运行,条件是参数 2 = 0 或小于对象空闲帧数";
		break;
	case 0x0003:
		s = "无条件跳转到参数 1位置,如参数2不等于0且未达到参数2指定的帧数 运行下一条";
		break;
	case 0x0004:
		s = "运行 参数1 指向的子脚本";
		break;
	case 0x0005:
		s = "刷新场景";
		break;
	case 0x0006:
		s = "条件跳转  参数1 概率  满足执行下一条，不满足跳转到参数2指向的地址";
		break;
	case 0x0007:
		s = "开始战斗  参数1 队伍号， 参数3 为 0 BOSS 参数2 战斗失败执行 参数3 战斗中逃跑执行 ";
		break;
	case 0x0008:
		s = "用下一条指令替换 ";
		break;
	case 0x0009:
		s = "等待 参数1 指定的时间（帧数） ";
		break;
	case 0x000a:
		s = "跳转到 参数1 指定地址，如玩家选择 No ";
		break;
	case 0x008e:
		s = "恢复场景";
		break;
	case 0xffff:
		s = "显示文字 参数1 文字地址";
		break;
	case 0x00ff:
		s = "随机执行 1-参数1 之间的任意一条指令  之后跳过 参数1 条数继续执行";
		break;
	case 0x000b:
	case 0x000c:
	case 0x000d:
	case 0x000e:
		s = "行走1步  入口 - 0x000b 方向";
		break;
	case 0x000f:
		s = "设置对象的方向和手势 参数1 方向 ，参数2 手势";
		break;
	case 0x0010:
		s = "直走到指定的位置 ,参数1 X，参数2 Y，参数3 高度";
		break;
	case 0x0011:
		s = "慢走到指定的位置 ,参数1 X，参数2 Y，参数3 高度";
		break;
	case 0x0012:
		s = "设置对象到相对于队伍的位置 参数1对象,参数2 X，参数3 Y";
		break;
	case 0x0013:
		s = "设置对象到指定的位置 参数1对象,参数2 X，参数3 Y";
		break;
	case 0x0014:
		s = "设置对象的形象（手势），参数1 手势 ，方向南";
		break;
	case 0x0015:
		s = "设置队伍成员的方向，形象（手势），参数1 队伍方向 ，参数3，成员序号，参数2，该成员的相对方向";
		break;
	case 0x0016:
		s = "设置对象的，方向和（ 手势），参数1对象不为0 ，参数2，方向，参数3，形象";
		break;
	case 0x0017:
		s = "设置额外属性，战后清除，参数1 位置，头0x0b,以下顺延，参数2属性09:体力,0A：真气.11：武术.12：灵力.13：防御.14：身法.15：吉运.16：毒抗17：风抗";
		break;
	case 0x0018:
		s = "指定部位的装备 参数1 = 0xb 头，参数2 新装备";
		break;
	case 0x0019:
		s = "属性增加，参数2 值，参数3 1 李，0本人，参数1= 06 等级，07 最大体力，08 最大真气，09:体力,0A：真气.11：武术.12：灵力.13：防御.14：身法.15：吉运.16：毒抗17：风抗";
		break;
	case 0x001a:
		s = "属性写入，参数2 值，参数3 1 李，0本人，参数1= 06 等级，07 最大体力，08 最大真气，09:体力,0A：真气.11：武术.12：灵力.13：防御.14：身法.15：吉运.16：毒抗17：风抗";
		break;
	case 0x001b:
		s = "体力增加减少，参数1 = 1 全体 0 单人，参数2 值";
		break;
	case 0x001c:
		s = "真气增加减少，参数1 = 1 全体 0 单人，参数2 值";
		break;
	case 0x001d:
		s = "体力、真气增加减少，参数1 = 1 全体 0 单人，参数2 体力值，参数3 真气值，参数3为0时=参数2";
		break;
	case 0x001e:
		s = "金钱改变，参数1 = 值，如参数1为且钱不足时，跳到参数2执行";
		break;
	case 0x001f:
		s = "向库存中添加物品，参数1 品种，参数2 数量";
		break;
	case 0x0020:
		s = "从库存中移除物品，参数1 品种，参数2 数量，如库存没有移除已装备物品，物品不足跳到参数3  ";
		break;
	case 0x0021:
		s = "伤敌指令，参数1 ！=0 全体，0单人。参数2 整数，小于0 增加体力";
		break;
	case 0x0022:
		s = "角色复活指令，参数1 ！=0 全体，0单人。参数2 整数，10满血，";
		break;
	case 0x0023:
		s = "装备移除指令，参数1 =0 李 2 赵，参数2 =0 所有的，=1 头";
		break;
	case 0x0024:
		s = "设置对象自动脚本地址，参数1对象 不等于0 参数2 地址";
		break;
	case 0x0025:
		s = "设置对象触发脚本地址，参数1对象 不等于0 参数2 地址";
		break;
	case 0x0026:
		s = "显示商店，参数1 商店号";
		break;
	case 0x0027:
		s = "显示当铺";
		break;
	case 0x0028:
		s = "敌方中毒指令 参数1 ！=0 全体，参数2 毒,失败跳转到参数3，参数3=0xffff,无视毒抗";
		break;
	case 0x0029:
		s = "我方中毒指令 参数1 ！=0 全体，参数2 毒,失败跳转到参数3,参数3=0xffff,无视毒抗";
		break;
	case 0x002a:
		s = "敌方解毒指令 参数1 ！=0 全体，参数2 毒";
		break;
	case 0x002b:
		s = "我方解毒指令 参数1 ！=0 全体，参数2 毒";
		break;
	case 0x002c:
		s = "我方多重解毒指令 参数1 ！=0 全体，参数2 解除不高于其等级的毒";
		break;
	case 0x002d:
		s = "我方特殊状态指令，参数1 =0 疯、睡、定、封、傀儡、攻、防、身、双击，参数2 回合数。失败跳到参数3";
		break;
	case 0x002e:
		s = "敌方特殊状态指令，参数1 =0 疯、睡、定、傀儡、攻、防、身、双击，参数2 回合数，参数3 如不成功 跳转到参数3指向的地址。";
		break;
	case 0x002f:
		s = "我方解除特殊状态指令，参数1 = 0 疯、睡、定、封、傀儡、攻、防、身、双击";
		break;
	case 0x0030:
		s = "按百分比暂时增加角色属性，参数1 = 11 武、12 灵、13 防、14 身、15 吉、16 毒、17 风、18 雷、19 水、1a 火 1b 土";
		break;
	case 0x0031:
		s = "战斗中改变形象 参数1 形象";
		break;
	case 0x0033:
		s = "收集妖怪炼丹,参数1 失败转到";
		break;
	case 0x0034:
		s = "灵壶炼丹指令,参数1 失败转到";
		break;
	case 0x0035:
		s = "摇动场景";
		break;
	case 0x0036:
		s = "设置当前正在播放的RNG动画 参数1";
		break;
	case 0x0037:
		s = "播放的RNG动画 参数1 开始帧数，参数2 播放帧数 （缺省999） 参数3 速度（缺省16） ";
		break;
	case 0x0038:
		s = "将队伍从现场传送出去 失败跳到参数1 指向地址 ";
		break;
	case 0x0039:
		s = "吸取生命 从选定对象吸取 参数1 点数生命补充自己";
		break;
	case 0x003a:
		s = "从战斗中逃走";
		break;
	case 0x003b:
		s = "在场景中间显示对话框,参数1 字体颜色 参数 3 是否显示人像 ";
		break;
	case 0x003c:
		s = "在场景上方显示对话框,参数2 字体颜色 ，参数1 人像号，参数 3 是否显示人像 ";
		break;
	case 0x003D:
		s = "在场景下方显示对话框,参数1 字体颜色 ，参数2 人像号，参数 3 是否显示人像 ";
		break;
	case 0x003e:
		s = "在场景上面显示文字,参数1 字体颜色  ";
		break;
	case 0x003f:
		s = "将指定对象低速移动到指定位置 参数1 X，参数2 Y，参数3 H";
		break;
	case 0x0040:
		s = "设置对象触发模式 如参数1对象 ！= 0 ，参数2 设置";
		break;
	case 0x0041:
		s = "令脚本执行失败";
		break;
	case 0x0042:
		s = "模拟法术，参数1 法术，参数2 基础伤害，参数3 对象";
		break;
	case 0x0043:
		s = "更换背景音乐，参数1 音乐号";
		break;
	case 0x0044:
		s = "将指定对象移动到指定位置 参数1 X，参数2 Y，参数3 H";
		break;
	case 0x0045:
		s = "设置战斗时音乐，参数1 音乐号";
		break;
	case 0x0046:
		s = "设置队伍在地图上的位置， 参数1 X，参数2 Y，参数3 H";
		break;
	case 0x0047:
		s = "播放音乐效果，参数1 音乐号";
		break;
	case 0x0049:
		s = "设置对象状态，参数2 状态";
		break;
	case 0x004a:
		s = "设置战斗场所，参数1 场所";
		break;
	case 0x004b:
		s = "使对象短时间消失";
		break;
	case 0x004c:
		s = "追逐队伍，参数1 最大距离，参数1 速度，参数2 是否浮动";
		break;
	case 0x004d:
		s = "按任意键继续";
		break;
	case 0x004e:
		s = "读入最近使用的文档";
		break;
	case 0x004f:
		s = "将场景淡入红色(游戏结束)";
		break;
	case 0x0050:
		s = "场景淡出";
		break;
	case 0x0051:
		s = "场景淡入";
		break;
	case 0x0052:
		s = "使对象消失一段时间，参数1 时间（缺省800）";
		break;
	case 0x0053:
		s = "使用白天调色版";
		break;
	case 0x0054:
		s = "使用夜间调色版";
		break;
	case 0x0055:
		s = "添加魔法，参数1魔法，参数2 对象，0缺省";
		break;
	case 0x0056:
		s = "移除仙术，参数1魔法，参数2 对象，0缺省";
		break;
	case 0x0057:
		s = "根据真气设定基础伤害";
		break;
	case 0x0058:
		s = "如果库存数量不足则跳过，参数1 品种，参数2 要求数量，参数3 跳转到";
		break;
	case 0x0059:
		s = "更换地图，参数1 地图号";
		break;
	case 0x005a:
		s = "体力减半";
		break;
	case 0x005b:
		s = "敌方体力减半，参数1 对象";
		break;
	case 0x005c:
		s = "隐藏  参数1 回合数";
		break;
	case 0x005d:
		s = "如我方没有中特定的毒，则跳转  参数1 毒种类，参数2 跳转到";
		break;
	case 0x005e:
		s = "如敌方没有中特定的毒，则跳转  参数1 毒种类，参数2 跳转到";
		break;
	case 0x005f:
		s = "我方立刻死亡";
		break;
	case 0x0060:
		s = "敌方立刻死亡";
		break;
	case 0x0061:
		s = "没有中毒，跳转，参数1 跳转到";
		break;
	case 0x0062:
		s = "暂停敌方 追赶，参数1 时间";
		break;
	case 0x0063:
		s = "加速敌方 追赶，参数1 时间";
		break;
	case 0x0064:
		s = "跳转，如敌方生命值高于 设定的百分比 ，参数1 比值，参数2 目标";
		break;
	case 0x0065:
		s = "设置图像，参数1 我方成员代码，参数2 图像，参数3 是否刷新";
		break;
	case 0x0066:
		s = " 投掷武器，参数2 伤害值";
		break;
	case 0x0067:
		s = "设置敌方魔法，参数1魔法，参数2 概率";
		break;
	case 0x0068:
		s = "如敌方行动，跳转到参数1";
		break;
	case 0x0069:
		s = "敌方逃跑";
		break;
	case 0x006a:
		s = "偷敌人，参数1 概率";
		break;
	case 0x006b:
		s = "吹走敌人，";
		break;
	case 0x006C:
		s = "NPC走一步， 参数1 事件对象，参数2 X，参数3 Y";
		break;
	case 0x006D:
		s = "设定一个场景的进入和传送脚本，参数1，场景号，参数2 进入脚本，参数3 传送脚本，参数2 和参数3同时为0 清除脚本";
		break;
	case 0x006E:
		s = "将队伍移动到指定位置 参数1 X，参数2 Y，参数3 H";
		break;
	case 0x006F:
		s = "将当前事件对象状态与另一个事件对象同步 参数1 事件对象号+1 参数2 状态";
		break;
	case 0x0070:
		s = "队伍走到指定位置 参数1 X，参数2 Y，参数3 H";
		break;
	case 0x0071:
		s = "波动场景 参数1，参数2";
		break;
	case 0x0073:
		s = "淡入场景";
		break;
	case 0x0074:
		s = "如不是全体满血，跳到参数1";
		break;
	case 0x0075:
		s = "设置队伍，参数1，参数2，参数3";
		break;
	case 0x0076:
		s = "Show FBP picture 参数1 ，参数2";
		break;
	case 0x0077:
		s = "停止正在播放音乐，参数1";
		break;
	case 0x0078:
		s = "未知";
		break;
	case 0x0079:
		s = "如果指定成员在队伍中，则跳到参数2";
		break;
	case 0x007a:
		s = "队伍快速走到指定位置 参数1 X，参数2 Y，参数3 H";
		break;
	case 0x007b:
		s = "队伍最快速走到指定位置 参数1 X，参数2 Y，参数3 H";
		break;
	case 0x007c:
		s = "队伍直接走到指定位置 参数1 X，参数2 Y，参数3 H";
		break;
	case 0x007D:
		s = "移动对象位置，参数1对象 参数2 X，参数3 Y";
		break;
	case 0x007E:
		s = "设置对象的层，参数1对象，参数2 ";
		break;
	case 0x007f:
		s = "移动视点，参数1 X，参数2 Y，参数3,不等于FFFF 更新场景";
		break;
	case 0x0080:
		s = "白天黑夜转换，参数1";
		break;
	case 0x0081:
		s = "跳过，如果没有面对对象，参数1 对象，参数2 改变触发方式，参数3地址";
		break;
	case 0x0082:
		s = "队伍快速直接走到指定位置 参数1 X，参数2 Y，参数3 H";
		break;
	case 0x0083:
		s = "如果事件对象不在当前事件对象的指定区域，则跳过,参数1对象，参数3地址";
		break;
	case 0x0084:
		s = "将玩家用作事件对象的物品放置到场景中,参数1，参数2，失败跳到参数3";
		break;
	case 0x0085:
		s = "延迟一段时间，参数1";
		break;
	case 0x0086:
		s = "如果未装备指定物品则跳过，参数1 物品，参数3 地址";
		break;
	case 0x0087:
		s = "画对象";
		break;
	case 0x0088:
		s = " 根据所剩金钱设置基础伤害值";
		break;
	case 0x0089:
		s = "设置战斗结果，参数1";
		break;
	case 0x008a:
		s = "设置下一场战斗为自动战斗";
		break;
	case 0x008b:
		s = "改变调色版，参数1";
		break;
	case 0x008c:
		s = "淡入到颜色，参数1 颜色，参数2 延时，参数3";
		break;
	case 0x008d:
		s = "升级，参数1 增加的等级";
		break;
	case 0x008f:
		s = "金钱减半指令";
		break;
	case 0x0090:
		s = "设置对象脚本，参数1 对象，参数3 偏移，参数2 值";
		break;
	case 0x0091:
		s = "如果敌方不是孤单，则跳转 ，参数1 地址";
		break;
	case 0x0092:
		s = "显示玩家魔法动画，参数1 魔法";
		break;
	case 0x0093:
		s = "褪色的场景。更新过程中的场景";
		break;
	case 0x0094:
		s = "如果事件对象的状态是指定的，则跳转,参数1 对象，参数2 状态，参数3 地址";
		break;
	case 0x0095:
		s = "如果当前场景等于参数1，则跳转,参数2 ";
		break;
	case 0x0096:
		s = "播放结束动画";
		break;
	case 0x0097:
		s = "将指定对象最快移动到指定位置 参数1 X，参数2 Y，参数3 H";
		break;
	case 0x0098:
		s = "设置跟随对象,参数1对象 >= 0 设置跟随，0 取消";
		break;
	case 0x0099:
		s = "更改指定场景的映射,参数1 = 0xffff 当前场景，否则参数1指定，参数2 场景";
		break;
	case 0x009A:
		s = "为多个对象设置状态，参数1 到参数2，设置为参数3";
		break;
	case 0x009b:
		s = "淡入当前场景";
		break;
	case 0x009c:
		s = "怪物复制自身,失败跳转到 参数2";
		break;
	case 0x009e:
		s = "怪物召唤，参数1 召唤的怪物ID，参数2 数量，参数3 失败跳转";
		break;
	case 0x009f:
		s = "怪物变身，参数1 要变的怪物ID";
		break;
	case 0x00a0:
		s = "离开游戏";
		break;
	case 0x00a1:
		s = "设置所有成员位置与第一名一样";
		break;
	case 0x00a2:
		s = "随机跳转到之后的第1 到参数1条 指令之一";
		break;
	case 0x00a3:
		s = "播放音乐，参数1，参数2";
		break;
	case 0x00a4:
		s = "Scroll FBP to the screen，参数1，参数2，参数3";
		break;
	case 0x00a5:
		s = "Show FBP picture with sprite ，参数1，参数2，参数3";
		break;
	case 0x00a6:
		s = "备份场景";
		break;
	case 0x00a7:
		s = "WIN95版，对象说明前缀";
		break;
	case 0x00b0:
		s = "消灭水魔兽";
		break;
	case 0x00b1:
		s = "结尾动画";
		break;
	case 0x00b2:
		s = "穿越重新开始";
		break;
	case 0x0100:
		s = "增加队伍中的人员";
		break;
	case 0x0101:
		s = "移除我方人员的装备额外效果";
		break;
	case 0x0102:
		s = "增加我方成员到3人以上，参数1 标志，按位或";
		break;
	case 0x0103:
		s = "队伍中没有指定角色则跳转，参数1 角色，参数2 跳转目标";
		break;
	default:
		s = "错误入口";
		break;
	}
	return  s;
}



CGetPalData::CGetPalData(int save, BOOL noRun, const CPalData* pf)
{
	pal = new CScript(save,noRun,pf);
	PAL_Object_classify();
}

CGetPalData::~CGetPalData() { 
	delete pal; 
	pal = nullptr;
}



int CGetPalData::showChineseMessageBox(QWidget* parent, const QString& title,
	const QString& text, QMessageBox::Icon icon, const QStringList& buttons)
{
	QMessageBox msgBox(parent);
	msgBox.setWindowTitle(title);
	msgBox.setText(text);
	msgBox.setIcon(icon);

	// 默认按钮
	QStringList btnTexts = buttons.isEmpty() ? QStringList{ "确定", "取消" } : buttons;

	QList<QAbstractButton*> addedButtons;
	for (const QString& buttonText : btnTexts) {
		// 使用 ActionRole，避免 ESC 自动绑定到某个按钮
		QAbstractButton* button = msgBox.addButton(buttonText, QMessageBox::ActionRole);
		addedButtons << button;
	}

	// 可选：显式设置默认按钮（第一个）
	if (!addedButtons.isEmpty()) {
		msgBox.setDefaultButton(static_cast<QPushButton*>(addedButtons.first()));
	}

	int result = msgBox.exec();

	QAbstractButton* clicked = msgBox.clickedButton();
	if (!clicked) {
		// 用户按 ESC、点击关闭按钮、或以其他方式关闭
		return -1;
	}

	return addedButtons.indexOf(clicked);
}
//返回是否有修改数据
bool CGetPalData::isSaveDataChaged() const {
	for (int i = 0; i < modRoc_End; i++)
		if (ModifRecord[i])
			return true;
	for (int i = 0; i < MAXSAVEFILES; i++)
		if (pal->gpGlobals->rgSaveDataChaged[i])
			return true;
	return false;
}

VOID CGetPalData::clearSaveDataChangd()
{
	for (int i = 0; i < modRoc_End; i++)
		ModifRecord[i] = 0;
	for (int i = 0; i < MAXSAVEFILES; i++)
		pal->gpGlobals->rgSaveDataChaged[i] = 0;
}
// 清除修改标志


QVector<uint> CGetPalData::sdl_PaltteToQColorList(SDL_Color* colors)
{
	QVector<uint> list;
	if (!colors)
		assert(0);

	for (int n = 0; n < 256; n++)
	{
		PAL_Color c = colors[n];
		list.push_back(c.toQColor());
	}
	return list;
}

uint CGetPalData::sdl_colorToUint(const SDL_Color& c)
{
	//sdl_color 与 qcolor 转换
	return c.b + (c.g << 8) + (c.r << 16) + (255 << 24);
}

PalErr CGetPalData::setCorrectDir(std::string& dir,bool change )
{
	if (!change && (CPalBase::isCorrectDir(dir) || CPalBase::isCorrectDir(dir += "pal/")))
	{
		auto mPalDir = QString(dir.c_str()).toLower();
		//设置工作目录 
		QDir::setCurrent(mPalDir);
		return 0;
	}
	//取得正确的工作目录
	do
	{
		QString PalDir = dir.c_str();
		QString ts = "打开游戏文件夹";
		QString NewDir = QFileDialog::getExistingDirectory(nullptr,ts, PalDir);
		if (NewDir.isEmpty())
		{
			//没有选择正确目录
			return 1;
		}
		NewDir += "/";
		if (QDir(NewDir).exists() && NewDir != PalDir)
		{
			//选择新目录
			dir = NewDir.toStdString();
		}
	} while (!(CPalBase::isCorrectDir(dir) || CPalBase::isCorrectDir(dir += "pal/")));
	auto mPalDir = QString(dir.c_str()).toLower();
	//设置工作目录 
	QDir::setCurrent(mPalDir);
	return 0;
}

PalErr CGetPalData::EncodeRLE(const void* Source, const UINT8 TransparentColor, 
	INT32 Stride, INT32 Width, INT32 Height,void*& Destination,	INT32& Length)
{
	INT32 i, j, count;
	UINT32 length;
	UINT8* src = (UINT8*)Source;
	UINT8* temp;
	UINT8* ptr;

	if (Source == NULL)
		return EINVAL;
	if ((ptr = temp = (UINT8*)malloc(Width * Height * 2 + 4)) == NULL)
		return ENOMEM;

	for (i = 0, ptr = temp + 4; i < Height; i++)
	{
		for (j = 0; j < Width;)
		{
			for (count = 0; j < Width && *src == TransparentColor; j++, src++, count++);
			while (count > 0)
			{
				*ptr++ = (count > 0x7f) ? 0xff : 0x80 | count;
				count -= 0x7f;
			}
			for (count = 0; j < Width && *src != TransparentColor; j++, src++, count++);
			while (count > 0)
			{
				if (count > 0x7f)
				{
					*ptr++ = 0x7f;
					memcpy(ptr, src - count, 0x7f);
					ptr += 0x7f;
				}
				else
				{
					*ptr++ = count;
					memcpy(ptr, src - count, count);
					ptr += count;
				}
				count -= 0x7f;
			}
		}
		src += Stride - Width;
	}
	if ((ptr - temp) % 2) //smkf subfile must be even; need rle encoder archive it
		*ptr++ = 0;
	length = (UINT32)(ptr - temp);

	if ((Destination = realloc(temp, length)) == NULL)
	{
		free(temp);
		return ENOMEM;
	}
	*((UINT16*)Destination) = (UINT16)Width;
	*((UINT16*)Destination + 1) = (UINT16)Height;
	Length = length;
	return 0;
}

VOID CGetPalData::PAL_Object_classify(VOID)
{

	memset(&gpObject_classify, 0, sizeof(gpObject_classify));
	int maxObj = pal->gpGlobals->g_TextLib.nWords;

	while (pal->PAL_GetWord(maxObj - 1).empty()) maxObj--;
	assert(maxObj > 0);
	//标记我方角色
	for (int n = 0; n < MAX_PLAYER_ROLES; n++)
	{
		WORD player = (WORD)pal->gpGlobals->g.PlayerRoles.rgwName[n];
		gpObject_classify[player] = kIsPlayer;
	}

	//标记敌方
	nEnemy = 0;
	for (int n = 0; n < pal->gpGlobals->g.nEnemyTeam; n++)
	{
		for (int s = 0; s < MAX_ENEMIES_IN_TEAM; s++)
		{
			WORD enemy = (WORD)pal->gpGlobals->g.lprgEnemyTeam[n].rgwEnemy[s];
			if (enemy != 0xffff && enemy != 0 && enemy < maxObj)
			{
				std::string w = pal->PAL_GetWord(enemy);
				if (w.size() < 2)
					continue;
				if (!(gpObject_classify[enemy] & kIsEnemy))
				{
					gpObject_classify[enemy] |= kIsEnemy;
					nEnemy++;
				}
			}
		}
	}
	//标记召唤的敌人
	for (int n = 0; n < pal->gpGlobals->g.nScriptEntry; n++)
	{
		WORD wOperation = pal->gpGlobals->g.lprgScriptEntry[n].wOperation;
		if (wOperation == 0x009e || wOperation == 0x009f)
		{
			WORD enemy = pal->gpGlobals->g.lprgScriptEntry[n].rgwOperand[0];
			if (enemy && !(gpObject_classify[enemy] & kIsEnemy))
			{
				gpObject_classify[enemy] |= kIsEnemy;
				nEnemy++;
			}
		}
	}
	assert(nEnemy <= pal->gpGlobals->g.nEnemy);
	//标记毒
	nPoisonID = 0;
	for (int n = 0; n < pal->gpGlobals->g.nScriptEntry; n++)
	{
		WORD wOperation = pal->gpGlobals->g.lprgScriptEntry[n].wOperation;
		if (wOperation == 0x0028 || wOperation == 0x0029)
		{
			WORD  poisonID = pal->gpGlobals->g.lprgScriptEntry[n].rgwOperand[1];
			assert(poisonID < maxObj);
			LPWORD lpData;
			auto lpObj = pal->gpGlobals->g.rgObject;
			lpData = lpObj[poisonID].rgwData;

			if (lpData[3] || lpData[5])
				continue;

			if (!(gpObject_classify[poisonID] & kIsPoison))
			{
				nPoisonID++;
				gpObject_classify[poisonID] |= kIsPoison;
			}
		}
	}

	//标记物品
	nItem = 0;
	//标记获得物品
	for (int n = 0; n < pal->gpGlobals->g.nScriptEntry; n++)
	{
		if (pal->gpGlobals->g.lprgScriptEntry[n].wOperation == 0x001f
			&& pal->gpGlobals->g.lprgScriptEntry[n].rgwOperand[0])
		{
			WORD ItemID = pal->gpGlobals->g.lprgScriptEntry[n].rgwOperand[0];
			assert(ItemID && ItemID < maxObj);
			if (!(gpObject_classify[ItemID] & kIsItem))
			{
				nItem++;
				gpObject_classify[ItemID] |= kIsItem;
			}
		}
	}
	//标记商店物品
	for (int n = 0; n < pal->gpGlobals->g.nStore; n++)
	{
		for (int t = 0; t < MAX_STORE_ITEM; t++)
		{
			WORD ItemID = pal->gpGlobals->g.lprgStore[n].rgwItems[t];
			if (ItemID == 0)
				continue;
			assert(ItemID && ItemID < maxObj);
			if ((gpObject_classify[ItemID] ^ kIsItem))
			{
				nItem++;
				gpObject_classify[ItemID] |= kIsItem;
			}
		}
	}
	//标记装备物品
	for (int n = 0; n < MAX_PLAYER_ROLES; n++)
	{
		for (int t = 0; t < MAX_PLAYER_EQUIPMENTS; t++)
		{
			WORD ItemID = pal->gpGlobals->g.PlayerRoles.rgwEquipment[n][t];
			if (ItemID == 0)continue;
			assert(ItemID && ItemID < maxObj);
			if (ItemID && !(gpObject_classify[ItemID] & kIsItem))
			{
				nItem++;
				gpObject_classify[ItemID] |= kIsItem;
			}
		}
	}
	//标记可偷物品
	for (int n = 0; n < maxObj; n++)
	{
		if (gpObject_classify[n] != kIsEnemy)
			continue;
		WORD EnemyID;

		EnemyID = pal->gpGlobals->g.rgObject[n].enemy.wEnemyID;
		if (EnemyID > pal->gpGlobals->g.nEnemy)
			continue;
		assert(EnemyID < pal->gpGlobals->g.nEnemy);
		if (pal->gpGlobals->g.lprgEnemy[EnemyID].nStealItem == 0)
			continue;
		WORD ItemID = pal->gpGlobals->g.lprgEnemy[EnemyID].wStealItem;
		if (ItemID == 0)continue;
		assert(ItemID && ItemID < maxObj);
		if (ItemID && !(gpObject_classify[ItemID] & kIsItem))
		{
			nItem++;
			gpObject_classify[ItemID] |= kIsItem;
		}
	}

	//标记有标志的物品
	for (int n = 0; n < maxObj; n++)
	{
		WORD pp;
		//
		pp = pal->gpGlobals->g.rgObject[n].rgwData[6];
		if (pp && gpObject_classify[n] == 0)
		{
			nItem++;
			gpObject_classify[n] |= kIsItem;
		}
	}

	//assert(nItem == );

	//标记魔法
	nMagic = 0;
#if 1
	//标记脚本魔法
	for (int n = 0; n < maxObj; n++)
	{
		//OBJECT_MAGIC sMagic;
		auto flags = pal->gpGlobals->g.rgObject[n].magic.wFlags;
		if (!(gpObject_classify[n] & kIsMagic) && (flags == 2 || flags == 0xa || flags == 0x1a || flags == 0x12))
		{
			gpObject_classify[n] = kIsMagic;
			nMagic++;
		}

	}
#endif
	for (int n = 0; n < MAX_PLAYER_ROLES; n++)
	{
		for (int pMagic = 0; pMagic < MAX_PLAYER_MAGICS; pMagic++)
		{
			//标记初始魔法
			WORD sMagic = pal->gpGlobals->g.PlayerRoles.rgwMagic[pMagic][n];
			if (sMagic && sMagic < maxObj && !(gpObject_classify[sMagic] & kIsMagic))
			{
				gpObject_classify[sMagic] = kIsMagic;
				nMagic++;
			}
		}
		//标记合体魔法
		{
			WORD sMagic = pal->gpGlobals->g.PlayerRoles.rgwCooperativeMagic[n];
			if (sMagic && !(gpObject_classify[sMagic] & kIsMagic))
			{
				gpObject_classify[sMagic] = kIsMagic;
				nMagic++;
			}
		}
	}
	//标记升级魔法
	for (int n = 0; n < MAX_PLAYER_ROLES - 1; n++)
	{
		for (int pMagic = 0; pMagic < pal->gpGlobals->g.nLevelUpMagic; pMagic++)
		{
			if (pal->gpGlobals->g.lprgLevelUpMagic[pMagic].m[n].wLevel == 0)
				continue;
			WORD sMagic = pal->gpGlobals->g.lprgLevelUpMagic[pMagic].m[n].wMagic;
			if (sMagic && !(gpObject_classify[sMagic] & kIsMagic))
			{
				gpObject_classify[sMagic] = kIsMagic;
				nMagic++;
			}
		}
	}
	//标记获得魔法
	for (int n = 0; n < pal->gpGlobals->g.nScriptEntry; n++)
	{
		SCRIPTENTRY p = pal->gpGlobals->g.lprgScriptEntry[n];
		switch (p.wOperation)
		{
		case 0x0055:
		case 0x0056:
		case 0x0067:
			//case 0x0092:
		{
			WORD sMagic = p.rgwOperand[0];
			if (sMagic && sMagic < maxObj && !(gpObject_classify[sMagic] & kIsMagic))
			{
				gpObject_classify[sMagic] = kIsMagic;
				nMagic++;
			}
			break;
		}
		default:
			break;
		}
	}

	assert(nMagic <= pal->gpGlobals->g.nMagic);
	return;
}


//标记脚本跳转地址
//Mark The script jumps to the address
//输入 跳转地址引用
//返回下一跳地址，为0 结束，jumps 不为0返回三个跳转地址
INT CGetPalData::MarkSprictJumpsAddress(WORD spriptEntry, MAPScript& mMaps)
{
	while (TRUE)
	{
		MAPScript::iterator pw;
		pw = mMaps.find(spriptEntry);
		if (pw != mMaps.end())
			return 0;
		mMaps[spriptEntry] = spriptEntry;
		LPSCRIPTENTRY p = pal->gpGlobals->g.lprgScriptEntry.data() + spriptEntry;
		if (p->wOperation == 0)
			return 0;
		switch (p->wOperation)
		{
			//以下参数二跳转地址
		case 0x0006://条件跳转  参数1 概率  满足执行下一条，不满足跳转到参数2指向的地址
		case 0x001e://金钱改变，参数1 = 值，如参数1为且钱不足时，跳到参数2执行
		case 0x0024://设置对象自动脚本地址，参数1 不等于0 参数2 地址
		case 0x0025://设置对象触发脚本地址，参数1 不等于0 参数2 地址
		case 0x005d://如我方没有中特定的毒，则跳转  参数1 毒种类，参数2 跳转到
		case 0x005e://如敌方没有中特定的毒，则跳转  参数1 毒种类，参数2 跳转到
		case 0x0064://跳转，如敌方生命值高于 设定的百分比 ，参数1 比值，参数2 目标
		case 0x0079://如果指定成员在队伍中，则跳到参数2
		case 0x0090://设置对象脚本，参数1 对象，参数3 偏移，参数2 值
		case 0x0095://如果当前场景等于参数1，则跳转,参数2
		case 0x009c://怪物复制自身,失败跳转到 参数2
		case 0x0103://队伍中没有指定角色则跳转，参数1 角色，参数2 跳转目标
			MarkSprictJumpsAddress(p->rgwOperand[1], mMaps);
			break;
		case 0x0002://停止运行，用参数1替换，用于下一轮运行,条件是参数 2 = 0 或小于对象空闲帧数
		case 0x0003://无条件跳转到参数 1位置,参数2等于0和未达到参数2指定的帧数 运行下一条
		case 0x0004://运行 参数1 指向的子脚本
		case 0x000a://跳转到 参数1 指定地址，如玩家选择 No
		case 0x0033://收集妖怪炼丹,参数1 失败转到
		case 0x0034://灵壶炼丹指令,参数1 失败转到
		case 0x0038://将队伍从现场传送出去 失败跳到参数1 指向地址
		case 0x003a://将队伍从现场传送出去 失败跳到参数1 指向地址
		case 0x0061://没有中毒，跳转，参数1 跳转到
		case 0x0068://如敌方行动，跳转到参数1
		case 0x0074://如不是全体满血，跳到参数1
		case 0x0091://如果敌方不是孤单，则跳转 ，参数1 地址
		{
			if (p->wOperation == 0x0003 && p->rgwOperand[1] == 0)
			{//条件是参数 2 = 0
				MarkSprictJumpsAddress(p->rgwOperand[0], mMaps);
				return 0;
			}
			else
			{
				MarkSprictJumpsAddress(p->rgwOperand[0], mMaps);
			}
			break;
		}
		//以下参数一跳转地址
		case 0x0020://从库存中移除物品，参数1 品种，参数2 数量，如库存没有移除已装备物品，物品不足跳到参数3
		case 0x0028://敌方中毒指令 参数1 ！=0 全体，参数2 毒,失败跳转到参数3
		case 0x0029://我方中毒指令 参数1 ！=0 全体，参数2 毒,失败跳转到参数3
		case 0x002d://我方特殊状态指令，参数1 =0 疯、睡、定、封、傀儡、攻、防、身、双击，参数2 回合数。失败跳到参数3
		case 0x002e://敌方特殊状态指令，参数1 =0 疯、睡、定、傀儡、攻、防、身、双击，参数2 回合数，参数3 如不成功 跳转到参数3指向的地址。
		case 0x0058://如果库存数量不足则跳过，参数1 品种，参数2 要求数量，参数3 跳转到
		case 0x0081://跳过，如果没有面对对象，参数1 对象，参数2 改变触发方式，参数3地址
		case 0x0083://如果事件对象不在当前事件对象的指定区域，则跳过,参数1，参数3地址
		case 0x0084://将玩家用作事件对象的物品放置到场景中,参数1，参数2，失败跳到参数3
		case 0x0086://如果未装备指定物品则跳过，参数1 物品，参数3 地址
		case 0x0094://如果事件对象的状态是指定的，则跳转,参数2 状态，参数3 地址
		case 0x009e://怪物召唤，参数1 召唤的怪物ID，参数2 数量，参数3 失败跳转
			if (p->rgwOperand[2] != 0xffff)
				MarkSprictJumpsAddress(p->rgwOperand[2], mMaps);
			break;
			//以下双跳转地址2 和 3
		case 0x0007://开始战斗  参数1 队伍号， 参数3 为 0 BOSS 参数2 战斗失败执行 参数3 战斗中逃跑执行
		case 0x006d://设定一个场景的进入和传送脚本，参数1，场景号，参数2 进入脚本，参数3 传送脚本，参数2 和参数3同时为0 清除脚本
			MarkSprictJumpsAddress(p->rgwOperand[1], mMaps);
			MarkSprictJumpsAddress(p->rgwOperand[2], mMaps);
			break;
		default:
			break;
		}
		spriptEntry++;
	}
	return 0;
}

INT CGetPalData::PAL_MarkScriptAll()
{
	//auto& gConfig = gpGlobals->gConfig;
	{
		pMark.clear();;
		pMark.resize(static_cast<std::vector<sgMark, 
			std::allocator<sgMark>>::size_type>(pal->gpGlobals->g.nScriptEntry) + 1);
		//pMark.resize(gpGlobals->g.nScriptEntry + 1);

		for (int save = 0; save <= pal->gpGlobals->rgSaveData.size(); save++)
		{
			//0 = 系统缺省 1--save 存储文件
			if (save)
				if (pal->gpGlobals->rgSaveData.at(save - 1 ).size())
					pal->gpGlobals->PAL_LoadGame(
						pal->gpGlobals->rgSaveData.at(save - 1));
				else
					continue;
			// 
			//标记对象脚本
			for (int n = 0; n < pal->gpGlobals->g.nObject; n++)
			{
				LPWORD p{};
				int objectSize{};
				p = pal->gpGlobals->g.rgObject[n].rgwData;
				objectSize = 7;
				//对象结构，分win95 dos
				for (int j = 2; j < objectSize - 1; j++)
				{
					if (p[j])
					{
						gMARK t(n, save, 1, j);
						PAL_MarkScriptEntryAll(p[j], t, save);
					}
					//PAL_MarkScriptEntryAll(p[j], (save << 28) + (1 << 24) + (j << 16) + n, save);
				}
			}
			//第二步，标记场景信息和事件对象 
			for (WORD s = 1, k = 0; s <= MAX_SCENES; s++)
			{
				LPWORD p = (LPWORD)&pal->gpGlobals->g.rgScene[s - 1];
				//标记进入和传送脚本
				if (p[1])//进入脚本
				{
					gMARK t(s - 1, save, 2, 1);
					PAL_MarkScriptEntryAll(p[1], t, save);
				}
				//PAL_MarkScriptEntryAll(p[1], (save << 28) + (2 << 24) + (1 << 16) + s - 1, save);
				if (p[2])//传送脚本
				{
					gMARK t(s - 1, save, 2, 2);
					PAL_MarkScriptEntryAll(p[2], t, save);
				}
				//PAL_MarkScriptEntryAll(p[2], (save << 28) + (2 << 24) + (2 << 16) + s - 1, save);
				for (; k < pal->gpGlobals->g.nEventObject &&
					((s < MAX_SCENES && k < pal->gpGlobals->g.rgScene[s].wEventObjectIndex) ||
					(s == MAX_SCENES && k < pal->gpGlobals->g.nEventObject))
					; k++)
				{
					LPWORD q = (LPWORD)&pal->gpGlobals->g.lprgEventObject[k];
					for (int n = 4; n < 6; n++)
					{
						//4 5	
						if (q[n])
						{
							gMARK t(k, save, 3, n);
							PAL_MarkScriptEntryAll(q[n], t, save);
						}
						//PAL_MarkScriptEntryAll(q[n], (save << 28) + (3 << 24) + (n << 16) + k, save);
					}
				}
			}
		}
		//重新装入缺省数据
		pal->gpGlobals->PAL_LoadDefaultGame();
		return 0;
	}
}

//标记具体的有效脚本地址
//参数1 入口，参数2 要标记的脚本号列表
VOID CGetPalData::PAL_MarkScriptEntryAll(WORD Entry, const gMARK& mark, int save)
{
	if (Entry == 0)
		return;

	pMark[Entry].s.push_back(mark);//标记
	// 已经标记过 返回
	if (pMark[Entry].s.size() > 1)
		return;
	LPSCRIPTENTRY p = &pal->gpGlobals->g.lprgScriptEntry[Entry];
	switch (p->wOperation)
	{
	case 0:
		return;
		//以下参数二跳转地址
	case 0x0006://条件跳转  参数1 概率  满足执行下一条，不满足跳转到参数2指向的地址
	case 0x001e://金钱改变，参数1 = 值，如参数1为且钱不足时，跳到参数2执行
	case 0x0024://设置对象自动脚本地址，参数1 不等于0 参数2 地址
	case 0x0025://设置对象触发脚本地址，参数1 不等于0 参数2 地址
	case 0x005d://如我方没有中特定的毒，则跳转  参数1 毒种类，参数2 跳转到
	case 0x005e://如敌方没有中特定的毒，则跳转  参数1 毒种类，参数2 跳转到
	case 0x0064://跳转，如敌方生命值高于 设定的百分比 ，参数1 比值，参数2 目标
	case 0x0079://如果指定成员在队伍中，则跳到参数2
	case 0x0090://设置对象脚本，参数1 对象，参数3 偏移，参数2 值
	case 0x0095://如果当前场景等于参数1，则跳转,参数2
	case 0x009c://怪物复制自身,失败跳转到 参数2
	case 0x0103://队伍中没有指定角色则跳转，参数1 角色，参数2 跳转目标
	{
		if (p->rgwOperand[1])
		{
			{
				gMARK t(Entry, save, 4, 2);
				PAL_MarkScriptEntryAll(p->rgwOperand[1], t, save);
			}
			//PAL_MarkScriptEntryAll(p->rgwOperand[1], (save << 28) + (4 << 24) + (2 << 16) + Entry, save);
		}
		break;
	}
	case 0x0002://停止运行，用参数1替换，用于下一轮运行,条件是参数 2 = 0 或小于对象空闲帧数
	case 0x0003://无条件跳转到参数 1位置,参数2等于0和未达到参数2指定的帧数 运行下一条
		if (p->rgwOperand[1] == 0)
		{
			//到此结束
			{
				gMARK t(Entry, save, 4, 1);
				PAL_MarkScriptEntryAll(p->rgwOperand[0], t, save);
			}
			//PAL_MarkScriptEntryAll(p->rgwOperand[0], (save << 28) + (4 << 24) + (1 << 16) + Entry, save);
			return;
		}
		break;
	case 0x0004://运行 参数1 指向的子脚本
	case 0x0033://收集妖怪炼丹,参数1 失败转到
	case 0x000a://跳转到 参数1 指定地址，如玩家选择 No
	case 0x0034://灵壶炼丹指令,参数1 失败转到
	case 0x0038://将队伍从现场传送出去 失败跳到参数1 指向地址
	case 0x003a://将队伍从现场传送出去 失败跳到参数1 指向地址
	case 0x0061://没有中毒，跳转，参数1 跳转到
	case 0x0068://如敌方行动，跳转到参数1
	case 0x0074://如不是全体满血，跳到参数1
	case 0x0091://如果敌方不是孤单，则跳转 ，参数1 地址
		if (p->rgwOperand[0])
		{
			gMARK t(Entry, save, 4, 1);
			PAL_MarkScriptEntryAll(p->rgwOperand[0], t, save);
		}
		//PAL_MarkScriptEntryAll(p->rgwOperand[0], (save << 28) + (4 << 24) + (1 << 16) + Entry, save);
		break;
		//以下参数一跳转地址
	case 0x0020://从库存中移除物品，参数1 品种，参数2 数量，如库存没有移除已装备物品，物品不足跳到参数3
	case 0x0028://敌方中毒指令 参数1 ！=0 全体，参数2 毒,失败跳转到参数3
	case 0x0029://我方中毒指令 参数1 ！=0 全体，参数2 毒,失败跳转到参数3
	case 0x002d://我方特殊状态指令，参数1 =0 疯、睡、定、封、傀儡、攻、防、身、双击，参数2 回合数。失败跳到参数3
	case 0x002e://敌方特殊状态指令，参数1 =0 疯、睡、定、傀儡、攻、防、身、双击，参数2 回合数，参数3 如不成功 跳转到参数3指向的地址。
	case 0x0058://如果库存数量不足则跳过，参数1 品种，参数2 要求数量，参数3 跳转到
	case 0x0081://跳过，如果没有面对对象，参数1 对象，参数2 改变触发方式，参数3地址
	case 0x0083://如果事件对象不在当前事件对象的指定区域，则跳过,参数1，参数3地址
	case 0x0084://将玩家用作事件对象的物品放置到场景中,参数1，参数2，失败跳到参数3
	case 0x0086://如果未装备指定物品则跳过，参数1 物品，参数3 地址
	case 0x0094://如果事件对象的状态是指定的，则跳转,参数2 状态，参数3 地址
	case 0x009e://怪物召唤，参数1 召唤的怪物ID，参数2 数量，参数3 失败跳转
		if (p->rgwOperand[2] && p->rgwOperand[2] != 0xFFFF)
		{
			gMARK t(Entry, save, 4, 3);
			PAL_MarkScriptEntryAll(p->rgwOperand[2], t, save);
		}
		//PAL_MarkScriptEntryAll(p->rgwOperand[2], (save << 28) + (4 << 24) + (3 << 16) + Entry, save);
		break;
		//以下双跳转地址2 和 3
	case 0x0007://开始战斗  参数1 队伍号， 参数3 为 0 BOSS 参数2 战斗失败执行 参数3 战斗中逃跑执行
	case 0x006d://设定一个场景的进入和传送脚本，参数1，场景号，参数2 进入脚本，参数3 传送脚本，参数2 和参数3同时为0 清除脚本
		if (p->rgwOperand[1])
		{
			gMARK t(Entry, save, 4, 2);
			PAL_MarkScriptEntryAll(p->rgwOperand[1], t, save);
		}
		//PAL_MarkScriptEntryAll(p->rgwOperand[1], (save << 28) + (4 << 24) + (2 << 16) + Entry, save);
		if (p->rgwOperand[2])
		{
			gMARK t(Entry, save, 4, 3);
			PAL_MarkScriptEntryAll(p->rgwOperand[2], t, save);
		}
		//PAL_MarkScriptEntryAll(p->rgwOperand[2], (save << 28) + (4 << 24) + (3 << 16) + Entry, save);//标注位置为3
		break;
	case 0xffff://显示文字 参数1 文字地址 打印该文字
		//fprintf_s(f, "%s\n", p_Pal->PAL_GetMsg(p->rgwOperand[0]));
	default:
		break;
	}
	if (Entry + 1 >= pal->gpGlobals->g.nScriptEntry || pMark[Entry + 1].s.size())
	{
		return;
	}
	{
		gMARK t(Entry , save, 4, 0);
		PAL_MarkScriptEntryAll(Entry + 1, t, save);
	}
	//PAL_MarkScriptEntryAll(Entry + 1, (save << 28) + (4 << 24) + Entry, save);//进行下一行标注，标注位置为0，代表自然跳转
	return;
}

// 替换mkf文件中的一个片段，
int CGetPalData::replaceMKFOne(ByteArray* f, int nNum, LPCVOID buf, int BufLen)
{
	if (f->empty() || !buf || nNum < 0)
		return -1;
	uint32_t headlen = *(uint32_t*)f->data();
	int count = headlen / sizeof(uint32_t) - 1;
	ByteArray t;//临时缓存
	int scOff = headlen;
	if (nNum >= count)
	{
		//从尾部追加
		nNum = count;
		count++;
		headlen += sizeof(uint32_t);
	}

	t.resize(f->size() + BufLen * 2 + 4);
	memset(t.data(), 0, t.size());
	uint32_t* sc = (uint32_t*)f->data(), * sd = (uint32_t*)t.data();//sc 源 sd 目标
	int sdOff = sd[0] = headlen;//sdoff 目标偏移 ，scOff 源偏移
	for (int i = 0; i < count; i++)
	{
		assert(sd[i] == sdOff);
		int oldLen = sc[i + 1] - sc[i];

		if (i == nNum)
		{
			memcpy(t.data() + sdOff, buf, BufLen);
			sdOff += BufLen;
			scOff += oldLen;
		}
		else
		{
			memcpy(t.data() + sdOff, f->data() + scOff, oldLen);
			sdOff += oldLen;
			scOff += oldLen;
		}
		sd[i + 1] = sdOff;
	}
	t.resize(sdOff);
	f->swap(t);
	return 0;
}

//Single script changes
//脚本单行变动处理
// 输入：变动的脚本，原行号，新行号,第一项等于第二项？
//
PalErr CGetPalData::SingleScriptChange(int sOld, int sNew, MAPScript& st)
{
	if (sOld == sNew)
		return 0;
	auto& sgMark = pMark[sOld].s;
	if (sgMark.size() == 0)
		return 0;
	LPWORD obj{};

	for (int n = 0; n < sgMark.size(); n++)
	{
		auto& m = sgMark.at(n);
		INT save = m.save;// (m >> 28);//>0 存储文档号
		INT from = m.from;// (m >> 24) & 0xf;//1 = 对象 2 = 场景 3 = 事件对象 4 脚本
		INT col = m.col;  // (m >> 16) & 0xff;//所在列
		INT row = m.row;  //m & 0xffff;//所在行
		obj = nullptr;
		switch (from)
		{
		case 1:
			//对象
			if (save == 0)
				obj = &pal->gpGlobals->g.rgObject[row].rgwData[col];
			else
			{
				obj = pal->gpGlobals->getSaveFileObject(1, row, col, 
					pal->gpGlobals->rgSaveData.at(save - 1));
			}
			assert(*obj == sOld);
			break;
		case 2:
			//场景
			if (save == 0)
				obj = &((LPWORD)(&pal->gpGlobals->g.rgScene[row]))[col];
			else
			{
				obj = pal->gpGlobals->getSaveFileObject(2, row, col, 
					pal->gpGlobals->rgSaveData.at(save - 1 ));
			}
			assert(*obj == sOld);
			break;
		case 3:
			//事件对象
			if (save == 0)
				obj = &((LPWORD)(&pal->gpGlobals->g.lprgEventObject[row]))[col];
			else
			{
				obj = pal->gpGlobals->getSaveFileObject(3, row, col,
					pal->gpGlobals->rgSaveData.at(save - 1));
			}
			assert(*obj == sOld);
			break;
		case 4:
			//脚本
			if (save == 0)
			{
				if (col == 0)
				{
					//新行=旧行-1
					assert(row == sOld - 1);
					continue;
				}
				//???
				auto sFind = st.find(row);
				if (sFind == st.end())
				{
					return 1;//误将有效调用删除了
				}
                auto& scriptEntry = pal->gpGlobals->g.lprgScriptEntry[sFind->second];
				obj = &((LPWORD)( & scriptEntry))[col];
				assert(*obj == sOld);
			}
			else
				continue;
			break;
		default:
			assert(FALSE);
			return 1;
			break;
		}
		*obj = sNew;
	}
	return 0;
}



INT CGetPalData::isUseYJ_1(const ByteArray& fp)
{
	int count = pal->PAL_MKFGetChunkCount(fp);
	int j{};
	while ( j < count && (pal->PAL_MKFGetChunkSize(j, fp)) < 4) j++;
	if (j >= count)
	{
		//出错了 ，空的文件 
		return  -1;
	}
	
	auto data = pal->PAL_MKFReadChunk( j, fp);
	if (data[0] == 'Y' && data[1] == 'J' && data[2] == '_' && data[3] == '1')
		return 1;
	return 0;
}

void CGetPalData::doSaveMapTilesToArray(int iMapNum)
{
	//将地图数据存储到缓存
	auto buf = &pal->gpGlobals->f.fpMAP;
	auto& Tiles = m_pPalMap->MapTiles;
	assert(iMapNum == m_pPalMap->iMapNum);
	UINT32 len{};
	LPVOID s{};
	CPalData::EnCompress(Tiles, sizeof(Tiles), s, len, 0);
	replaceMKFOne(buf, iMapNum, s, len);
	free(s);
	//将fpMAP修改作出标记
	ModifRecord[modRoc_mapMkf] = TRUE;
}

VOID CGetPalData::saveGameDataToCache()
{
	//重新生成sss.mkf   和 date.mkf 的数据
	DWORD cLen[17]{}, zLen[17]{};
	DWORD j{}, c{ 6 * sizeof(DWORD) };
	auto p = &pal->gpGlobals->g;
	cLen[0] = zLen[0] = c;
	for (j = 1; j < 6; j++)
	{
		switch (j)
		{
		case 1://0+1
			cLen[j] = p->nEventObject * sizeof(EVENTOBJECT);
			break;
		case 2://
			cLen[j] = p->nScene * sizeof(SCENE);
			break;
		case 3://
			cLen[j] = p->nObject * (CPalEvent::ggConfig->fIsWIN95 ?
				sizeof(OBJECT) : sizeof(OBJECT_DOS));
			break;
		case 4:
			cLen[j] = (pal->gpGlobals->g_TextLib.nMsgs + 1) * sizeof(DWORD);
			//3 信息字段数
			break;
		case 5://
			cLen[j] = p->nScriptEntry * sizeof(SCRIPTENTRY);
			break;
		default:
			break;
		}
		c += cLen[j];
		zLen[j] = c;
	}
	memcpy(pal->gpGlobals->f.fpSSS.data(), zLen, sizeof(DWORD) * 6);
	if (pal->gpGlobals->f.fpSSS.size() != c)
		pal->gpGlobals->f.fpSSS.resize(c);

	for (j = 0; j < 5; j++)
	{
		c = cLen[j];
		switch (j)
		{
		case 0:
			memcpy(pal->gpGlobals->f.fpSSS.data() + zLen[j], p->lprgEventObject.data(), cLen[j + 1]);
			break;
		case 1:
			memcpy(pal->gpGlobals->f.fpSSS.data() + zLen[j], p->rgScene.data(), cLen[j + 1]);
			break;
		case 2:
			if (CPalEvent::ggConfig->fIsWIN95)
				memcpy(pal->gpGlobals->f.fpSSS.data() + zLen[j], p->rgObject.data(), cLen[j + 1]);
			else
			{
				std::vector<OBJECT_DOS>buf(p->nObject);
				for (int i = 0; i < p->nObject; i++)
				{
					memcpy(&buf[i], &p->rgObject[i], sizeof(OBJECT_DOS));
					buf[i].rgwData[5] = p->rgObject[i].rgwData[6];
				}
				memcpy(pal->gpGlobals->f.fpSSS.data() + zLen[j], buf.data(), cLen[j + 1]);
			}
			break;
		case 3:
			//3 信息字段数
			memcpy(pal->gpGlobals->f.fpSSS.data() + zLen[j],
				pal->gpGlobals->g_TextLib.lpMsgOffset.data(),
				cLen[j + 1]);
			break;
		case 4:
			memcpy(pal->gpGlobals->f.fpSSS.data() + zLen[j], p->lprgScriptEntry.data(), cLen[j + 1]);
			break;
		default:
			break;
		}
	}
	//assert(c == gpGlobals->f.fpSSS.size());
	//SSS.MKF建立完成
	c = cLen[0] = zLen[0] = 17 * sizeof(DWORD);

	for (j = 0; j < 16; j++)
	{
		switch (j)
		{
		case 0:
			cLen[j + 1] = sizeof(STORE) * p->nStore;
			break;
		case 1:
			cLen[j + 1] = sizeof(ENEMY) * p->nEnemy;
			break;
		case 2:
			cLen[j + 1] = sizeof(ENEMYTEAM) * p->nEnemyTeam;
			break;
		case 3:
			cLen[j + 1] = sizeof(PLAYERROLES);
			break;
		case 4:
			cLen[j + 1] = sizeof(MAGIC) * p->nMagic;
			break;
		case 5:
			cLen[j + 1] = sizeof(BATTLEFIELD) * p->nBattleField;
			break;
		case 6:
			cLen[j + 1] = sizeof(LEVELUPMAGIC_ALL) * p->nLevelUpMagic;
			break;
		case 7:
		case 8:
		case 9://gpSpriteUI
		case 10://g_Battle.lpEffectSprite
		case 12://g_TextLib.bufDialogIcons
			cLen[j + 1] = pal->gpGlobals->PAL_MKFGetChunkSize(j, pal->gpGlobals->f.fpDATA);
			break;
		case 11:
			cLen[j + 1] = sizeof(p->rgwBattleEffectIndex);
			break;
		case 13:
			cLen[j + 1] = sizeof(p->EnemyPos);
			break;
		case 14:
			cLen[j + 1] = sizeof(p->rgLevelUpExp);
			break;
		case 15:
			cLen[j + 1] = sizeof(pal->ggConfig->m_Function_Set) + pal->ggConfig->m_FontName.length() + 4;
			break;
		default:
			break;
		}
		c += cLen[j + 1];
		zLen[j + 1] = c;
	}
	ByteArray sBak = std::move(pal->gpGlobals->f.fpDATA);
	pal->gpGlobals->f.fpDATA.resize(c);
	memcpy(pal->gpGlobals->f.fpDATA.data(), zLen, sizeof(DWORD) * 17);
	for (j = 0; j < 16; j++)
	{
		c = cLen[j + 1];
		switch (j)
		{
		case 0:
			memcpy(pal->gpGlobals->f.fpDATA.data() + zLen[j], p->lprgStore.data(), c);
			break;
		case 1:
			memcpy(pal->gpGlobals->f.fpDATA.data() + zLen[j], p->lprgEnemy.data(), c);
			break;
		case 2:
			memcpy(pal->gpGlobals->f.fpDATA.data() + zLen[j], p->lprgEnemyTeam.data(), c);
			break;
		case 3:
			memcpy(pal->gpGlobals->f.fpDATA.data() + zLen[j], &p->PlayerRoles, c);
			break;
		case 4:
			memcpy(pal->gpGlobals->f.fpDATA.data() + zLen[j], p->lprgMagic.data(), c);
			break;
		case 5:
			memcpy(pal->gpGlobals->f.fpDATA.data() + zLen[j], p->lprgBattleField.data(), c);
			break;
		case 6:
			memcpy(pal->gpGlobals->f.fpDATA.data() + zLen[j], p->lprgLevelUpMagic.data(), c);
			break;
		case 7:
		case 8:
		case 9://gpSpriteUI
		case 10://g_Battle.lpEffectSprite
		case 12://g_TextLib.bufDialogIcons
		{
			LPBYTE pp = new BYTE[(c)];
			pal->gpGlobals->PAL_MKFReadChunk(pp, (INT&)c, j, sBak);
			memcpy(pal->gpGlobals->f.fpDATA.data() + zLen[j], pp, c);
			delete[] pp;
			break;
		}
		case 11:
			memcpy(pal->gpGlobals->f.fpDATA.data() + zLen[j], p->rgwBattleEffectIndex, c);
			break;
		case 13:
			memcpy(pal->gpGlobals->f.fpDATA.data() + zLen[j], &p->EnemyPos, c);
			break;
		case 14:
			memcpy(pal->gpGlobals->f.fpDATA.data() + zLen[j], p->rgLevelUpExp, c);
			break;
		case 15:
			
			memcpy(pal->gpGlobals->f.fpDATA.data() + zLen[j], pal->ggConfig->m_Function_Set, c);
			//拷贝字库目录
			memcpy(pal->gpGlobals->f.fpDATA.data() + zLen[j] + sizeof(pal->ggConfig->m_Function_Set),
				pal->ggConfig->m_FontName.c_str(), pal->ggConfig->m_FontName.size());
			break;
		default:
			break;
		}
	}
	//修改纪录
	ModifRecord[modRoc_dataMkf] = 1;
	ModifRecord[modRoc_sssMkf] = 1;
}


//初始化压缩标志
void CGetPalData::set_CompressFlag(int flag)
{
	if (flag == -1)
		CConfig::fisUSEYJ1DeCompress = pal->gpGlobals->is_Use_YJ1_Decompress();
	else
		CConfig::fisUSEYJ1DeCompress = flag;
}

VOID CGetPalData::Utf8ToSys(std::string& s)
{
	//去除尾部空格
	s.erase(s.find_last_not_of(" ") + 1);
	Cls_Iconv Istr{};
	//
	if (!pal->ggConfig->fIsUseBig5)
	{
		s = Istr.UTF8toGBK(s.c_str());
		//936 to 950
		if (pal->gpGlobals->bIsBig5)
			s = Istr.Gb2312ToBig5(s.c_str());
	}
	else
	{
		s = Istr.UTF8toGBK(s.c_str());
		//950 to 936
		if (!pal->gpGlobals->bIsBig5)
			s = Istr.Big5ToGb2312(s.c_str());
	}
}

PalErr CGetPalData::backupFile()
{
	//功能：备份数据文件,将数据文件备份到BAK目录下
	bool bi{ 1 };
	{
		//准备目录
		std::string dirBak = pal->PalDir + "BAK\\";
		if (!pal->gpGlobals->IsDirExist(dirBak))
			bi &= QDir().mkdir(dirBak.c_str());
	}

	for (int i = 0; PalFileName[i]; i++)
	{
		std::string dir = pal->PalDir + PalFileName[i];
		std::string dirbak = pal->PalDir + "BAK\\";
		dirbak += PalFileName[i];
		if (pal->gpGlobals->IsFileExist(dirbak))
			bi &= QFile::remove(dirbak.c_str());//删除旧的备份文件
		if (pal->gpGlobals->IsFileExist(dir))
			bi &= QFile(dir.c_str()).copy(dirbak.c_str());//复制文件
	}
	//备份存档文件
	for (int n = 0; n < pal->gpGlobals->rgSaveData.size(); n++)
	{
		std::string s;
		s += pal->va("%d.rpg", n + 1);
		std::string dir = pal->PalDir + s;
		std::string dirbak = pal->PalDir + "BAK\\";
		dirbak += s;
		if (pal->gpGlobals->IsFileExist(dirbak))
			bi &= QFile::remove(dirbak.c_str());//删除旧的备份文件
		if (pal->gpGlobals->IsFileExist(dir))
			bi &= QFile(dir.c_str()).copy(dirbak.c_str());//复制文件
	}
	return bi != true;
}

PalErr CGetPalData::restoreFile()
{
	//功能：恢复所有数据文件，
	bool bi{ 1 };
	if (!pal->gpGlobals->IsDirExist(pal->PalDir + "BAK/"))
		return 1;
	for (int i = 0; PalFileName[i]; i++)
	{
		std::string dir = pal->PalDir + PalFileName[i];
		std::string dirbak = pal->PalDir + "BAK/";
		dirbak += PalFileName[i];
		if (CPalData::IsFileExist(dirbak))
		{
			bi &= QFile(dir.c_str()).remove();;
			bi &= QFile(dirbak.c_str()).copy(dir.c_str());
		}
	}

	//存档文件
	for (int n = 0; n < MAXSAVEFILES; n++)
	{
		std::string s = pal->va("%d.rpg", n + 1 - 
			CPalEvent::ggConfig->m_Function_Set[53]);
		std::string dir = pal->PalDir + s;
		std::string dirbak = pal->PalDir + "BAK/";
		dirbak += s;
		if (CPalData::IsFileExist(dir))
			bi &= QFile(dir.c_str()).remove();
		if (CPalData::IsFileExist(dirbak))
			bi &= QFile(dirbak.c_str()).copy(dir.c_str());
	}
	return bi == false;
}

PalErr CGetPalData::delBackupFile()
{
	bool bi{ 1 };
	if (!pal->gpGlobals->IsDirExist(pal->PalDir + "BAK/"))
		return 0;
	for (int i = 0; PalFileName[i]; i++)
	{
		std::string dirbak = pal->PalDir + "BAK/";
		dirbak += PalFileName[i];
		if (CPalData::IsFileExist(dirbak))
		{
			bi &= QFile(dirbak.c_str()).remove();
		}
	}

	//存档文件
	for (int n = 0; n < MAXSAVEFILES; n++)
	{
		std::string s = pal->va("%d.rpg", n + 1 - 
			CPalEvent::ggConfig->m_Function_Set[53]);
		std::string dirbak = pal->PalDir + "BAK/";
		dirbak += s;
		if (CPalData::IsFileExist(dirbak))
			bi &= QFile(dirbak.c_str()).remove();
	}
	//删除备份目录
	std::string olddir = pal->PalDir + "BAK/";
	bi &= QDir().rmdir(olddir.c_str());
	return bi == false;
}

PalErr CGetPalData::saveDataFile()
{
	int err{};
	//保存数据文件
	for (int i = 0; i < modRoc_End; i++)
	{
		if (!ModifRecord[i])
			continue;
		std::string s;
		ByteArray* pfp{};
		switch (i)
		{
		case	modRoc_abcMkf:
			s = "abc.mkf";
			pfp = &pal->gpGlobals->f.fpABC;
			break;
		case	modRoc_ballMkf:
			s = "ball.mkf";
			pfp = &pal->gpGlobals->f.fpBALL;
			break;
		case	modRoc_dataMkf:
			s = "data.mkf";
			pfp = &pal->gpGlobals->f.fpDATA;
			break;
		case	modRoc_fMkf:
			s = "f.mkf";
			pfp = &pal->gpGlobals->f.fpF;
			break;
		case	modRoc_fbpMkf:
			s = "fbp.mkf";
			pfp = &pal->gpGlobals->f.fpFBP;
			break;
		case	modRoc_fireMkf:
			s = "fire.mkf";
			pfp = &pal->gpGlobals->f.fpFIRE;
			break;
		case	modRoc_gopMkf:
			s = "gop.mkf";
			pfp = &pal->gpGlobals->f.fpGOP;
			break;
		case	modRoc_mapMkf:
			s = "map.mkf";
			pfp = &pal->gpGlobals->f.fpMAP;
			break;
		case	modRoc_mgoMkf:
			s = "mgo.mkf";
			pfp = &pal->gpGlobals->f.fpMGO;
			break;
		case	modRoc_rgmMkf:
			s = "rgm.mkf";
			pfp = &pal->gpGlobals->f.fpRGM;
			break;
		case	modRoc_sssMkf:
			s = "sss.mkf";
			pfp = &pal->gpGlobals->f.fpSSS;
			break;
		case modRoc_mMsg:
			s = "m.msg";
			pfp = &pal->gpGlobals->f.fpMsg;
			break;
		case modRoc_wordDat:
			s = "word.dat";
			pfp = &pal->gpGlobals->f.fpWord;
			break;
		case modRoc_descDat:
			s = "desc.dat";
			pfp = &pal->gpGlobals->f.fpDesc;
			break;
		default:
			return 1;
			break;
		}
        std::string dir = pal->gpGlobals->PalDir + s;
		std::string dirold = dir + ".old";
		if (QFile::rename(dir.c_str(),dirold.c_str() ))
		{
			FILE* fp{};
			if (fopen_s(&fp, dir.c_str(), "wb"))
			{
				err = 1;
				continue;
			}
			auto len = pfp->size();
			fwrite(pfp->data(), len, 1, fp);
			fclose(fp);
		}
		QFile(dirold.c_str()).remove();
		ModifRecord[i] = 0;
	}
	//存档文件
	int maxSaveFile = CPalEvent::ggConfig->m_Function_Set[47] + 
		CPalEvent::ggConfig->m_Function_Set[53];
	for (int i = 0; i <maxSaveFile; i++)
	{
		if (pal->gpGlobals->rgSaveData[i].empty())
			continue;
		auto fileName = pal->gpGlobals->PalDir + pal->va("%d.rpg", i + 1
		- CPalEvent::ggConfig->m_Function_Set[53]);
		FILE* fp{};
		if (fopen_s(&fp, fileName.c_str(), "wb"))
		{
			err = 1;
			continue;
		}
		//auto len = gpGlobals->rgSaveData[i].size();
		auto len = pal->gpGlobals->getSaveFileLen();
		fwrite(pal->gpGlobals->rgSaveData.at(i).data(), len, 1, fp);
		fclose(fp);
	}
	return err;
}

//调试选项
QStringList debug_items = {
	"无敌模式",
	"速杀模式",
	"禁止普通脚本",
	"显示装备脚本",
	"显示自动脚本",
	"标志对象位置",
	"显示简洁脚本",
	"穿越障碍模式",
};


testRun::testRun(CPalData* p, QWidget* para)
	:QDialog(para), p_PalData(p) {
	setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
	CScript::isTestRun = true;
	m_sList.setParent(this);
	m_sEdit.setParent(this);
	m_sEdit.setFocusPolicy(Qt::NoFocus);//没有焦点
	timer = new QTimer(this);
	connect(timer, &QTimer::timeout, [&]() {
		while (!CScript::m_ScriptMessages.empty())
		{
			m_sEdit.moveCursor(QTextCursor::End);
			m_sEdit.append(CScript::m_ScriptMessages.front().c_str());
			CScript::m_ScriptMessages.pop_front();
			m_sEdit.moveCursor(QTextCursor::End);
			m_sEdit.show();
		}
		if (!CPalEvent::isTestRun)
		{
			close();
		}
		});
	timer->start(20);
	QTimer::singleShot(10,[this]() {
        auto threadRun = std::thread([this]() {
            runTest();
        });
		threadRun.detach();//分离线程，线程结束后自动释放资源
		});
	{
		// 获取主屏幕
		QScreen* screen = QGuiApplication::primaryScreen();
		QRect screenGeometry = screen->geometry();
		// 设置窗口大小和位置
		int width = screenGeometry.width();
		int height = screenGeometry.height();
		resize((int)(width * 0.46), (int)(height * 0.7));
		move((int)width * 0.52, (int)(height * 0.08));
		setWindowTitle("调试信息");
	}
	//建立列表
	m_sList.setSpacing(m_sList.height() / 20);
	foreach(const QString & item, debug_items) {
		QListWidgetItem* listItem = new QListWidgetItem(item, &m_sList);
		listItem->setCheckState(Qt::Unchecked); // 初始未选中
		m_sList.addItem(listItem);
	}
	connect(&m_sList, &QListWidget::itemChanged, this, &testRun::onListItemChanged);
}

testRun::~testRun()
{
	delete timer;
}

void testRun::keyPressEvent(QKeyEvent* event) {
		event->ignore();  // 忽略所有键，所属窗口仍可操作
}

void testRun::onListItemChanged(QListWidgetItem* item) {
	for (int i = 0; i < m_sList.count(); i++) {
		auto s = item->text();
		if (s == debug_items[i]) {
			if (item->checkState() == Qt::Checked) {
				CPalEvent::debugSwitch[i] = TRUE;
			}
			else
			{
				CPalEvent::debugSwitch[i] = FALSE;
			}
			return;
		}
	}
}


void testRun::resizeEvent(QResizeEvent* event)
{
	QDialog::resizeEvent(event);
	int cx = event->size().width();
	int cy = event->size().height();
	m_sList.resize(200, cy - 8);
	m_sList.move(4, 4);
	m_sEdit.resize(cx - 208, cy - 8);
	m_sEdit.move(200, 4);
}

void testRun::closeEvent(QCloseEvent* event)
{
	if (CScript::isTestRun)
	{
		event->ignore();//忽略关闭信号，阻止窗体关闭
	}
	else
		QDialog::closeEvent(event);
}


void testRun::runTest() {
	CScript::isTestRun = true;//生成调试信息
	CScript::m_ScriptMessages.clear();

	auto m = new CScript(0, 0, p_PalData);//运行
	for (int i = 0; i < p_PalData->rgSaveData.size(); i++)
	{
		p_PalData->rgSaveData[i] =	std::move( m->gpGlobals->rgSaveData.at(i));
		p_PalData->rgSaveDataChaged[i] = m->gpGlobals->rgSaveDataChaged[i];
	}
	delete m;
	m = nullptr;
	CScript::isTestRun = false;
}
