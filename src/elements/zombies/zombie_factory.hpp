#pragma once
#include "game_param.hpp" // ZombieType codes + COUNT_ZOMBIE_TYPE

/*
Spawn-factory manifest (one row per ZombieType, IN ENUM ORDER -- the
static_asserts in zombie_type.cpp reject a missing, duplicated or
mis-ordered row at compile time, replacing the runtime exit(1234) on an
unregistered type as the primary guard):

    X(TYPE, ConcreteZombieClass)

Adding a zombie: write the class, append its row AFTER the enum entry in
game_param.hpp gets its place, done -- init_zombie picks it up with no
switch to edit. YETI_TYPE stays the appended last enum entry (see the
note in game_param.hpp), so its row stays last here.
The row order mirrors ZOMBIE_DEFS in level/zombies_registry.hpp (kept in
sync by the enum codes; that table is the name/key registry, this one the
construction registry).
*/
#define ZOMBIE_FACTORY_X(X) \
    X(NORMAL_TYPE, NormalZombie) \
    X(FLAG_TYPE, FlagZombie) \
    X(CONE_TYPE, ConeZombie) \
    X(BUCKET_TYPE, BucketZombie) \
    X(DOOR_TYPE, DoorZombie) \
    X(POLEVAULT_TYPE, PoleVaultingZombie) \
    X(NEWSPAPER_TYPE, NewspaperZombie) \
    X(FOOTBALL_TYPE, FootballZombie) \
    X(DANCER_TYPE, DancingZombie) \
    X(BACKUP_TYPE, BackupDancerZombie) \
    X(DUCKYTUBE_TYPE, DuckyTubeZombie) \
    X(SNORKEL_TYPE, SnorkelZombie) \
    X(ZOMBONI_TYPE, ZomboniZombie) \
    X(BOBSLED_TYPE, BobsledZombie) \
    X(DOLPHINRIDER_TYPE, DolphinRiderZombie) \
    X(JACKBOX_TYPE, JackboxZombie) \
    X(BALLOON_TYPE, BalloonZombie) \
    X(DIGGER_TYPE, DiggerZombie) \
    X(POGO_TYPE, PogoZombie) \
    X(BUNGEE_TYPE, BungeeZombie) \
    X(LADDER_TYPE, LadderZombie) \
    X(CATAPULT_TYPE, CatapultZombie) \
    X(GARGANTUAR_TYPE, GargantuarZombie) \
    X(IMP_TYPE, ImpZombie) \
    X(ZOMBOSS_TYPE, BossZombie) \
    X(YETI_TYPE, YetiZombie)
