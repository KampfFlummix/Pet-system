# Add

if app.ENABLE_NEW_PET_SYSTEM:
	import item
	import localeInfo
	def getInjectCheck(text):
		words = ["SELECT", "TRUNCATE", "INSERT", "REPLACE", "DELETE", "ALTER", "DROP",";", ":", "*", "'", '"', "="]
		characters = []
		for word in words:
			if text.find(word) != -1:
				return True
		return False
	_interface_instance = None
	def GetInterfaceInstance():
		global _interface_instance
		return _interface_instance
	def SetInterfaceInstance(instance):
		global _interface_instance
		if _interface_instance:
			del _interface_instance
		_interface_instance = instance

	PET_BONUS_MAX_LEVEL = 20.0
	PET_SKILL_MAX_LEVEL = 20.0

	pet_bonus_types=[item.APPLY_MAX_HP,item.APPLY_ATTBONUS_MONSTER,item.APPLY_CRITICAL_PCT]
	pet_bonus_value=[4000,20,10]

	evoleItems = [\
		[ [55004,10],[27992,10],[27993,10], [27994,10],[51501,10], [50513,10]],\
		[ [55005,10],[71129,10],[71123,10], [70031,1],[51501,10], [50513,10]],\
		[ [55006,10],[31006,10],[31005,10], [30610,10],[51501,10], [50513,10]]]

	pet_skill_data = [\
		["", 0, 0],\
		[localeInfo.PET_SKIL_1, item.APPLY_ATTBONUS_STONE, 10],\
		[localeInfo.PET_SKIL_2, item.APPLY_ATTBONUS_BOSS, 10],\
		[localeInfo.PET_SKIL_3, [item.APPLY_ATTBONUS_UNDEAD,item.APPLY_ATTBONUS_DEVIL], 15],\
		[localeInfo.PET_SKIL_4, item.APPLY_ATTBONUS_HUMAN, 5],\
		[localeInfo.PET_SKIL_5, item.APPLY_CAST_SPEED, 15],\
		[localeInfo.PET_SKIL_6, item.APPLY_ATTBONUS_MONSTER, 10],\
		[localeInfo.PET_SKIL_7, item.APPLY_STR, 10],\
		[localeInfo.PET_SKIL_8, item.APPLY_INT, 10],\
		[localeInfo.PET_SKIL_9, item.APPLY_DEX, 10],\
		[localeInfo.PET_SKIL_10, item.APPLY_SKILL_DAMAGE_BONUS, 5],\
		[localeInfo.PET_SKIL_11, item.APPLY_NORMAL_HIT_DAMAGE_BONUS, 10],\
		[localeInfo.PET_SKIL_12, item.APPLY_ANTI_CRITICAL_PCT, 10],\
		[localeInfo.PET_SKIL_13, item.APPLY_ANTI_PENETRATE_PCT, 10],\
		[localeInfo.PET_SKIL_14, item.APPLY_RESIST_HUMAN, 5],\
		[localeInfo.PET_SKIL_15, item.APPLY_BLOCK, 10],\
		[localeInfo.PET_SKIL_16, item.APPLY_ATTBONUS_BOSS, 10],\
		[localeInfo.PET_SKIL_17, [item.APPLY_RESIST_ICE,item.APPLY_RESIST_EARTH,item.APPLY_RESIST_DARK], 10],\
		[localeInfo.PET_SKIL_18, item.APPLY_NORMAL_HIT_DEFEND_BONUS, 10],\
		[localeInfo.PET_SKIL_19, item.APPLY_SKILL_DEFEND_BONUS, 5],\
		[localeInfo.PET_SKIL_20, [item.APPLY_EXP_DOUBLE_BONUS,item.APPLY_ITEM_DROP_BONUS,item.APPLY_GOLD_DOUBLE_BONUS], 20],\
		[localeInfo.PET_SKIL_21, item.APPLY_POISON_REDUCE, 10],\
		[localeInfo.PET_SKIL_22, item.APPLY_CON, 10],\
	]
	exp_table = [
		0,
		100,
		150,
		260,
		380,
		600,
		1300,
		3300,
		5700,
		8700,
		12800,
		18000,
		25000,
		36000,
		52000,
		73000,
		100000,
		125000,
		160000,
		220000,
		280000,
		370000,
		540000,
		670000,
		880000,
		1000000,
		1237000,
		1418000,
		1624000,
		1857000,
		2122000,
		2421000,
		2761000,
		3145000,
		3580000,
		4073000,
		4632000,
		5194000,
		5717000,
		6264000,
		6837000,
		7600000,
		8274000,
		8990000,
		9753000,
		10560000,
		11410000,
		12320000,
		13270000,
		14280000,
		15340000,
		16870000,
		18960000,
		19980000,
		21420000,
		22930000,
		24530000,
		26200000,
		27960000,
		29800000,
		32780000,
		36060000,
		39670000,
		43640000,
		48000000,
		52800000,
		58080000,
		63890000,
		70280000,
		77310000,
		85040000,
		93540000,
		102900000,
		113200000,
		124500000,
		137000000,
		150700000,
		165700000,
		236990000,
		260650000,
		286780000,
		315380000,
		346970000,
		381680000,
		419770000,
		461760000,
		508040000,
		558740000,
		614640000,
		676130000,
		743730000,
		1041222000,
		1145344200,
		1259878620,
		1385866482,
		1524453130,
		1676898443,
		1844588288,
		2029047116,
		2100000000,
		2100000000,
		2100000000,
		2100000000,
		2100000000,
		2100000000,
		2100000000,
		2100000000,
		2100000000,
		2100000000,
		2100000000,
		2100000000,
		2100000000,
		2100000000,
		2100000000,
		2100000000,
		2100000000,
		2100000000,
		2100000000,
		2100000000,
		2100000000,
		2100000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000,
		2500000000
	]