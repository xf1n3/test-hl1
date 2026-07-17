#ifndef MYCOLDWEAPON_H
#define MYCOLDWEAPON_H

class CMyColdWeapon : public CBasePlayerWeapon
{
public:
    void Spawn() override;
    void Precache() override;
    int iItemSlot() override { return 1; }
    int GetItemInfo(ItemInfo *p) override;
    int AddToPlayer(CBasePlayer *pPlayer) override;

    void PrimaryAttack() override;
    void SecondaryAttack() override {}
    BOOL Deploy() override;
    void Holster(int skiplocal = 0) override;
    void SwingAgain();
    void Smack();
    int Swing(int fFirst);

    int m_iSwing;
    TraceResult m_trHit;
};

#endif
