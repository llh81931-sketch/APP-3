// 失控进化手游 (com.tencent.rmcn) - 偏移矩阵集中定义官网:泽峰.top    更多公益开源资源分享 \n TG@ASDK886 QQ1063167389\n各大源码免费分享 网络搜集 打破信息差    官方频道:https://t.me/+FDfQgP9RFdljYjFl
// 引擎: Unity 2022 (il2cpp) + libunity.so
#pragma once

// =====================================================================
// 失控进化 · 当前对接链（相对 libunity.so:bss[1]）
// 包名: com.tencent.rmcn
//
// 世界数组:
//   libunity.so:bss[1] +0x79be8 +0x668 +0x4c8 -0x3f8 +0x80 +0x70 +0x78
// 矩阵:
//   libunity.so:bss    +0xABB80 +0x40 +0x10 +0x110   (末级为字段偏移)
// 人物数组:
//   libunity.so:bss[1] +0x8cd40 +0x128 +0x4d0 +0x410 +0x5f0 +0x718 +0x10 +0x18 +0x0
//
// 实现见 game_esp.h :: ResolveChains / ModuleBssIndex
// =====================================================================
#define RMC_CHAIN_WORLD_0   0x79be8
#define RMC_CHAIN_WORLD_1   0x668
#define RMC_CHAIN_WORLD_2   0x4c8
#define RMC_CHAIN_WORLD_3   0x3f8   /* 减法 */
#define RMC_CHAIN_WORLD_4   0x80
#define RMC_CHAIN_WORLD_5   0x70
#define RMC_CHAIN_WORLD_6   0x78

#define RMC_CHAIN_MATRIX_0  0xABB80
#define RMC_CHAIN_MATRIX_1  0x40
#define RMC_CHAIN_MATRIX_2  0x10
#define RMC_CHAIN_MATRIX_3  0x110  /* 字段偏移，不再解引用 */

#define RMC_CHAIN_ARRAY_0   0x8cd40
#define RMC_CHAIN_ARRAY_1   0x128
#define RMC_CHAIN_ARRAY_2   0x4d0
#define RMC_CHAIN_ARRAY_3   0x410
#define RMC_CHAIN_ARRAY_4   0x5f0
#define RMC_CHAIN_ARRAY_5   0x718
#define RMC_CHAIN_ARRAY_6   0x10
#define RMC_CHAIN_ARRAY_7   0x18
#define RMC_CHAIN_ARRAY_8   0x0


// ============ 基址 ============官网:泽峰.top    更多公益开源资源分享 \n TG@ASDK886 QQ1063167389\n各大源码免费分享 网络搜集 打破信息差    官方频道:https://t.me/+FDfQgP9RFdljYjFl
#define RMC_GWORLD_ADDR         0x7286842000  // 全局世界对象(本次运行)
#define RMC_LIBIL2CPP_BASE      0x7155023000   // libil2cpp.so加载基址
#define RMC_LIBUNITY_BASE       0x7177671000   // libunity.so加载基址

// ============ 世界对象链 ============
// GameManager静态全局管理器
#define RMC_GM_CLIENT            0x8     // GameManager.client(静态)
#define RMC_GM_SERVER            0x10    // GameManager.server(静态)
#define RMC_GM_GENERIC           0x18    // GameManager.generic(静态)
#define RMC_GM_CLIENTSIDE        0x20    // GameManager.Clientside(bool)
#define RMC_GM_SERVERSIDE         0x21    // GameManager.Serverside(bool)
// EntityBase基类字段
#define RMC_ENT_IS_NET_CREATED   0x28    // EntityBase.IsCreatedFromNet(bool)
#define RMC_ENT_ENTITY_ID         0x30    // EntityBase.EntityId(Int64)
#define RMC_ENT_IS_MANAGED        0x38    // EntityBase.IsManaged(bool)
#define RMC_ENT_COMPONENTS        0x40    // EntityBase.components(数组)
#define RMC_ENT_LOD_LEVEL         0x48    // EntityBase.lodLevel(Int32)
#define RMC_ENT_MID               0x50    // EntityBase.Mid(Int64)
#define RMC_ENT_IS_FROM_POOL      0x5a    // EntityBase.IsFromPool(bool)

// ============ 网格/骨骼 ============
// BaseRustEntity模型字段
#define RMC_ENTITY_GO             0x98    // BaseRustEntity.EntityGo(GameObject)
#define RMC_MODEL                 0xc8    // BaseRustEntity.model(Model)
#define RMC_PARENT_BONE           0xd0    // BaseRustEntity.parentBone(uint32)
#define RMC_BOUNDS                 0xa0    // BaseRustEntity.bounds(Bounds)
// Bone(RootMotionFinalIK)
#define RMC_BONE_LENGTH           0x54    // Bone.length(float)
#define RMC_BONE_SQR_MAG          0x58    // Bone.sqrMag(float)
#define RMC_BONE_AXIS             0x5c    // Bone.axis(Vector3)
#define RMC_BONE_ROT_LIMIT        0x68    // Bone._rotationLimit(指针)
#define RMC_BONE_IS_LIMITED       0x70    // Bone.isLimited(bool)

// ============ 玩家基础信息 ============
//完整版懒得发，小幸开源首发，完全免费，请勿被圈（本人为实测，但是应该有效）
#define RMC_PLAYER_CURRENT_TEAM   0x10    // BasePlayer.currentTeam(uint64)
#define RMC_PLAYER_MOUNTED        0x28    // BasePlayer.mounted(BaseMountable指针)
#define RMC_PLAYER_IS_MOUNTED     0x30    // BasePlayer.isMounted(bool)
#define RMC_PLAYER_SV_ITEM_ID     0x38    // BasePlayer.svActiveItemID(uint32)
#define RMC_PLAYER_VIEW_ANGLES    0x68    // BasePlayer.viewAngles(Vector3)
#define RMC_PLAYER_UID             0xd8    // BasePlayer.userID(uint64)
#define RMC_PLAYER_UID_STR        0xe0    // BasePlayer.UserIDString(string)
#define RMC_PLAYER_GAMEMODE_TEAM  0xe8    // BasePlayer.gamemodeteam(int32)
#define RMC_PLAYER_REPUTATION      0xec    // BasePlayer.reputation(int32)
#define RMC_PLAYER_DISPLAY_NAME   0xf0    // BasePlayer._displayName(string)
// BaseCombatEntity
#define RMC_COMBAT_BASE_PROTECT   0xf8    // BaseCombatEntity.baseProtection(ProtectionProperties)
#define RMC_COMBAT_START_HP       0x100   // BaseCombatEntity.startHealth(float)
#define RMC_COMBAT_LIFESTATE      0x108   // BaseCombatEntity.lifestate(LifeState)
#define RMC_COMBAT_HEALTH          0x110   // BaseCombatEntity._health(float)
#define RMC_COMBAT_MAX_HEALTH     0x114   // BaseCombatEntity._maxHealth(float)
#define RMC_COMBAT_FACTION         0x118   // BaseCombatEntity.faction(Faction)
#define RMC_COMBAT_LAST_ATK_TIME  0x11c   // BaseCombatEntity.lastAttackedTime(float)
#define RMC_COMBAT_LAST_ATK_DIR   0x120   // BaseCombatEntity.LastAttackedDir(Vector3)
#define RMC_COMBAT_LAST_DAMAGE     0x134   // BaseCombatEntity.lastDamage(DamageType)
#define RMC_COMBAT_LAST_ATTACKER   0x138   // BaseCombatEntity.lastAttacker(指针)
#define RMC_COMBAT_UNHOSTILE_TIME  0x14c   // BaseCombatEntity.unHostileTime(float)
// BaseRustEntity
#define RMC_RUST_CREATOR_ENTITY   0x78    // BaseRustEntity.creatorEntity(指针)
#define RMC_RUST_ENTITY            0x90    // BaseRustEntity.Entity(IEntity)
#define RMC_RUST_HAS_BRAIN         0xe0    // BaseRustEntity.HasBrain(bool)
#define RMC_RUST_NAME               0xe8    // BaseRustEntity._name(string)
#define RMC_RUST_OWNER_ID           0xf0    // BaseRustEntity.OwnerID(uint64)
#define RMC_RUST_SKIN_ID            0xd8    // BaseRustEntity.skinID(uint64)
// PlayerEntity
#define RMC_PE_POS_X                0x5c    // PlayerEntity._PosX_Smooth(float)
#define RMC_PE_POS_Y                0x60    // PlayerEntity._PosY_Smooth(float)
#define RMC_PE_POS_Z                0x64    // PlayerEntity._PosZ_Smooth(float)
#define RMC_PE_ROT_X                 0x90    // PlayerEntity.RotateX_Smooth(float)
#define RMC_PE_ROT_Y                 0x94    // PlayerEntity.RotateY_Smooth(float)
#define RMC_PE_ROT_Z                 0x98    // PlayerEntity.RotateZ_Smooth(float)
#define RMC_PE_HIT_ANIM_START       0x78    // PlayerEntity.hitAnimStartTime(int64)
#define RMC_PE_LAST_UPDATE_TS       0xa8    // PlayerEntity.LastUpdatedTs(int64)
#define RMC_PE_AMMO_IN_BAG          0xc8    // PlayerEntity.ammoInBag(int32)
#define RMC_PE_WEARED_ITEMS         0xd8    // PlayerEntity.WearedItems(List)
#define RMC_PE_LOCAL_LOGIC_STATE    0xf0    // PlayerEntity.LocalLogicState(int64)
#define RMC_PE_HELD_ITEM_SWITCH     0x108   // PlayerEntity.HeldItemSwitchSeq(int32)
#define RMC_PE_IS_IN_FIRE           0x179   // PlayerEntity.IsInFire(bool)
#define RMC_PE_REBORNED_TIME        0x198   // PlayerEntity.RebornedTime(int64)
#define RMC_PE_LAST_ATK_TIME        0x210   // PlayerEntity.LastAttackedTime(int64)
#define RMC_PE_LAST_ATK_PLAYER_ID   0x218   // PlayerEntity.LastAttackPlayerId(int64)
#define RMC_PE_ARMOR_PROTECT        0x228   // PlayerEntity.ArmorProtection(float[])
#define RMC_PE_NEXT_WEAPON_ID       0x280   // PlayerEntity.NextWeaponId(int64)
#define RMC_PE_PREV_WEAPON_TBL_ID   0x288   // PlayerEntity.PreviousWeaponTableId(int64)
#define RMC_PE_DEFAULT_SCENE_FOV    0x298   // PlayerEntity.defaultSceneFov(float)
#define RMC_PE_GUN_FOV              0x2a4   // PlayerEntity.GunFov(float)
#define RMC_PE_B_LOCAL_PLAYER       0x2c0   // PlayerEntity.bLocalPlayer(bool)
#define RMC_PE_IS_AUTO_FIRE         0x2d4   // PlayerEntity.IsAutoFire(bool)
#define RMC_PE_CAN_AUTO_FIRE        0x2d5   // PlayerEntity.CanAutoFire(bool)
#define RMC_PE_TEMPLATE_ID          0x328   // PlayerEntity.TemplateId(int64)
#define RMC_PE_LAST_IS_HOSTILE      0x330   // PlayerEntity.lastIsHostile(bool)
#define RMC_PE_IS_IN_AIR            0x350   // PlayerEntity.IsInAir(bool)
#define RMC_PE_FIRE_SCALE           0x358   // PlayerEntity.FireScale(float)
#define RMC_PE_CTRL_ID              0x470   // PlayerEntity.CtrlId(int64)
#define RMC_PE_CUR_FOCUS_ENT_ID     0x488   // PlayerEntity.CurrentFocusedEntityId(int64)
#define RMC_PE_HIT_SOURCE_ROLE_ID   0x4e8   // PlayerEntity.HitSourceRoleId(uint64)
#define RMC_PE_IS_INSIDE            0x500   // PlayerEntity.IsInside(bool)
#define RMC_PE_IS_PATROL_HOSTILE    0x520   // PlayerEntity.IsPatrolHostile(bool)
#define RMC_PE_MELEE_ATK_REQ        0x528   // PlayerEntity.MeleeAttackRequest(结构体)
#define RMC_PE_E_WEAPON_COLOR      0x574   // PlayerEntity.eWeaponColor(int32)
#define RMC_PE_AIM_YAW              0x58c   // PlayerEntity.AimYaw(float)
#define RMC_PE_LOGIC_FRAME_SEQ      0x630   // PlayerEntity.LogicFrameSequence(int64)
#define RMC_PE_FP_ANIM_INDEX        0x638   // PlayerEntity.FpAnimIndex(WeaponUnique)
#define RMC_PE_IS_MAGIC_BULLET      0x91c   // PlayerEntity.IsMagicBullet(bool)
#define RMC_PE_HIT_SRC_ENTITY_ID    0x920   // PlayerEntity.HitSourceEntityId(int64)
#define RMC_PE_CUR_LOCKED_ENT_ID    0x1ee0  // PlayerEntity.CurrentLockedEntityId(int64)
#define RMC_PE_ADD_SPEED            0x2028  // PlayerEntity.AddSpeed(Vector3)

// ============ 自身多级指针 ============
// 自身PlayerEntity通过GameManager.client静态获取
#define RMC_SELF_LOCAL_PLAYER      0x18    // PlayerEntity.MyPlayerClient(静态)
#define RMC_SELF_STATE_OFFSET_MAP  0x38    // PlayerEntity.StateOffsetMap(静态)

// ============ 击杀信息 ============
// HitInfo命中信息结构体
#define RMC_HIT_INITIATOR          0x10    // HitInfo.Initiator(BaseRustEntity)
#define RMC_HIT_WEAPON_PREFAB      0x18    // HitInfo.WeaponPrefab(BaseRustEntity)
#define RMC_HIT_DID_HIT            0x24    // HitInfo.DidHit(bool)
#define RMC_HIT_ENTITY              0x28    // HitInfo.HitEntity(BaseRustEntity)
#define RMC_HIT_BONE                0x30    // HitInfo.HitBone(uint32)
#define RMC_HIT_PART                0x34    // HitInfo.HitPart(uint32)
#define RMC_HIT_MATERIAL            0x38    // HitInfo.HitMaterial(uint32)
#define RMC_HIT_POS_WORLD           0x3c    // HitInfo.HitPositionWorld(Vector3)
#define RMC_HIT_POS_LOCAL           0x48    // HitInfo.HitPositionLocal(Vector3)
#define RMC_HIT_NORMAL_WORLD        0x54    // HitInfo.HitNormalWorld(Vector3)
#define RMC_HIT_NORMAL_LOCAL        0x60    // HitInfo.HitNormalLocal(Vector3)
#define RMC_HIT_POINT_START         0x6c    // HitInfo.PointStart(Vector3)
#define RMC_HIT_POINT_END           0x78    // HitInfo.PointEnd(Vector3)
#define RMC_HIT_PROJECTILE_ID       0x84    // HitInfo.ProjectileID(int32)
#define RMC_HIT_PROJECTILE_HITS     0x88    // HitInfo.ProjectileHits(int32)
#define RMC_HIT_PROJ_DISTANCE       0x8c    // HitInfo.ProjectileDistance(float)
#define RMC_HIT_PROJ_TRAVEL_TIME    0x94    // HitInfo.ProjectileTravelTime(float)
#define RMC_HIT_PROJ_VELOCITY       0x9c    // HitInfo.ProjectileVelocity(Vector3)
#define RMC_HIT_DAMAGE_TYPES        0xb0    // HitInfo.damageTypes(DamageTypeList)
#define RMC_HIT_GATHER_SCALE        0xbc    // HitInfo.gatherScale(float)

// ============ 载具 ============
// BaseVehicle
//完整版懒得发，小幸开源首发完全免费，请勿被圈（本人暂未实测，但是应该有效）
#define RMC_VEH_RENDERER_GO         0x218   // BaseVehicle.RendererGo(GameObject)
#define RMC_VEH_GO                  0x220   // BaseVehicle.VehicleGo(BaseVehicleGo)
#define RMC_VEH_FUEL_COMP           0x228   // BaseVehicle.BaseFuelComponent(指针)
#define RMC_VEH_IS_AUTHORITY        0x230   // BaseVehicle.IsAuthority(bool)
#define RMC_VEH_COLLISION_SPD_TH    0x260   // BaseVehicle.CollisionEffectSpeedThreshOld(float)
#define RMC_VEH_CLIENT_POS_X        0x2ac   // BaseVehicle.ClientPosX(float)
#define RMC_VEH_CLIENT_POS_Y        0x2b0   // BaseVehicle.ClientPosY(float)
#define RMC_VEH_CLIENT_POS_Z        0x2b4   // BaseVehicle.ClientPosZ(float)
#define RMC_VEH_RIGIDBODY           0x2b8   // BaseVehicle.rigidBody(Rigidbody)
#define RMC_VEH_MOUNT_POINTS        0x2c0   // BaseVehicle.mountPoints(List)
#define RMC_VEH_DRIVER_SEAT_IDX     0x2e0   // BaseVehicle.DriverSeatIndexes
#define RMC_VEH_PASSENGER_SEAT_IDX  0x2e8   // BaseVehicle.PassengerSeatIndexes
#define RMC_VEH_SAFE_AREA_RADIUS    0x328   // BaseVehicle.safeAreaRadius(float)
#define RMC_VEH_SAFE_AREA_ORIGIN    0x32c   // BaseVehicle.safeAreaOrigin(float)
#define RMC_VEH_SPAWN_TIME          0x338   // BaseVehicle.spawnTime(float)
// VehicleEntity
#define RMC_VE_POS_X                 0xd4    // VehicleEntity.PosX_Smooth(float)
#define RMC_VE_POS_Y                 0xd8    // VehicleEntity.PosY_Smooth(float)
#define RMC_VE_POS_Z                 0xdc    // VehicleEntity.PosZ_Smooth(float)
#define RMC_VE_ROT_X                 0xe0    // VehicleEntity.RotateX_Smooth(float)
#define RMC_VE_ROT_Y                 0xe4    // VehicleEntity.RotateY_Smooth(float)
#define RMC_VE_ROT_Z                 0xe8    // VehicleEntity.RotateZ_Smooth(float)
#define RMC_VE_PARENT_ID             0xf0    // VehicleEntity.ParentId(int64)
#define RMC_VE_MOUNT_ID              0xf8    // VehicleEntity.MountID(int64)
#define RMC_VE_NEXT_FIRE_CD_FINISH   0x104   // VehicleEntity.NextFireCDFinishTime(float)
#define RMC_VE_LAST_COPTER_HP        0x108   // VehicleEntity.LastCopterHp(float)
#define RMC_VE_CHILDREN_LIST         0x110   // VehicleEntity.childrenList(HashSet)
#define RMC_VE_FIRST_TAKE_PLAYER_ID  0x118   // VehicleEntity.firstTakeInPlayerId(int64)

// ============ 准星 ============
// CrossHairController十字线控制器
#define RMC_CH_AIM_STYLE_DIC         0x10    // CrossHairController.aimStyleDic(字典)
#define RMC_CH_AIM_NODE              0x18    // CrossHairController.aimNode(字典)
#define RMC_CH_AIM_DATA              0x20    // CrossHairController.aimData(AimData)
#define RMC_CH_AIM_ASSET             0x28    // CrossHairController.aimAsset(AimAsset)
#define RMC_CH_CUR_AIM_STYLE         0x30    // CrossHairController.curAimStyle(AimStyle)
#define RMC_CH_CUR_ALPHA              0x40    // CrossHairController.curAlpha(float)

// ============ 开火/开镜 ============
//完整版懒得发，小幸开源首发（本人暂未实测，但是应该有效）
#define RMC_IS_FIRING                0x179   // PlayerEntity.IsInFire(bool)
#define RMC_IS_ADS                    0x158   // WeaponCustom.IsInAds(bool)
#define RMC_IS_AUTO_FIRE              0x2d4   // PlayerEntity.IsAutoFire(bool)
#define RMC_CAN_AUTO_FIRE             0x2d5   // PlayerEntity.CanAutoFire(bool)
#define RMC_IS_MOVING_VEHICLE         0x159   // WeaponCustom.IsInMovingVehicle(bool)
#define RMC_WARMUP_STATE              0x188   // WeaponCustom.warmupState(EWarmupState)
#define RMC_LAST_SHOOT_TIME           0x13c   // WeaponCustom.LastShootTime(float)

// ============ 投掷物 ============
// BulletEntity子弹实体相关
#define RMC_THROW_PARENT_ID           0x78    // BulletEntity.ParentId(int64)
#define RMC_THROW_MOUNT_ID           0x80    // BulletEntity.MountID(int64)
#define RMC_GRENADE_FUSE              0xd0    // BulletEntity.FuzeDuration(int32,毫秒)
#define RMC_THROW_WEAPON_ID           0x110   // BulletEntity.ThrowWeaponId(int64)
#define RMC_THROW_POWER_VELOCITY      0x8c    // BulletEntity.PowerForVelocity(float)
#define RMC_THROW_ACCELERATION        0x90    // BulletEntity.Acceleration(Vector3)
#define RMC_THROW_MOVE_TIME           0xa8    // BulletEntity.MoveTime(float)
#define RMC_THROW_OWNER_TABLE_ID     0xa0    // BulletEntity.OwnerTableID(int64)
#define RMC_THROW_WEAPON_TABLE_ID     0xc8    // BulletEntity.WeaponTableID(int64)
#define RMC_THROW_BULLET_INDEX        0xd4    // BulletEntity.BulletIndex(int32)
#define RMC_THROW_MAGIC_BULLET        0xd8    // BulletEntity.MagicBullet(bool)
#define RMC_THROW_IS_SERVER          0xe0    // BulletEntity.IsServer(bool)
#define RMC_THROW_SOURCE_POS          0xe4    // BulletEntity.SourceEntityPosition(Vector3)
#define RMC_THROW_ADS_PROGRESS        0xf0    // BulletEntity.AdsProgress(float)
#define RMC_THROW_TOTAL_MOVE_DIST     0x10c   // BulletEntity.TotalMoveDistance(float)
#define RMC_THROW_TARGET_HISTORY      0x120   // BulletEntity.targetHistory(HashSet)
#define RMC_THROW_PREDICT_HITS        0x128   // BulletEntity.preditcHits(List)

// ============ 手持武器 ============
// WeaponCustom武器定制类
//完整版懒得发，小幸开源首发，完全免费，请勿被圈（本人暂未实测，但是应该有效）
#define RMC_WC_IS_WARMUP_LOCAL        0x38    // WeaponCustom.IsWarmupWeapon_Local(bool)
#define RMC_WC_CUR_SCOPE_LEVEL        0x3c    // WeaponCustom.curScopeLevel(EScopeLevel)
#define RMC_WC_FAKE                    0x40    // WeaponCustom.fake(bool)
#define RMC_WC_SHOTS_FIRED             0x48    // WeaponCustom.shotsFired(int32)
#define RMC_WC_VIEW_KICK_PITCH         0x58    // WeaponCustom.ViewKickPitch(float)
#define RMC_WC_VIEW_KICK_YAW           0x5c    // WeaponCustom.ViewKickYaw(float)
#define RMC_WC_SPREAD                  0x80    // WeaponCustom.spread(float)
#define RMC_WC_MOVE_SPREAD             0x84    // WeaponCustom.moveSpread(float)
#define RMC_WC_SHOOT_SPREAD            0x88    // WeaponCustom.shootSpread(float)
#define RMC_WC_ACCURACY                0x90    // WeaponCustom.accuracy(float)
#define RMC_WC_USING_AMMO_NODE_ID     0x98    // WeaponCustom.UsingAmmoNodeId(int64)
#define RMC_WC_BULLET_SPEED            0x138   // WeaponCustom.BulletSpeed(float)
#define RMC_WC_IS_CLIENT                0x164   // WeaponCustom.IsClient(bool)
#define RMC_WC_AIM_TYPE                 0xe0    // WeaponCustom.AimType(int32)
#define RMC_WC_BULLET_RELOAD_TIME      0x130   // WeaponCustom.BulletReloadTime(float)
#define RMC_WC_START_RELOAD_TIME       0x134   // WeaponCustom.StartReloadTime(float)
#define RMC_WC_SHOOT_ATK_DAMAGE         0xb0    // WeaponCustom.ShootAttackDamage(float)
// PlayerEntity武器相关字段
#define RMC_WEAPON_NEXT_ID             0x280   // PlayerEntity.NextWeaponId(int64)
#define RMC_WEAPON_PREV_TBL_ID         0x288   // PlayerEntity.PreviousWeaponTableId(int64)
#define RMC_WEAPON_HELD_SWITCH_SEQ     0x108   // PlayerEntity.HeldItemSwitchSeq(int32)
#define RMC_WEAPON_AMMO_IN_BAG         0xc8    // PlayerEntity.ammoInBag(int32)
#define RMC_WEAPON_E_COLOR              0x574   // PlayerEntity.eWeaponColor(int32)
#define RMC_WEAPON_FP_ANIM_INDEX       0x638   // PlayerEntity.FpAnimIndex(WeaponUnique)

// ============ 掩体/摄像机 ============
// BaseCameraState相机状态基类
#define RMC_CAM_SCENE_GO               0x10    // BaseCameraState.SceneCameraGo(GameObject)
#define RMC_CAM_SCENE_CAM              0x18    // BaseCameraState.SceneCamera(Camera)
#define RMC_CAM_SCENE_TRANSFORM        0x20    // BaseCameraState.SceneCameraTransform(Transform)
#define RMC_CAM_IS_DAMPING              0x28    // BaseCameraState.IsDamping(bool)
#define RMC_CAM_BLEND_RULE              0x2c    // BaseCameraState.BlendRule(int32)
#define RMC_CAM_ADD_ANIM_POS            0x30    // BaseCameraState.AddAnimationCameraPos(Vector3)
#define RMC_CAM_SHAKE_WEIGHT            0x68    // BaseCameraState.CameraShakeWeight(float)
#define RMC_CAM_IS_MY_PLAYER_DEAD       0x70    // BaseCameraState.IsMyPlayerDead(bool)
#define RMC_CAM_KILLER_SRC_POS           0x74    // BaseCameraState.KillerSourcePos(float)
#define RMC_CAM_DEAD_OFFSET_XZ          0x80    // BaseCameraState.DeadCameraOffsetXZ(float)
#define RMC_CAM_DEAD_OFFSET_Y           0x84    // BaseCameraState.DeadCameraOffsetY(float)
// PlayerEntity相机相关字段
#define RMC_PLAYER_DEFAULT_FOV          0x298   // PlayerEntity.defaultSceneFov(float)
#define RMC_PLAYER_GUN_FOV              0x2a4   // PlayerEntity.GunFov(float)
#define RMC_PLAYER_LAND_CAM_SHAKE       0x10c   // PlayerEntity.LandCameraShakeScale(float)

// ============ 背包物资 ============
// ItemContainerNode物品容器节点
//完整版懒得发，小幸开源首发（本人暂未实测，但是应该有效）
#define RMC_BP_CONFIG                  0x30    // ItemContainerNode.config(配置)
#define RMC_BP_CONTAINER_CLIENT        0x38    // ItemContainerNode.containerClient(指针)
#define RMC_BP_ACCEPT_CHECK_FUNC       0x40    // ItemContainerNode.OnAcceptCheckFunc(委托)
// DirectoryItemNode目录物品节点
#define RMC_BP_DIR_CONTAINER           0x28    // DirectoryItemNode.containerClient(指针)
#define RMC_BP_DIR_CUSTOM_FUNC         0x30    // DirectoryItemNode._getCustomNodeFunc(委托)
// PlayerEntity背包相关字段
#define RMC_BP_AMMO_IN_BAG              0xc8    // PlayerEntity.ammoInBag(int32)
#define RMC_BP_WEARED_ITEMS             0xd8    // PlayerEntity.WearedItems(List)

// ============ 装备槽 ============
// Equipment装备类
#define RMC_EQUIP_ID                   0x10    // Equipment.Id(int64)
#define RMC_EQUIP_AFFECT_HITBOX        0x18    // Equipment.AffectHitbox(int32[])
#define RMC_EQUIP_BELONG_POS            0x20    // Equipment.BelongPosition(int32)
#define RMC_EQUIP_OCCUPY_POS            0x28    // Equipment.OccupyPosition(int32[])
#define RMC_EQUIP_DEFENSE_ID            0x30    // Equipment.DefenseId(int32)
#define RMC_EQUIP_SPEED_REDUCTION       0x44    // Equipment.SpeedReduction(float)
#define RMC_EQUIP_SWIM_SPEED_REDUCTION  0x48    // Equipment.SwimspeedReduction(float)
#define RMC_EQUIP_ATTACH_POINT         0x50    // Equipment.AttachmentPoint(string)
// PlayerEntity装备相关字段
#define RMC_EQUIP_ARMOR_PROTECT        0x228   // PlayerEntity.ArmorProtection(float[])
#define RMC_EQUIP_ON_MOUNT_PROTECT     0x230   // PlayerEntity.OnMountProtection(float[])
#define RMC_EQUIP_BUFF_PROTECT          0x238   // PlayerEntity.BuffProtection(float[])
#define RMC_EQUIP_LOCAL_EQUIP_VERSION   0x240   // PlayerEntity.LocalEquipVersion(int32)

// ============ 通用物品字段 ============
// Item物品基类
#define RMC_ITEM_CONDITION              0x10    // Item.condition(float)
#define RMC_ITEM_MAX_CONDITION          0x14    // Item.maxCondition(float)
#define RMC_ITEM_INFO                    0x18    // Item.info(ItemInfo)
#define RMC_ITEM_POSITION                0x20    // Item.position(int32)
// BulletEntity子弹物品字段
#define RMC_ITEM_BULLET_SPEED            0x138   // WeaponCustom.BulletSpeed(float)
#define RMC_ITEM_DAMAGE_SCALE            0xc0    // BulletEntity.DamageScale(float)
#define RMC_ITEM_WEAPON_TABLE_ID        0xc8    // BulletEntity.WeaponTableID(int64)
#define RMC_ITEM_OWNER_TABLE_ID         0xa0    // BulletEntity.OwnerTableID(int64)

// ============ 容器(箱子/抽屉) ============
// ItemContainerNode容器节点
#define RMC_CONTAINER_CONFIG             0x30    // ItemContainerNode.config
#define RMC_CONTAINER_CLIENT              0x38    // ItemContainerNode.containerClient
#define RMC_CONTAINER_ACCEPT_FUNC        0x40    // ItemContainerNode.OnAcceptCheckFunc
// DirectoryItemNode目录节点
#define RMC_CONTAINER_DIR_CLIENT         0x28    // DirectoryItemNode.containerClient
#define RMC_CONTAINER_DIR_FUNC           0x30    // DirectoryItemNode._getCustomNodeFunc
// ============ 空投 ============
// 使用BoxEntity偏移
#define RMC_AIRDROP_ATTACHED_TRANSFORM  0x60    // BoxEntity.AttachedTransform
#define RMC_AIRDROP_POS_X                0x68    // BoxEntity.PosX_Smooth
#define RMC_AIRDROP_POS_Y                0x6c    // BoxEntity.PosY_Smooth
#define RMC_AIRDROP_POS_Z                0x70    // BoxEntity.PosZ_Smooth
#define RMC_AIRDROP_ROT_X                0x74    // BoxEntity.RotateX_Smooth
#define RMC_AIRDROP_ROT_Y                0x78    // BoxEntity.RotateY_Smooth
#define RMC_AIRDROP_ROT_Z                0x7c    // BoxEntity.RotateZ_Smooth
#define RMC_AIRDROP_MOUNT_ID             0x80    // BoxEntity.MountID

// ============ 子弹 ============
// BulletEntity子弹实体(完整字段)
#define RMC_BULLET_POS_X                 0x5c    // BulletEntity.PosX_Smooth(float)
#define RMC_BULLET_POS_Y                 0x60    // BulletEntity.PosY_Smooth(float)
#define RMC_BULLET_POS_Z                 0x64    // BulletEntity.PosZ_Smooth(float)
#define RMC_BULLET_ROT_X                 0x68    // BulletEntity.RotateX_Smooth(float)
#define RMC_BULLET_ROT_Y                 0x6c    // BulletEntity.RotateY_Smooth(float)
#define RMC_BULLET_ROT_Z                 0x70    // BulletEntity.RotateZ_Smooth(float)
#define RMC_BULLET_PARENT_ID             0x78    // BulletEntity.ParentId(int64)
#define RMC_BULLET_MOUNT_ID              0x80    // BulletEntity.MountID(int64)
#define RMC_BULLET_POWER_VELOCITY        0x8c    // BulletEntity.PowerForVelocity(float)
#define RMC_BULLET_ACCELERATION          0x90    // BulletEntity.Acceleration(Vector3)
#define RMC_BULLET_OWNER_TABLE_ID        0xa0    // BulletEntity.OwnerTableID(int64)
#define RMC_BULLET_MOVE_TIME             0xa8    // BulletEntity.MoveTime(float)
#define RMC_BULLET_DAMAGE_SCALE          0xc0    // BulletEntity.DamageScale(float)
#define RMC_BULLET_WEAPON_TABLE_ID       0xc8    // BulletEntity.WeaponTableID(int64)
#define RMC_BULLET_FUZE_DURATION         0xd0    // BulletEntity.FuzeDuration(int32)
#define RMC_BULLET_BULLET_INDEX          0xd4    // BulletEntity.BulletIndex(int32)
#define RMC_BULLET_MAGIC_BULLET          0xd8    // BulletEntity.MagicBullet(bool)
#define RMC_BULLET_IS_SERVER              0xe0    // BulletEntity.IsServer(bool)
#define RMC_BULLET_SOURCE_POS             0xe4    // BulletEntity.SourceEntityPosition(Vector3)
#define RMC_BULLET_ADS_PROGRESS          0xf0    // BulletEntity.AdsProgress(float)
#define RMC_BULLET_TOTAL_MOVE_DIST       0x10c   // BulletEntity.TotalMoveDistance(float)
#define RMC_BULLET_THROW_WEAPON_ID       0x110   // BulletEntity.ThrowWeaponId(int64)
#define RMC_BULLET_TARGET_HISTORY        0x120   // BulletEntity.targetHistory(HashSet)
#define RMC_BULLET_PREDICT_HITS          0x128   // BulletEntity.preditcHits(List)
//完整版懒得发，小幸开源首发，完全免费，请勿被圈官网:泽峰.top    更多公益开源资源分享 \n TG@ASDK886 QQ1063167389\n各大源码免费分享 网络搜集 打破信息差    官方频道:https://t.me/+FDfQgP9RFdljYjFl