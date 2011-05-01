/* Copyright (C) 2006 - 2011 ScriptDev2 <http://www.scriptdev2.com/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Sholazar_Basin
SD%Complete: 100
SDComment: Quest support: 12573, 12570, 12580, 12688
SDCategory: Sholazar Basin
EndScriptData */

/* ContentData
npc_helice
npc_injured_rainspeaker
npc_mosswalker_victim
npc_vekjik - TODO, can be moved to database (already exist)
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## npc_helice
######*/

enum
{
    QUEST_ENGINEERING_DISASTER          = 12688,

    SAY_HELICE_ACCEPT                   = -1000657,
    SAY_HELICE_EXPLOSIVES_1             = -1000658,
    SAY_HELICE_EXPLODE_1                = -1000659,
    SAY_HELICE_MOVE_ON                  = -1000660,
    SAY_HELICE_EXPLOSIVES_2             = -1000661,
    SAY_HELICE_EXPLODE_2                = -1000662,
    SAY_HELICE_COMPLETE                 = -1000663,

    SPELL_DETONATE_EXPLOSIVES_1         = 52369,            // first "barrel"
    SPELL_DETONATE_EXPLOSIVES_2         = 52371,            // second "barrel"
};

struct MANGOS_DLL_DECL npc_heliceAI : public npc_escortAI
{
    npc_heliceAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        m_uiExplodeTimer = 5000;
        m_uiExplodePhase = 0;
        m_bFirstBarrel = true;
        Reset();
    }

    uint32 m_uiExplodeTimer;
    uint32 m_uiExplodePhase;
    bool m_bFirstBarrel;

    void Reset()
    {
    }

    void WaypointReached(uint32 uiPointId)
    {
        switch(uiPointId)
        {
            case 2:
            {
                if (Player* pPlayer = GetPlayerForEscort())
                {
                    DoScriptText(SAY_HELICE_EXPLOSIVES_1, m_creature, pPlayer);
                    SetEscortPaused(true);
                }
                break;
            }
            case 13:
            {
                if (Player* pPlayer = GetPlayerForEscort())
                {
                    DoScriptText(SAY_HELICE_EXPLOSIVES_2, m_creature, pPlayer);
                    SetEscortPaused(true);
                }
                break;
            }
            case 22:
            {
                if (Player* pPlayer = GetPlayerForEscort())
                {
                    DoScriptText(SAY_HELICE_COMPLETE, m_creature, pPlayer);
                    pPlayer->GroupEventHappens(QUEST_ENGINEERING_DISASTER, m_creature);
                }
                break;
            }
        }
    }

    void UpdateEscortAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
        {
            if (!HasEscortState(STATE_ESCORT_ESCORTING))
                return;

            if (HasEscortState(STATE_ESCORT_PAUSED))
            {
                if (m_uiExplodeTimer < uiDiff)
                {
                    if (m_bFirstBarrel)
                    {
                        switch(m_uiExplodePhase)
                        {
                            case 0:
                                DoCastSpellIfCan(m_creature, SPELL_DETONATE_EXPLOSIVES_1);

                                if (Player* pPlayer = GetPlayerForEscort())
                                    DoScriptText(SAY_HELICE_EXPLODE_1, m_creature, pPlayer);

                                m_uiExplodeTimer = 2500;
                                ++m_uiExplodePhase;
                                break;
                            case 1:
                                if (Player* pPlayer = GetPlayerForEscort())
                                    DoScriptText(SAY_HELICE_MOVE_ON, m_creature, pPlayer);

                                m_uiExplodeTimer = 2500;
                                ++m_uiExplodePhase;
                                break;
                            case 2:
                                SetEscortPaused(false);
                                m_uiExplodePhase = 0;
                                m_uiExplodeTimer = 5000;
                                m_bFirstBarrel = false;
                                break;
                        }
                    }
                    else
                    {
                        switch(m_uiExplodePhase)
                        {
                            case 0:
                                DoCastSpellIfCan(m_creature, SPELL_DETONATE_EXPLOSIVES_2);

                                if (Player* pPlayer = GetPlayerForEscort())
                                    DoScriptText(SAY_HELICE_EXPLODE_2, m_creature, pPlayer);

                                m_uiExplodeTimer = 2500;
                                ++m_uiExplodePhase;
                                break;
                            case 1:
                                SetEscortPaused(false);
                                m_uiExplodePhase = 0;
                                m_uiExplodeTimer = 5000;
                                m_bFirstBarrel = true;
                                break;
                        }
                    }
                }
                else
                    m_uiExplodeTimer -= uiDiff;
            }

            return;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_helice(Creature* pCreature)
{
    return new npc_heliceAI(pCreature);
}

bool QuestAccept_npc_helice(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_ENGINEERING_DISASTER)
    {
        DoScriptText(SAY_HELICE_ACCEPT, pCreature, pPlayer);

        if (npc_heliceAI* pEscortAI = dynamic_cast<npc_heliceAI*>(pCreature->AI()))
        {
            pEscortAI->Start(false, pPlayer->GetGUID(), pQuest);
            pCreature->SetFactionTemporary(FACTION_ESCORT_N_NEUTRAL_PASSIVE, TEMPFACTION_RESTORE_RESPAWN);
        }
    }

    return false;
}

/*######
## npc_injured_rainspeaker
######*/

enum
{
    QUEST_FORTUNATE_MISUNDERSTAND       = 12570,

    GOSSIP_ITEM_READY                   = -3000103,

    SAY_ACCEPT                          = -1000605,
    SAY_START                           = -1000606,
    SAY_END_1                           = -1000607,
    SAY_END_2                           = -1000608,
    SAY_TRACKER                         = -1000609,         // not used in escort (aggro text for trackers? something for vekjik?)

    NPC_FRENZYHEART_TRACKER             = 28077,

    SPELL_ORACLE_ESCORT_START           = 51341,            // unknown purpose
    SPELL_FEIGN_DEATH                   = 51329,
    SPELL_ORACLE_INTRO                  = 51448,
};

struct MANGOS_DLL_DECL npc_injured_rainspeakerAI : public npc_escortAI
{
    npc_injured_rainspeakerAI(Creature* pCreature) : npc_escortAI(pCreature) { Reset(); }

    void Reset() { }

    void JustStartedEscort()
    {
        if (Player* pPlayer = GetPlayerForEscort())
        {
            DoScriptText(SAY_START, m_creature, pPlayer);
            DoCastSpellIfCan(m_creature, SPELL_ORACLE_ESCORT_START);
        }
    }

    void WaypointReached(uint32 uiPointId)
    {
        switch(uiPointId)
        {
            case 22:
            {
                if (Player* pPlayer = GetPlayerForEscort())
                {
                    DoScriptText(SAY_END_1, m_creature, pPlayer);
                    DoCastSpellIfCan(m_creature, SPELL_ORACLE_INTRO);
                }
                break;
            }
            case 23:
            {
                DoScriptText(SAY_END_2, m_creature);

                // location behind
                float fAngle = m_creature->GetOrientation();
                fAngle += M_PI_F;

                float fX, fY, fZ;
                m_creature->GetNearPoint(m_creature, fX, fY, fZ, 0.0f, 15.0f, fAngle);

                m_creature->SummonCreature(NPC_FRENZYHEART_TRACKER, fX, fY, fZ, m_creature->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 30000);
                break;
            }
        }
    }

    void UpdateEscortAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_injured_rainspeaker(Creature* pCreature)
{
    return new npc_injured_rainspeakerAI(pCreature);
}

bool QuestAccept_npc_injured_rainspeaker(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_FORTUNATE_MISUNDERSTAND)
    {
        pCreature->RemoveAurasDueToSpell(SPELL_FEIGN_DEATH);
        DoScriptText(SAY_ACCEPT, pCreature, pPlayer);

        // Workaround, GossipHello/GossipSelect doesn't work well when object already has gossip from database
        if (npc_injured_rainspeakerAI* pEscortAI = dynamic_cast<npc_injured_rainspeakerAI*>(pCreature->AI()))
        {
            pEscortAI->Start(true, pPlayer->GetGUID(), pQuest);
            pCreature->SetFactionTemporary(FACTION_ESCORT_N_NEUTRAL_PASSIVE, TEMPFACTION_RESTORE_RESPAWN);
        }
    }

    return false;
}

/*
bool GossipHello_npc_injured_rainspeaker(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->GetQuestStatus(QUEST_FORTUNATE_MISUNDERSTAND) == QUEST_STATUS_INCOMPLETE)
    {
        pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_READY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
        return true;
    }

    return false;
}

bool GossipSelect_npc_injured_rainspeaker(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF)
    {
        pPlayer->CLOSE_GOSSIP_MENU();

        if (npc_injured_rainspeakerAI* pEscortAI = dynamic_cast<npc_injured_rainspeakerAI*>(pCreature->AI()))
            pEscortAI->Start(true, pPlayer->GetGUID());
    }

    return false;
}
*/

/*######
## npc_mosswalker_victim
######*/

enum
{
    QUEST_MOSSWALKER_SAVIOR         = 12580,
    SPELL_DEAD_SOLDIER              = 45801,                // not clear what this does, but looks like all have it
    SPELL_MOSSWALKER_QUEST_CREDIT   = 52157,

    GOSSIP_ITEM_PULSE               = -3000104,
    TEXT_ID_INJURED                 = 13318,

    EMOTE_PAIN                      = -1000610,

    SAY_RESCUE_1                    = -1000611,
    SAY_RESCUE_2                    = -1000612,
    SAY_RESCUE_3                    = -1000613,
    SAY_RESCUE_4                    = -1000614,

    SAY_DIE_1                       = -1000615,
    SAY_DIE_2                       = -1000616,
    SAY_DIE_3                       = -1000617,
    SAY_DIE_4                       = -1000618,
    SAY_DIE_5                       = -1000619,
    SAY_DIE_6                       = -1000620,
};

bool GossipHello_npc_mosswalker_victim(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->GetQuestStatus(QUEST_MOSSWALKER_SAVIOR) == QUEST_STATUS_INCOMPLETE)
    {
        // doesn't appear they always emote
        if (urand(0,3) == 0)
            DoScriptText(EMOTE_PAIN, pCreature);

        pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_PULSE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
    }

    pPlayer->SEND_GOSSIP_MENU(TEXT_ID_INJURED, pCreature->GetGUID());
    return true;
}

bool GossipSelect_npc_mosswalker_victim(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF)
    {
        pPlayer->CLOSE_GOSSIP_MENU();

        // just to prevent double credit
        if (pCreature->GetLootRecipient())
            return true;
        else
            pCreature->SetLootRecipient(pPlayer);

        if (urand(0,2))                                     // die
        {
            switch(urand(0,5))
            {
                case 0: DoScriptText(SAY_DIE_1, pCreature, pPlayer); break;
                case 1: DoScriptText(SAY_DIE_2, pCreature, pPlayer); break;
                case 2: DoScriptText(SAY_DIE_3, pCreature, pPlayer); break;
                case 3: DoScriptText(SAY_DIE_4, pCreature, pPlayer); break;
                case 4: DoScriptText(SAY_DIE_5, pCreature, pPlayer); break;
                case 5: DoScriptText(SAY_DIE_6, pCreature, pPlayer); break;
            }
        }
        else                                                // survive
        {
            switch(urand(0,3))
            {
                case 0: DoScriptText(SAY_RESCUE_1, pCreature, pPlayer); break;
                case 1: DoScriptText(SAY_RESCUE_2, pCreature, pPlayer); break;
                case 2: DoScriptText(SAY_RESCUE_3, pCreature, pPlayer); break;
                case 3: DoScriptText(SAY_RESCUE_4, pCreature, pPlayer); break;
            }

            pCreature->CastSpell(pPlayer, SPELL_MOSSWALKER_QUEST_CREDIT, true);
        }

        // more details may apply, instead of just despawn
        pCreature->ForcedDespawn(5000);
    }
    return true;
}

/*######
## npc_vekjik - TODO, can be moved to database (already exist)
######*/

#define GOSSIP_VEKJIK_ITEM1 "Shaman Vekjik, I have spoken with the big-tongues and they desire peace. I have brought this offering on their behalf."
#define GOSSIP_VEKJIK_ITEM2 "No no... I had no intentions of betraying your people. I was only defending myself. it was all a misunderstanding."

enum
{
    GOSSIP_TEXTID_VEKJIK1       = 13137,
    GOSSIP_TEXTID_VEKJIK2       = 13138,

    SAY_TEXTID_VEKJIK1          = -1000208,

    SPELL_FREANZYHEARTS_FURY    = 51469,

    QUEST_MAKING_PEACE          = 12573
};

bool GossipHello_npc_vekjik(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());

    if (pPlayer->GetQuestStatus(QUEST_MAKING_PEACE) == QUEST_STATUS_INCOMPLETE)
    {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_VEKJIK_ITEM1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_VEKJIK1, pCreature->GetGUID());
        return true;
    }

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
    return true;
}

bool GossipSelect_npc_vekjik(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_VEKJIK_ITEM2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_VEKJIK2, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->CLOSE_GOSSIP_MENU();
            DoScriptText(SAY_TEXTID_VEKJIK1, pCreature, pPlayer);
            pPlayer->AreaExploredOrEventHappens(QUEST_MAKING_PEACE);
            pCreature->CastSpell(pPlayer, SPELL_FREANZYHEARTS_FURY, false);
            break;
    }

    return true;
}

void AddSC_sholazar_basin()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_helice";
    pNewScript->GetAI = &GetAI_npc_helice;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_helice;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_injured_rainspeaker";
    pNewScript->GetAI = &GetAI_npc_injured_rainspeaker;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_injured_rainspeaker;
    //pNewScript->pGossipHello = &GossipHello_npc_injured_rainspeaker;
    //pNewScript->pGossipSelect = &GossipSelect_npc_injured_rainspeaker;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_mosswalker_victim";
    pNewScript->pGossipHello = &GossipHello_npc_mosswalker_victim;
    pNewScript->pGossipSelect = &GossipSelect_npc_mosswalker_victim;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_vekjik";
    pNewScript->pGossipHello = &GossipHello_npc_vekjik;
    pNewScript->pGossipSelect = &GossipSelect_npc_vekjik;
    pNewScript->RegisterSelf();
}