# Search

	PASSWD_MAX_LEN			= 16,

# Add after

#ifdef ENABLE_NEW_PET_SYSTEM
	PET_MAX_LEVEL_CONST = 120,
	PET_MAX_EVOLUTION_CONST = 3,
	PET_MAX_SKILL_LEVEL_CONST = 20,
	PET_HATCH_COST = 100000,
	PET_APPLY_MAX_NUM = 3,
	PET_SKILLS_MAX_NUM = 3,
#endif

# Search

	WEAR_COSTUME_WEAPON,

# Add after

#ifdef ENABLE_NEW_PET_SYSTEM
	WEAR_PET,
#endif

# Search

enum ECharType

# add in end

#ifdef ENABLE_NEW_PET_SYSTEM
	CHAR_TYPE_PET,
#endif
