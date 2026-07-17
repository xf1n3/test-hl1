#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"

LINK_ENTITY_TO_CLASS(weapon_mycoldweapon, CMyColdWeapon);

void CMyColdWeapon::Spawn()
{
    Precache();
    m_iId = WEAPON_MYCOLDWEAPON;
    SET_MODEL(ENT(pev), "models/w_mycoldweapon.mdl");
    m_iClip = -1;

    FallInit();
}

void CMyColdWeapon::Precache()
{
    PRECACHE_MODEL("models/v_mycoldweapon.mdl");
    PRECACHE_MODEL("models/p_mycoldweapon.mdl");
    PRECACHE_MODEL("models/w_mycoldweapon.mdl");

    // звуки от лома - оставляем без изменений
    PRECACHE_SOUND("weapons/cbar_hit1.wav");
    PRECACHE_SOUND("weapons/cbar_hit2.wav");
    PRECACHE_SOUND("weapons/cbar_hitbod1.wav");
    PRECACHE_SOUND("weapons/cbar_hitbod2.wav");
    PRECACHE_SOUND("weapons/cbar_hitbod3.wav");
    PRECACHE_SOUND("weapons/cbar_miss1.wav");

    m_usMyColdWeapon = PRECACHE_EVENT(1, "events/mycoldweapon.sc");
}

int CMyColdWeapon::AddToPlayer(CBasePlayer *pPlayer)
{
    if (CBasePlayerWeapon::AddToPlayer(pPlayer))
    {
        MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
            WRITE_BYTE(m_iId);
        MESSAGE_END();
        return TRUE;
    }
    return FALSE;
}

int CMyColdWeapon::GetItemInfo(ItemInfo *p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = NULL;
    p->iMaxAmmo1 = -1;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WEAPON_NOCLIP;
    p->iSlot = 0;
    p->iPosition = 3;
    p->iId = WEAPON_MYCOLDWEAPON;
    p->iWeight = MYCOLDWEAPON_WEIGHT;
    return 1;
}

BOOL CMyColdWeapon::Deploy()
{
    return DefaultDeploy("models/v_mycoldweapon.mdl", "models/p_mycoldweapon.mdl", MYCOLDWEAPON_DRAW, "crowbar");
}

void CMyColdWeapon::Holster(int skiplocal)
{
    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
    SendWeaponAnim(MYCOLDWEAPON_HOLSTER);
}

void CMyColdWeapon::PrimaryAttack()
{
    if (!Swing(1))
    {
        SetThink(&CMyColdWeapon::SwingAgain);
        pev->nextthink = gpGlobals->time + 0.1;
    }
}

void CMyColdWeapon::Smack()
{
    DecalGunshot(&m_trHit, BULLET_PLAYER_CROWBAR);
}

void CMyColdWeapon::SwingAgain()
{
    Swing(0);
}

int CMyColdWeapon::Swing(int fFirst)
{
    int fDidHit = FALSE;

    TraceResult tr;

    UTIL_MakeVectors(m_pPlayer->pev->v_angle);
    Vector vecSrc = m_pPlayer->GetGunPosition();
    Vector vecEnd = vecSrc + gpGlobals->v_forward * 32;

    UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

    if (tr.flFraction >= 1.0)
    {
        UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);
        if (tr.flFraction < 1.0)
        {
            CBaseEntity *pHit = CBaseEntity::Instance(tr.pHit);
            if (!pHit || pHit->IsBSPModel())
                FindHullIntersection(vecSrc, tr, Vector(-16,-16,-18), Vector(16,16,18), m_pPlayer->edict());
            vecEnd = tr.vecEndPos;
        }
    }

    if (tr.flFraction >= 1.0)
    {
        if (fFirst)
        {
            SendWeaponAnim(MYCOLDWEAPON_MISS1);
            m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
            m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
            m_iSwing = TRUE;
        }
    }
    else
    {
        switch (((m_iSwing++) % 2) + 1)
        {
        case 0: SendWeaponAnim(MYCOLDWEAPON_ATTACK1HIT); break;
        case 1: SendWeaponAnim(MYCOLDWEAPON_ATTACK2HIT); break;
        case 2: SendWeaponAnim(MYCOLDWEAPON_ATTACK3HIT); break;
        }

        m_pPlayer->m_iWeaponVolume = NORMAL_ANIM_VOLUME;

        CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);
        ClearMultiDamage();

        if ((m_flNextPrimaryAttack + 1 < UTIL_WeaponTimeBase()) || g_pGameRules->IsMultiplayer())
        {
            pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgMyColdWeapon * 3, gpGlobals->v_forward, &tr, DMG_CLUB);
        }
        else
        {
            pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgMyColdWeapon, gpGlobals->v_forward, &tr, DMG_CLUB);
        }
        ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

        m_flNextPrimaryAttack = GetNextAttackDelay(0.25);

        if (!g_pGameRules->IsMultiplayer())
        {
            m_iSwing = 0;
        }

        m_trHit = tr;

        // звук удара - от лома
        if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
        {
            switch (RANDOM_LONG(0,2))
            {
            case 0: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hitbod1.wav", 1, ATTN_NORM); break;
            case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hitbod2.wav", 1, ATTN_NORM); break;
            case 2: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hitbod3.wav", 1, ATTN_NORM); break;
            }
        }
        else
        {
            EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, RANDOM_LONG(0,1) ? "weapons/cbar_hit1.wav" : "weapons/cbar_hit2.wav", 1, ATTN_NORM);
        }

        m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.25;
        fDidHit = TRUE;
    }

    return fDidHit;
}
