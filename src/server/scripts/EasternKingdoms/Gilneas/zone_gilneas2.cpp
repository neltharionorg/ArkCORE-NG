/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2011-2016 ArkCORE <http://www.arkania.net/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "zone_gilneas.h"
#include "ScriptPCH.h"
#include "Unit.h"
#include "ScriptedEscortAI.h"
#include "ScriptedFollowerAI.h"
#include "Vehicle.h"
#include "GameObjectAI.h"
#include "CreatureGroups.h"

// Phase 2

/* 35660 // part showfight worgen <> liam */   /* Quest 14098 */
class npc_rampaging_worgen_35660 : public CreatureScript
{
public:
    npc_rampaging_worgen_35660() : CreatureScript("npc_rampaging_worgen_35660") { }

    struct npc_rampaging_worgen_35660AI : public ScriptedAI
    {
        npc_rampaging_worgen_35660AI(Creature* creature) : ScriptedAI(creature) { }

        uint32 m_attackLiamT;
        uint32 m_attackLiamP;
        uint32 m_enrage;
        uint32 m_merchantP;
        uint32 m_merchantT;

        void Reset()
        {
            m_attackLiamT = 0;
            m_attackLiamP = 0;
            m_enrage = 0;
            m_merchantP = 0;
            m_merchantT = 0;
        }

        void JustDied(Unit* /*killer*/)
        {
            if (Creature* liam = me->FindNearestCreature(34913, 4.0f))
            {
                m_attackLiamP = 0;
                me->DespawnOrUnsummon(1000);
            }
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE && id == 1234)
            {
                m_attackLiamP = 1;
                m_attackLiamT = 500;
            }
        }

        void DamageTaken(Unit* who, uint32 &damage)
        {
            if (who->GetEntry() == 34913)
            {
                damage /= 2;
                if (!m_enrage && me->GetHealthPct() < 50.0f)
                {
                    me->CastSpell(me, 56646);
                    m_enrage = 250;
                }
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (m_attackLiamT <= diff)
            {
                m_attackLiamT = 500;
                DoFight();
            }
            else m_attackLiamT -= diff;

            if (m_merchantT <= diff)
            {
                m_merchantT = 1000;
                DoMerchant();
            }
            else m_merchantT -= diff;

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }

        void DoFight()
        {
            if (m_enrage)
                m_enrage--;

            switch (m_attackLiamP)
            {
            case 1:
                me->SetSpeed(MOVE_RUN, 1.8f, true);
                me->GetMotionMaster()->MovePoint(1234, -1482.9f, 1394.6f, 35.55f);
                m_attackLiamP = 2;
                break;
            case 2:
                if (Creature* liam = me->FindNearestCreature(34913, 100.0f))
                {
                    m_attackLiamP = 3;
                    Position p1 = liam->GetPosition();
                    float o1 = liam->GetAngle(p1.GetPositionX(), p1.GetPositionY());
                    Position p2 = liam->GetNearPosition(2.0f, o1);
                    me->GetMotionMaster()->MovePoint(0, p2, true);
                }
                break;
            case 3:
                if (Creature* liam = me->FindNearestCreature(34913, 4.0f))
                {
                    m_attackLiamP = 4;
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->Attack(liam, true);
                    liam->Attack(me, true);
                }
                break;
            }
        }

        void DoMerchant()
        {
            switch (m_merchantP)
            {
            case 1:
                m_merchantP = 2;
                if (Creature* c = me->FindNearestCreature(35836, 10.0f))
                {
                    me->GetMotionMaster()->MoveFollow(c, 1.0f, 0.0f);
                }
                break;
            case 2:
                Creature* c = me->FindNearestCreature(35836, 10.0f);
                if (!c)
                    me->DespawnOrUnsummon(0);
                break;
            }
        }

        void StartAnimMerchant(Player* player)
        {
            if (!this) return;

            m_merchantT = 1000;
            m_merchantP = 1;
        }

        void StartAnimFight()
        {
            if (!this) return;

            m_attackLiamT = 500;
            m_attackLiamP = 1;
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_rampaging_worgen_35660AI(creature);
    }
};

// 34913 // part showfight liam <> worgen
class npc_prince_liam_greymane_34913 : public CreatureScript
{
public:
    npc_prince_liam_greymane_34913() : CreatureScript("npc_prince_liam_greymane_34913") { }

    struct npc_prince_liam_greymane_34913AI : public ScriptedAI
    {
        npc_prince_liam_greymane_34913AI(Creature* c) : ScriptedAI(c) {}

        uint32 m_timer;
        uint32 m_phase;

        void Reset()
        {
            m_timer = 2000;
            m_phase = 0;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
        }

        void DamageTaken(Unit* who, uint32 &damage)
        {
            damage /= 2;
        }

        void UpdateAI(uint32 diff)
        {
            if (m_timer <= diff)
            {
                m_timer = 120000;
                DoShowFight();
            }
            else m_timer -= diff;

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }

        void DoShowFight()
        {
            if (Unit* unit = me->SummonCreature(35660, -1491.9f, 1413.1f, 35.56f, 5.5f))
                if (Creature* worgen = unit->ToCreature())
                    CAST_AI(npc_rampaging_worgen_35660::npc_rampaging_worgen_35660AI, worgen->AI())->StartAnimFight();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_prince_liam_greymane_34913AI(creature);
    }
};

// 34916 // part showfight guard <> worgen..
class npc_gilneas_city_guard_34916 : public CreatureScript
{
public:
    npc_gilneas_city_guard_34916() : CreatureScript("npc_gilneas_city_guard_34916") { }

    struct npc_gilneas_city_guard_34916AI : public ScriptedAI
    {
        npc_gilneas_city_guard_34916AI(Creature* creature) : ScriptedAI(creature) { }

        uint32 m_timer;
        uint32 m_phase;

        void Reset()
        {
            m_timer = urand(100, 2000);
            m_phase = 0;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
        }
       
        void DamageTaken(Unit* who, uint32 &damage)
        {
            if (who->GetEntry() == 34884)
                damage = 0;
        }

        void UpdateAI(uint32 diff)
        {
            if (m_timer <= diff)
            {
                m_timer = urand(900,1200);
                DoShowFight();
            }
            else m_timer -= diff;

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }

        void DoShowFight()
        {
            if (Creature* worgen = me->FindNearestCreature(34884, 4.0f))
            {
                me->SetFacingToObject(worgen);
                me->HandleEmote(EMOTE_ONESHOT_ATTACK1H);
            }
            else if (Creature* worgen = me->FindNearestCreature(35660, 4.0f))
            {
                me->SetFacingToObject(worgen);
                me->HandleEmote(EMOTE_ONESHOT_ATTACK1H);
            }
            else if (Creature* worgen = me->FindNearestCreature(35118, 4.0f))
            {
                me->SetFacingToObject(worgen);
                me->HandleEmote(EMOTE_ONESHOT_ATTACK1H);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gilneas_city_guard_34916AI(creature);
    }
};

// 34884 // part showfight worgen <> guard..
class npc_rampaging_worgen_34884 : public CreatureScript
{
public:
    npc_rampaging_worgen_34884() : CreatureScript("npc_rampaging_worgen_34884") {}

    struct npc_rampaging_worgen_34884AI : public ScriptedAI
    {
        npc_rampaging_worgen_34884AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 m_timer;
        uint32 m_phase;
        uint32 m_encount;

        void Reset()
        {
            m_timer = urand(100, 2000);
            m_phase = 0;
            m_encount = urand(20, 220);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
        }

        void DamageTaken(Unit* who, uint32 &damage)
        {
            if (who->ToPlayer())
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
            if (who->GetEntry() == 34916)
                damage = 0;
        }

        void UpdateAI(uint32 diff)
        {
            if (m_timer <= diff)
            {
                m_timer = urand(900, 1200);
                DoShowFight();
            }
            else m_timer -= diff;

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }

        void DoShowFight()
        {
            if (Creature* guard = me->FindNearestCreature(34916, 4.0f))
            {
                me->SetFacingToObject(guard);
                if (m_encount)
                {
                    me->HandleEmote(EMOTE_ONESHOT_ATTACK1H);
                    m_encount--;
                }
                else
                {
                    m_encount = urand(160, 220);
                    me->CastSpell(me, 8599);
                    m_timer = 2500;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_rampaging_worgen_34884AI(creature);
    }
};

/* Quest Evacuate the Merchant Square 14098
   34981 Frightened Citizen,  NPC may be spawned via a script ( alone )
   35836 Frightened Citizen,  NPC may be spawned via a script ( with worgen )
   35660 Rampaging Worgen,  NPC may be spawned via a script */   
class npc_frightened_citizen_34981 : public CreatureScript
{
public:
    npc_frightened_citizen_34981() : CreatureScript("npc_frightened_citizen_34981") {}

    struct npc_frightened_citizen_34981AI : public ScriptedAI
    {
        npc_frightened_citizen_34981AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 m_merchantP;
        uint32 m_merchantT;

        void Reset()
        {
            startPath = false;
            mui_moveTimer = 1000;
            m_merchantP = 0;
            m_merchantT = 0;
        }

        void MovementInform(uint32 type, uint32 point)
        {
            if (type == 8)
                switch (point)
                {
                case 1230:
                    m_merchantP = 3;
                    break;
                case 1231:
                    me->DespawnOrUnsummon();
                    break;
                }
        }

        void UpdateAI(uint32 diff)
        {
            if (m_merchantT <= diff)
            {
                m_merchantT = 1000;
                DoMerchant();
            }
            else m_merchantT -= diff;

            if (!startPath)
                return;

            if (mui_moveTimer <= diff)
            {
                me->ClearUnitState(UNIT_STATE_IGNORE_PATHFINDING);
                me->GetMotionMaster()->MovePoint(5, FrightenedWay[urand(0, 1)]);
                mui_moveTimer = 10000;
            }
            else
                mui_moveTimer -= diff;
        }

        void DoMerchant()
        {
            me->setFaction(35);
            switch (m_merchantP)
            {
            case 1:
                m_merchantP = 2;
                if (me->GetEntry() == 34981)
                {
                    if (Creature* m = me->FindNearestCreature(35830, 10.0f))
                    {
                        me->SetFacingToObject(m);
                        Talk(0);
                        me->GetMotionMaster()->MovePoint(1230, m->GetPosition());
                    }
                }
                else if (me->GetEntry() == 35836)
                {
                    if (Creature* m = me->FindNearestCreature(35660, 10.0f))
                    {
                        me->SetFacingToObject(m);
                        Talk(1);
                        me->GetMotionMaster()->MovePoint(1230, m->GetPosition());
                    }
                }
                break;
            case 2:
                break;
            case 3:
                m_merchantP = 4;
                if (Creature* m = me->FindNearestCreature(35010, 100.0f))
                    me->GetMotionMaster()->MovePoint(1231, m->GetPosition());
                break;
            case 4:
                break;
            }
        }

        void StartAnim(Player* player)
        {
            if (!this) return;

            m_merchantP = 1;
            m_merchantT = 1000;
        }

    private:
        bool startPath;
        uint32 mui_moveTimer;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_frightened_citizen_34981AI(creature);
    }
};

// 195327
class go_merchant_square_door_195327 : public GameObjectScript
{
public:
    go_merchant_square_door_195327() : GameObjectScript("go_merchant_square_door_195327") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if (!player || !go)
            return false;

        switch (urand(0, 1))
        {
            case 0:
                SummonOnlyCitizen(player, go);
                break;
            case 1:
                SummonCitizenAndWorgen(player, go);
                break;
        }

        return false;
    }

private:
    void SummonOnlyCitizen(Player* player, GameObject* go)
    {
        Position posC = GetCitizenPosition(go);
        Unit* unitC = player->SummonCreature(34981, posC, TEMPSUMMON_TIMED_DESPAWN, 30000);
        if (Creature* citizen = unitC->ToCreature())
            CAST_AI(npc_frightened_citizen_34981::npc_frightened_citizen_34981AI, citizen->AI())->StartAnim(player);
    }
        
    void SummonCitizenAndWorgen(Player* player, GameObject* go)
    {
        Position posC = GetCitizenPosition(go);
        Unit* unitC = player->SummonCreature(35836, posC, TEMPSUMMON_TIMED_DESPAWN, 30000);
        Position posW = GetWorgenPosition(go);
        Unit* unitW = player->SummonCreature(35660, posW, TEMPSUMMON_TIMED_DESPAWN, 30000);
        if (Creature* citizen = unitC->ToCreature())
            if (Creature* worgen = unitW->ToCreature())
            {
                CAST_AI(npc_frightened_citizen_34981::npc_frightened_citizen_34981AI, citizen->AI())->StartAnim(player);
                CAST_AI(npc_rampaging_worgen_35660::npc_rampaging_worgen_35660AI, worgen->AI())->StartAnimMerchant(player);
            }
    }

    Position GetCitizenPosition(GameObject* go)
    {
        float x, y;
        go->GetNearPoint2D(x, y, 1.0f, go->GetOrientation() + M_PI);
        return Position(x, y, go->GetPositionZ());
    }

    Position GetWorgenPosition(GameObject* go)
    {
        float x, y;
        go->GetNearPoint2D(x, y, 3.0f, go->GetOrientation() + M_PI * 0.75f);
        return Position(x, y, go->GetPositionZ());
    }
};

// 35872
class npc_mariam_spellwalker_35872 : public CreatureScript
{
public:
    npc_mariam_spellwalker_35872() : CreatureScript("npc_mariam_spellwalker_35872") {}

    struct npc_mariam_spellwalker_35872AI : public ScriptedAI
    {
        npc_mariam_spellwalker_35872AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSeek;

        void Reset()
        {
            tSeek = urand(1000, 2000);
        }

        void DamageTaken(Unit* who, uint32& damage)
        {
            if (me->HealthBelowPct(AI_MIN_HP) && who->GetEntry() == NPC_BLOODFANG_WORGEN)
                damage = 0;
        }

        void UpdateAI(uint32 diff)
        {
            if (tSeek <= diff)
            {
                if ((me->IsAlive()) && (!me->IsInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
                    if (Creature* enemy = me->FindNearestCreature(NPC_BLOODFANG_WORGEN, 5.0f, true))
                        me->AI()->AttackStart(enemy); // She should really only grab agro when npc Cleese is not there, so we will keep this range small
                tSeek = urand(1000, 2000); // optimize cpu load, seeking only sometime between 1 and 2 seconds
            }
            else tSeek -= diff;

            if (!UpdateVictim())
                return;

            if (me->GetVictim()->GetEntry() == NPC_BLOODFANG_WORGEN)
                DoSpellAttackIfReady(SPELL_FROSTBOLT_VISUAL_ONLY); //Dummy spell, visual only to prevent getting agro (Blizz-like)
            else
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_mariam_spellwalker_35872AI(creature);
    }
};

// 35232
class npc_gilnean_royal_guard_35232 : public CreatureScript
{
public:
    npc_gilnean_royal_guard_35232() : CreatureScript("npc_gilnean_royal_guard_35232") {}

    struct npc_gilnean_royal_guard_35232AI : public ScriptedAI
    {
        npc_gilnean_royal_guard_35232AI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            tAnimate   = DELAY_ANIMATE;
            dmgCount   = 0;
            tSound     = DELAY_SOUND;
            playSound  = false;
            tSeek      = urand(1000, 2000);
        }

        void DamageTaken(Unit* who, uint32 &Damage)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
                dmgCount = 0;
            }
            else if (who->IsPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
                dmgCount = 0;
            }
        }

        void DamageDealt(Unit* target, uint32 &damage, DamageEffectType damageType)
        {
            if (target->GetEntry() == NPC_BLOODFANG_WORGEN)
                dmgCount ++;
        }

        void UpdateAI(uint32 diff)
        {
            if (tSeek <= diff)
            {
                if ((me->IsAlive()) && (!me->IsInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
                    if (Creature* enemy = me->FindNearestCreature(NPC_BLOODFANG_WORGEN, 16.0f, true))
                        me->AI()->AttackStart(enemy);
                tSeek = urand(1000, 2000);
            }
            else tSeek -= diff;

            if (!UpdateVictim())
                return;

            if (tSound <= diff)
            {
                me->PlayDistanceSound(SOUND_SWORD_FLESH);
                tSound = DELAY_SOUND;
                playSound = false;
            }

            if (playSound == true)
                tSound -= diff;

            if (dmgCount < 2)
                DoMeleeAttackIfReady();
            else
                if (me->GetVictim()->GetTypeId() == TYPEID_PLAYER) dmgCount = 0;
            else
                if (me->GetVictim()->IsPet()) dmgCount = 0;
            else
            {
                if (tAnimate <= diff)
                {
                    me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
                    playSound = true;
                    tAnimate = DELAY_ANIMATE;
                }
                else
                    tAnimate -= diff;
            }
        }

    private :
        uint32 tAnimate;
        uint32 tSound;
        uint32 dmgCount;
        uint32 tSeek;
        bool playSound;

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gilnean_royal_guard_35232AI(creature);
    }
};

// 35839
class npc_sergeant_cleese_35839 : public CreatureScript
{
public:
    npc_sergeant_cleese_35839() : CreatureScript("npc_sergeant_cleese_35839") {}

    struct npc_sergeant_cleese_35839AI : public ScriptedAI
    {
        npc_sergeant_cleese_35839AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSeek;

        void Reset()
        {
            tSeek      = urand(1000, 2000);
        }

        void DamageTaken(Unit* who, uint32& damage)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (who->IsPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (me->HealthBelowPct(AI_MIN_HP) && who->GetEntry() == NPC_BLOODFANG_WORGEN)
                damage = 0;
        }

        void UpdateAI(uint32 diff)
        {
            if (tSeek <= diff)
            {
                if ((me->IsAlive()) && (!me->IsInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
                    if (Creature* enemy = me->FindNearestCreature(NPC_BLOODFANG_WORGEN, 10.0f, true))
                        me->AI()->AttackStart(enemy);
                tSeek = urand(1000, 2000); // optimize cpu load, seeking only sometime between 1 and 2 seconds
            }
            else tSeek -= diff;

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_sergeant_cleese_35839AI(creature);
    }
};

/* 35077 - QUEST: 14154 - By The Skin of His Teeth - START */
class npc_lord_darius_crowley_35077 : public CreatureScript
{
public:
    npc_lord_darius_crowley_35077() : CreatureScript("npc_lord_darius_crowley_35077") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == 14154)
            if (CAST_AI(npc_lord_darius_crowley_35077AI,creature->AI())->m_playerGUID == 0)
            {
                creature->AI()->SetGUID(player->GetGUID());
                creature->AI()->DoAction(ACTION_START_EVENT);
                creature->CastSpell(player, SPELL_BY_THE_SKIN, true);
            }
        return true;
    }

    struct npc_lord_darius_crowley_35077AI : public ScriptedAI
    {
        npc_lord_darius_crowley_35077AI(Creature* creature) : ScriptedAI(creature), m_summons(me) { Init(); }

        enum eQ14154
        {
            Event120Secounds = 1,
            EventCheckPlayerIsAlive,
            EventSummonNextWave,
            EventHelpPlayer,
        };

        uint64 m_playerGUID;
        EventMap m_events;
        SummonList m_summons;
        uint32 m_phase;

        void Init()
        {
            m_events.Reset();
            m_playerGUID = 0;
            m_phase = 0;
            m_summons.DespawnAll();
        }

        void JustSummoned(Creature* summoned) override
        {
            m_summons.Summon(summoned);
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
        {
            m_summons.Despawn(summon);
        }

        void DoAction(int32 /*action*/) override
        {
            m_phase = 1;
            m_events.ScheduleEvent(EventCheckPlayerIsAlive, 1000);
            m_events.ScheduleEvent(EventSummonNextWave, 1000);
            m_events.ScheduleEvent(Event120Secounds, 120000);
            m_events.ScheduleEvent(EventHelpPlayer, 250);
        }

        void SetGUID(uint64 guid, int32 type = 0) override
        {
            m_playerGUID = guid;
        }

        void DamageTaken(Unit* attacker, uint32& /*damage*/) 
        { 
            if (Creature* worgen = attacker->ToCreature())
                if (!me->IsInCombat())
                {
                } //   me->Attack(worgen, true);
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case Event120Secounds:
                        Init();
                        break;
                    case EventCheckPlayerIsAlive: // check every sec player is alive
                        if (m_playerGUID && m_phase)
							if (Player* player = ObjectAccessor::GetPlayer(*me, m_playerGUID))
                                if (!player->IsInWorld() || !player->IsAlive())
                                    Init();

                        m_events.ScheduleEvent(EventCheckPlayerIsAlive, 1000);
                        break;
                    case EventSummonNextWave:
                    {
                        for (int i = 0; i < 4; i++)
                        {
                            uint32 w1 = RAND(NPC_WORGEN_RUNT_C1, NPC_WORGEN_RUNT_C2, NPC_WORGEN_ALPHA_C1, NPC_WORGEN_ALPHA_C2);
                            uint32 w2 = RAND(NPC_WORGEN_RUNT_C1, NPC_WORGEN_RUNT_C2, NPC_WORGEN_ALPHA_C1, NPC_WORGEN_ALPHA_C2);
                            Creature* creature1 = me->SummonCreature(w1, -1610.39f, 1507.16f, 74.99f, 3.94f, TEMPSUMMON_TIMED_DESPAWN, 120000);
                            m_summons.Summon(creature1);
                            creature1->AI()->SetGUID(m_playerGUID);
                            creature1->AI()->DoAction(1);
                            Creature* creature2 = me->SummonCreature(w2, -1718.01f, 1516.81f, 55.40f, 4.6f, TEMPSUMMON_TIMED_DESPAWN, 120000);
                            m_summons.Summon(creature2);
                            creature2->AI()->SetGUID(m_playerGUID);
                            creature2->AI()->DoAction(2);
                        }

                        m_events.ScheduleEvent(EventSummonNextWave, 30000); // every 30 secounds one wave
                        break;
                    }
                    case EventHelpPlayer:
                    {
                        if (!me->IsInCombat())
                        {
                            Creature* creature = NULL;
                            creature = me->FindNearestCreature(NPC_WORGEN_RUNT_C1, 5.0f);
                            if (!creature)
                                creature = me->FindNearestCreature(NPC_WORGEN_RUNT_C2, 5.0f);
                            if (!creature)
                                creature = me->FindNearestCreature(NPC_WORGEN_ALPHA_C1, 5.0f);
                            if (!creature)
                                creature = me->FindNearestCreature(NPC_WORGEN_ALPHA_C2, 5.0f);
                            if (creature)
                            {
                                me->Attack(creature, true);
                                // creature->Attack(me, true);
                            }
                        }

                        m_events.ScheduleEvent(EventHelpPlayer, 250);
                        break;
                    }
                }
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_lord_darius_crowley_35077AI(creature);
    }
};

// 35124
class npc_tobias_mistmantle_35124 : public CreatureScript
{
public:
    npc_tobias_mistmantle_35124() : CreatureScript("npc_tobias_mistmantle_35124") { }

    struct npc_tobias_mistmantle_35124AI : public ScriptedAI
    {
        npc_tobias_mistmantle_35124AI(Creature* creature) : ScriptedAI(creature)  { }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_tobias_mistmantle_35124AI(creature);
    }
};

// 35188
class npc_worgen_runt_35188 : public CreatureScript
{
public:
    npc_worgen_runt_35188() : CreatureScript("npc_worgen_runt_35188") {}

    struct npc_worgen_runt_35188AI : public ScriptedAI
    {
        npc_worgen_runt_35188AI(Creature* creature) : ScriptedAI(creature) { Init(); }
       
        uint64 m_playerGUID;
        EventMap m_events;
        uint32 m_phase;
        Position jump;

        void Init()
        {
            m_playerGUID = 0;
            m_phase = 0;
        }

        void DoAction(int32 action) override
        {
            if (m_playerGUID)
            {
                m_events.ScheduleEvent(1, 500);
                m_phase = action;
                me->SetSpeed(MOVE_RUN, frand(1.2f, 1.6f));
            }
        }

        void SetGUID(uint64 guid, int32 type = 0) override
        {
            m_playerGUID = guid;
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == POINT_MOTION_TYPE || type == EFFECT_MOTION_TYPE)
                m_phase = pointId;
        }

        void UpdateAI(uint32 diff)
        {
            m_events.Update(diff);

            uint32 eventId = m_events.ExecuteEvent();
            switch (eventId)
            {
                case 1:
                {
                    m_events.ScheduleEvent(1, 500);
                    DoWalk();
                    break;
                }
            }

            

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

        void DoWalk()
        {
            switch (m_phase)
            {
            case 1:
            {
                m_phase = 3;
                uint8 rol = urand(0, 2);
                jump = JumpW1[rol];
                me->GetMotionMaster()->MovePoint(11 + rol, jump);
                break;
            }
            case 2:
                m_phase = 3;
                jump = Position(-1717.73f, 1486.27f, 57.23f, 5.45f);
                me->GetMotionMaster()->MovePoint(21, jump);
                break;
            case 11:
                m_phase = 4;
                me->GetMotionMaster()->MoveJump(LandingW1[0], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            case 12:
                m_phase = 4;
                me->GetMotionMaster()->MoveJump(LandingW1[1], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            case 13:
                m_phase = 4;
                me->GetMotionMaster()->MoveJump(LandingW1[2], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            case 21:
            {
                m_phase = 5;
                uint8 rol = urand(0, 3);
                jump = LandingW2[rol];
                me->GetMotionMaster()->MoveJump(jump, frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            }
            case 25:
                m_phase = 6;
                if (m_playerGUID)
					if (Player* player = ObjectAccessor::GetPlayer(*me, m_playerGUID))
                        if (player->IsInWorld() || player->IsAlive())
                        {
                            Position pos = player->GetNearPosition(frand(2.0f, 4.0f), frand(3.14f, 6.28f));
                            me->GetMotionMaster()->MovePoint(26, pos);
                        }
                break;
            case 26:
                m_phase = 7;
                me->SetHomePosition(me->GetPosition());
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_worgen_runt_35188AI(creature);
    }
};

// 35456
class npc_worgen_runt_35456 : public CreatureScript
{
public:
    npc_worgen_runt_35456() : CreatureScript("npc_worgen_runt_35456") { }

    struct npc_worgen_runt_35456AI : public ScriptedAI
    {
        npc_worgen_runt_35456AI(Creature* creature) : ScriptedAI(creature) { Init(); }
        
        uint64 m_playerGUID;
        EventMap m_events;
        uint32 m_phase;
        Position jump;

        void Init()
        {
            m_playerGUID = 0;
            m_phase = 0;
        }

        void DoAction(int32 action) override
        {
            if (m_playerGUID)
            {
                m_events.ScheduleEvent(1, 500);
                m_phase = action;
                me->SetSpeed(MOVE_RUN, frand(1.2f, 1.6f));
            }
        }

        void SetGUID(uint64 guid, int32 type = 0) override
        {
            m_playerGUID = guid;
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == POINT_MOTION_TYPE || type == EFFECT_MOTION_TYPE)
                m_phase = pointId;
        }

        void UpdateAI(uint32 diff)
        {
            m_events.Update(diff);

            uint32 eventId = m_events.ExecuteEvent();
            switch (eventId)
            {
                case 1:
                {
                    m_events.ScheduleEvent(1, 500);
                    DoWalk();
                    break;
                }
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

        void DoWalk()
        {
            switch (m_phase)
            {
            case 1:
            {
                m_phase = 3;
                uint8 rol = urand(0, 2);
                jump = JumpW1[rol];
                me->GetMotionMaster()->MovePoint(11 + rol, jump);
                break;
            }
            case 2:
                m_phase = 3;
                jump = Position(-1717.73f, 1486.27f, 57.23f, 5.45f);
                me->GetMotionMaster()->MovePoint(21, jump);
                break;
            case 11:
                m_phase = 4;
                me->GetMotionMaster()->MoveJump(LandingW1[0], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            case 12:
                m_phase = 4;
                me->GetMotionMaster()->MoveJump(LandingW1[1], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            case 13:
                m_phase = 4;
                me->GetMotionMaster()->MoveJump(LandingW1[2], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            case 21:
            {
                m_phase = 5;
                uint8 rol = urand(0, 3);
                jump = LandingW2[rol];
                me->GetMotionMaster()->MoveJump(jump, frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            }
            case 25:
                m_phase = 6;
                if (m_playerGUID)
					if (Player* player = ObjectAccessor::GetPlayer(*me, m_playerGUID))
                        if (player->IsInWorld() || player->IsAlive())
                        {
                            Position pos = player->GetNearPosition(frand(2.0f, 4.0f), frand(3.14f, 6.28f));
                            me->GetMotionMaster()->MovePoint(26, pos);
                        }
                break;
            case 26:
                m_phase = 7;
                me->SetHomePosition(me->GetPosition());
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_worgen_runt_35456AI(creature);
    }
};

// 35170
class npc_worgen_alpha_35170 : public CreatureScript
{
public:
    npc_worgen_alpha_35170() : CreatureScript("npc_worgen_alpha_35170") {}

    struct npc_worgen_alpha_35170AI : public ScriptedAI
    {
        npc_worgen_alpha_35170AI(Creature* creature) : ScriptedAI(creature) { Init(); }

        uint64 m_playerGUID;
        EventMap m_events;
        uint32 m_phase;
        Position jump;

        void Init()
        {
            m_playerGUID = 0;
            m_phase = 0;
        }

        void DoAction(int32 action) override
        {
            if (m_playerGUID)
            {
                m_events.ScheduleEvent(1, 500);
                m_phase = action;
                me->SetSpeed(MOVE_RUN, frand(1.2f, 1.6f));
            }
        }

        void SetGUID(uint64 guid, int32 type = 0) override
        {
            m_playerGUID = guid;
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == POINT_MOTION_TYPE || type == EFFECT_MOTION_TYPE)
                m_phase = pointId;
        }

        void UpdateAI(uint32 diff)
        {
            m_events.Update(diff);

            uint32 eventId = m_events.ExecuteEvent();
            switch (eventId)
            {
                case 1:
                {
                    m_events.ScheduleEvent(1, 500);
                    DoWalk();
                    break;
                }
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

        void DoWalk()
        {
            switch (m_phase)
            {
                case 1:
                {
                    m_phase = 3;
                    uint8 rol = urand(0, 2);
                    jump = JumpW1[rol];
                    me->GetMotionMaster()->MovePoint(11 + rol, jump);
                    break;
                }
                case 2:
                    m_phase = 3;
                    jump = Position(-1717.73f, 1486.27f, 57.23f, 5.45f);
                    me->GetMotionMaster()->MovePoint(21, jump);
                    break;
                case 11:
                    m_phase = 4;
                    me->GetMotionMaster()->MoveJump(LandingW1[0], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                    break;
                case 12:
                    m_phase = 4;
                    me->GetMotionMaster()->MoveJump(LandingW1[1], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                    break;
                case 13:
                    m_phase = 4;
                    me->GetMotionMaster()->MoveJump(LandingW1[2], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                    break;
                case 21:
                {
                    m_phase = 5;
                    uint8 rol = urand(0, 3);
                    jump = LandingW2[rol];
                    me->GetMotionMaster()->MoveJump(jump, frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                    break;
                }
                case 25:
                    m_phase = 6;
                    if (m_playerGUID)
						if (Player* player = ObjectAccessor::GetPlayer(*me, m_playerGUID))
                            if (player->IsInWorld() || player->IsAlive())
                            {
                                Position pos = player->GetNearPosition(frand(2.0f, 4.0f), frand(3.14f, 6.28f));
                                me->GetMotionMaster()->MovePoint(26, pos);
                            }
                    break;
                case 26:
                    m_phase = 7;
                    me->SetHomePosition(me->GetPosition());
                    break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_worgen_alpha_35170AI(creature);
    }
};

// 35167
class npc_worgen_alpha_35167 : public CreatureScript
{
public:
    npc_worgen_alpha_35167() : CreatureScript("npc_worgen_alpha_35167") {}

    struct npc_worgen_alpha_35167AI : public ScriptedAI
    {
        npc_worgen_alpha_35167AI(Creature* creature) : ScriptedAI(creature) { Init(); }

        uint64 m_playerGUID;
        EventMap m_events;
        uint32 m_phase;
        Position jump;

        void Init()
        {
            m_playerGUID = 0;
            m_phase = 0;
        }

        void DoAction(int32 action) override
        {
            if (m_playerGUID)
            {
                m_events.ScheduleEvent(1, 500);
                m_phase = action;
                me->SetSpeed(MOVE_RUN, frand(1.2f, 1.6f));
            }
        }

        void SetGUID(uint64 guid, int32 type = 0) override
        {
            m_playerGUID = guid;
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == POINT_MOTION_TYPE || type == EFFECT_MOTION_TYPE)
                m_phase = pointId;
        }

        void UpdateAI(uint32 diff)
        {
            m_events.Update(diff);

            uint32 eventId = m_events.ExecuteEvent();
            switch (eventId)
            {
                case 1:
                {
                    m_events.ScheduleEvent(1, 500);
                    DoWalk();
                    break;
                }
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

        void DoWalk()
        {
            switch (m_phase)
            {
            case 1:
            {
                m_phase = 3;
                uint8 rol = urand(0, 2);
                jump = JumpW1[rol];
                me->GetMotionMaster()->MovePoint(11 + rol, jump);
                break;
            }
            case 2:
                m_phase = 3;
                jump = Position(-1717.73f, 1486.27f, 57.23f, 5.45f);
                me->GetMotionMaster()->MovePoint(21, jump);
                break;
            case 11:
                m_phase = 4;
                me->GetMotionMaster()->MoveJump(LandingW1[0], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            case 12:
                m_phase = 4;
                me->GetMotionMaster()->MoveJump(LandingW1[1], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            case 13:
                m_phase = 4;
                me->GetMotionMaster()->MoveJump(LandingW1[2], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            case 21:
            {
                m_phase = 5;
                uint8 rol = urand(0, 3);
                jump = LandingW2[rol];
                me->GetMotionMaster()->MoveJump(jump, frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            }
            case 25:
                m_phase = 6;
                if (m_playerGUID)
					if (Player* player = ObjectAccessor::GetPlayer(*me, m_playerGUID))
                        if (player->IsInWorld() || player->IsAlive())
                        {
                            Position pos = player->GetNearPosition(frand(2.0f, 4.0f), frand(3.14f, 6.28f));
                            me->GetMotionMaster()->MovePoint(26, pos);
                        }
                break;
            case 26:
                m_phase = 7;
                me->SetHomePosition(me->GetPosition());
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_worgen_alpha_35167AI(creature);
    }
};

// 35112
class npc_king_genn_greymane_35112 : public CreatureScript
{
public:
    npc_king_genn_greymane_35112() : CreatureScript("npc_king_genn_greymane_35112") { }

    struct npc_king_genn_greymane_35112AI : public ScriptedAI
    {
        npc_king_genn_greymane_35112AI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_king_genn_greymane_35112AI(creature);
    }
};

// 35115
class npc_lord_godfrey_35115 : public CreatureScript
{
public:
    npc_lord_godfrey_35115() : CreatureScript("npc_lord_godfrey_35115") { }

    struct npc_lord_godfrey_35115AI : public ScriptedAI
    {
        npc_lord_godfrey_35115AI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_lord_godfrey_35115AI(creature);
    }
};

// 35118 showfight <> guard 34916
class npc_bloodfang_worgen_35118 : public CreatureScript
{
public:
    npc_bloodfang_worgen_35118() : CreatureScript("npc_bloodfang_worgen_35118") {}

    struct npc_bloodfang_worgen_35118AI : public ScriptedAI
    {
        npc_bloodfang_worgen_35118AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 m_timer;
        uint32 m_phase;
        uint32 m_encount;

        void Reset()
        {
            m_timer = urand(100, 2000);
            m_phase = 0;
            m_encount = urand(20, 220);
            if (Creature* guard = me->FindNearestCreature(34916, 4.0f))
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
        }

        void DamageTaken(Unit* who, uint32 &damage)
        {
            if (Player* player = who->ToPlayer())
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                me->Attack(player, true);
            }
        }

        void SpellHit(Unit* caster, SpellInfo const* spell) 
        { 
            if (Player* player = caster->ToPlayer())
            {
                if (player->GetQuestStatus(14276) == QUEST_STATUS_INCOMPLETE)
                    if (spell->Id == 56641)
                        player->KilledMonsterCredit(44175);

                if (player->GetQuestStatus(14281) == QUEST_STATUS_INCOMPLETE)
                    if (spell->Id == 5143)
                        player->KilledMonsterCredit(44175);
            }
        }


        void UpdateAI(uint32 diff)
        {
            if (m_timer <= diff)
            {
                m_timer = urand(900, 1200);
                DoShowFight();
            }
            else m_timer -= diff;

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }

        void DoShowFight()
        {
            if (Creature* guard = me->FindNearestCreature(34916, 4.0f))
            {
                me->SetFacingToObject(guard);
                if (m_encount)
                {
                    me->HandleEmote(EMOTE_ONESHOT_ATTACK1H);
                    m_encount--;
                }
                else
                {
                    m_encount = urand(160, 220);
                    me->CastSpell(me, 8599);
                    m_timer = 2500;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodfang_worgen_35118AI(creature);
    }
};
/* QUEST - 14154 - By The Skin of His Teeth - END */

/* Phase 4 - QUEST - 14159 - The Rebel Lord's Arsenal - START */
// 
// 35369
class npc_josiah_avery_35369 : public CreatureScript
{
public:
    npc_josiah_avery_35369() : CreatureScript("npc_josiah_avery_35369") {}

    bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt)
    {
        if (quest->GetQuestId() == QUEST_THE_REBEL_LORDS_ARSENAL)
        {
            creature->AddAura(SPELL_WORGEN_BITE, player);
            player->RemoveAura(SPELL_PHASE_QUEST_ZONE_SPECIFIC_01);
            creature->CastSpell(creature, SPELL_SUMMON_JOSIAH_AVERY);
        }
        return true;
    }

    struct npc_josiah_avery_35369AI : public ScriptedAI
    {
        npc_josiah_avery_35369AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 m_timerSay; 
        uint32 cSay; // Current Say

		        // Evade or Respawn
        void Reset()
        {
            m_timerSay = DELAY_SAY_JOSIAH_AVERY; // Reset timer
            cSay = 1;                            // Start from 1
        }

        //Timed events
        void UpdateAI(uint32 diff)
        {
            if (!me->GetVictim())
            {
                if (m_timerSay <= diff)
                {
                    m_timerSay = DELAY_SAY_JOSIAH_AVERY; // Reset the timer
                    DoWork();
                }
                else
                    m_timerSay -= diff;
            }
        }

        void DoWork()
        {
            std::list<Player*> pList = GetListOfPlayersNearAndIndoorsAndWithQuest();
            if (pList.empty())
                return;

            switch (cSay)
            {
            case 1:
                TalkToGroup(pList, 0);
                cSay++;
                break;
            case 2:
                TalkToGroup(pList, 1);
                cSay++;
                break;
            case 3:
                TalkToGroup(pList, 2);
                cSay++;
                break;
            case 4:
                TalkToGroup(pList, 3);
                cSay++;
                break;
            case 5:
                TalkToGroup(pList, 4);
                cSay++;
                break;
            case 6:
                TalkToGroup(pList, 5);
                cSay = 1; // Reset to 1
                break;
            }
        }

        std::list<Player*> GetListOfPlayersNearAndIndoorsAndWithQuest()
        {
            std::list<Player*> pList = me->FindNearestPlayers(20.0f);
            while (DeleteWrongPlayer(pList)) { }
            return pList;
        }

        bool DeleteWrongPlayer(std::list<Player*> &pList)
        {
            if (pList.empty())
                return false;

            for (std::list<Player*>::const_iterator itr = pList.begin(); itr != pList.end(); itr++)
            {
                Player* player = (*itr);
                if (!player->GetMap()->IsIndoors(player))
                {
                    pList.remove(player);
                    return true;
                }
                if (player->GetQuestStatus(14159) != QUEST_STATUS_INCOMPLETE)
                {
                    pList.remove(player);
                    return true;
                }
            }
            return false;
        }

        void TalkToGroup(std::list<Player*> pList, uint8 groupId)
        {
            if (pList.empty())
                return;

            for (std::list<Player*>::const_iterator itr = pList.begin(); itr != pList.end(); itr++)
                Talk(groupId, (*itr));
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_josiah_avery_35369AI(creature);
    }
};

// 50415
class npc_josiah_avery_trigger_50415 : public CreatureScript
{
public:
    npc_josiah_avery_trigger_50415() : CreatureScript("npc_josiah_avery_trigger_50415") {}

    struct npc_josiah_avery_trigger_50415AI : public ScriptedAI
    {
        npc_josiah_avery_trigger_50415AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 Phase, tEvent;
        uint64 PlayerGUID;

        void Reset()
        {
            Phase       = 0;
            PlayerGUID  = 0;
            tEvent      = 0;
        }

        void UpdateAI(uint32 diff)
        {
            if (Creature* Lorna = me->FindNearestCreature(NPC_LORNA_CROWLEY_P4, 60.0f, true))
            if (Creature* BadAvery = me->FindNearestCreature(NPC_JOSIAH_AVERY_P4, 80.0f, true))
            if (Player* player = me->SelectNearestPlayer(50.0f))
            {
                if (!player->HasAura(SPELL_WORGEN_BITE))
                    return;
                else
                    PlayerGUID = player->GetGUID();
                    if (tEvent <= diff)
                    {
                        switch (Phase)
                        {
                            case (0):
                            {
                                me->AI()->Talk(SAY_JOSAIH_AVERY_TRIGGER, player); // Tell Player they have been bitten
                                tEvent = 200;
                                Phase++;
                                break;
                            }

                            case (1):
                            {
                                BadAvery->SetOrientation(BadAvery->GetAngle(player)); // Face Player
                                BadAvery->CastSpell(player, 69873, true); // Do Cosmetic Attack
                                player->GetMotionMaster()->MoveKnockTo(-1791.94f, 1427.29f, 12.4584f, 22.0f, 8.0f, PlayerGUID);
                                BadAvery->getThreatManager().resetAllAggro();
                                tEvent = 1200;
                                Phase++;
                                break;
                            }

                            case (2):
                            {
                                BadAvery->GetMotionMaster()->MoveJump(-1791.94f, 1427.29f, 12.4584f, 18.0f, 7.0f);
                                tEvent = 600;
                                Phase++;
                                break;
                            }

                            case (3):
                            {
                                Lorna->CastSpell(BadAvery, SPELL_SHOOT, true);
                                tEvent = 200;
                                Phase++;
                                break;
                            }

                            case (4):
                            {
                                BadAvery->CastSpell(BadAvery, SPELL_GET_SHOT, true);
                                BadAvery->SetDeathState(JUST_DIED);
                                player->SaveToDB();
                                BadAvery->DespawnOrUnsummon(1000);
                                me->DespawnOrUnsummon(1000);
                                tEvent = 5000;
                                Phase++;
                                break;
                            }
                        }
                    }
                    else tEvent -= diff;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_josiah_avery_trigger_50415AI(creature);
    }
};

// 35378
class npc_lorna_crowley_35378 : public CreatureScript
{
public:
    npc_lorna_crowley_35378() : CreatureScript("npc_lorna_crowley_35378") {}

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_FROM_THE_SHADOWS)
        {
            player->CastSpell(player, SPELL_SUMMON_GILNEAN_MASTIFF);
            creature->AI()->Talk(SAY_LORNA_CROWLEY_P4);
        }
        return true;
    }
};

// 35631
class npc_gilnean_mastiff_35631 : public CreatureScript
{
public:
    npc_gilnean_mastiff_35631() : CreatureScript("npc_gilnean_mastiff_35631") { }

    struct npc_gilnean_mastiff_35631AI : public ScriptedAI
    {
        npc_gilnean_mastiff_35631AI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            if (CharmInfo* info = me->GetCharmInfo())
            {
                info->InitEmptyActionBar(false);
                info->SetActionBar(0, SPELL_ATTACK_LURKER, ACT_PASSIVE);
                me->SetReactState(REACT_DEFENSIVE);
                info->SetIsFollowing(true);
            }
        }

        void UpdateAI(uint32 diff) /*diff*/
        {
            Player* player = me->GetOwner()->ToPlayer();

            if (player->GetQuestStatus(QUEST_FROM_THE_SHADOWS) == QUEST_STATUS_REWARDED)
            {
                me->DespawnOrUnsummon(1);
            }

            if (!UpdateVictim())
            {
                if (CharmInfo* info = me->GetCharmInfo())
                    info->SetIsFollowing(true);
                me->SetReactState(REACT_DEFENSIVE);
                return;
            }

            DoMeleeAttackIfReady();
        }

        void SpellHitTarget(Unit* Mastiff, SpellInfo const* cSpell)
        {
            if (cSpell->Id == SPELL_ATTACK_LURKER)
            {
                Mastiff->RemoveAura(SPELL_SHADOWSTALKER_STEALTH);
                Mastiff->AddThreat(me, 1.0f);
                me->AddThreat(Mastiff, 1.0f);
                me->AI()->AttackStart(Mastiff);
            }
        }

        void JustDied(Unit* /*killer*/) // Otherwise, player is stuck with pet corpse they cannot remove from world
        {
            me->DespawnOrUnsummon(1);
        }

        void KilledUnit(Unit* /*victim*/)
        {
            Reset();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gilnean_mastiff_35631AI(creature);
    }
};

// 35463
class npc_bloodfang_lurker_35463 : public CreatureScript
{
public:
    npc_bloodfang_lurker_35463() : CreatureScript("npc_bloodfang_lurker_35463") {}

    struct npc_bloodfang_lurker_35463AI : public ScriptedAI
    {
        npc_bloodfang_lurker_35463AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tEnrage, tSeek;
        bool willCastEnrage;

        void Reset()
        {
            tEnrage           = 0;
            willCastEnrage    = urand(0, 1);
            tSeek             = urand(5000, 10000);
            DoCast(me, SPELL_SHADOWSTALKER_STEALTH);
        }

        void UpdateAI(uint32 diff)
        {
            if (tSeek <= diff)
            {
                if ((me->IsAlive()) && (!me->IsInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 2.0f)))
                    if (Player* player = me->SelectNearestPlayer(2.0f))
                    {
                        if (!player->IsInCombat())
                        {
                            me->AI()->AttackStart(player);
                            tSeek = urand(5000, 10000);
                        }
                    }
            }
            else tSeek -= diff;

            if (!UpdateVictim())
                return;

            if (tEnrage <= diff && willCastEnrage && me->GetHealthPct() <= 30)
            {
                me->MonsterTextEmote(-106, 0);
                DoCast(me, SPELL_ENRAGE);
                tEnrage = CD_ENRAGE;
            }
            else
                tEnrage -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodfang_lurker_35463AI(creature);
    }
};

// 67805
class spell_attack_lurker_67805 : public SpellScriptLoader
{
	public:
        spell_attack_lurker_67805() : SpellScriptLoader("spell_attack_lurker_67805"){}

        class spell_attack_lurker_67805_SpellScript : public SpellScript
		{
            PrepareSpellScript(spell_attack_lurker_67805_SpellScript);
			void HandleDummy(SpellEffIndex /*index*/)
			{
				Unit * caster = GetCaster();
				caster->CastSpell(caster,SPELL_ATTACK_LURKER,true);
				caster->SummonCreature(35463,caster->GetPositionX(),caster->GetPositionY(),caster->GetPositionZ(),caster->GetOrientation(),TEMPSUMMON_DEAD_DESPAWN);
			}
			void Register()
			{
                OnEffectHit += SpellEffectFn(spell_attack_lurker_67805_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
			}
		};

		SpellScript * GetSpellScript() const
		{
            return new spell_attack_lurker_67805_SpellScript();
		}
};

// 35550
class npc_king_genn_greymane_35550 : public CreatureScript
{
public:
    npc_king_genn_greymane_35550() : CreatureScript("npc_king_genn_greymane_35550") {}

    struct npc_king_genn_greymane_35550AI : public ScriptedAI
    {
        npc_king_genn_greymane_35550AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSummon, tSay;
        bool EventActive, RunOnce;

        void Reset()
        {
            tSay            = urand(10000, 20000);
            tSummon         = urand(3000, 5000); // How often we spawn
        }

        void SummonNextWave()
        {
            switch (urand (1,4))
            {
                case (1):
                    for (int i = 0; i < 5; i++)
                        me->SummonCreature(NPC_BLOODFANG_RIPPER_P4, -1781.173f + irand(-15, 15), 1372.90f + irand(-15, 15), 19.7803f, urand(0, 6), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 15000);
                    break;
                case (2):
                    for (int i = 0; i < 5; i++)
                        me->SummonCreature(NPC_BLOODFANG_RIPPER_P4, -1756.30f + irand(-15, 15), 1380.61f + irand(-15, 15), 19.7652f, urand(0, 6), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 15000);
                    break;
                case (3):
                    for (int i = 0; i < 5; i++)
                        me->SummonCreature(NPC_BLOODFANG_RIPPER_P4, -1739.84f + irand(-15, 15), 1384.87f + irand(-15, 15), 19.841f, urand(0, 6), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 15000);
                    break;
                case (4):
                    for (int i = 0; i < 5; i++)
                        me->SummonCreature(NPC_BLOODFANG_RIPPER_P4, -1781.173f + irand(-15, 15), 1372.90f + irand(-15, 15), 19.7803f, urand(0, 6), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 15000);
                    break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (tSay <= diff) // Time for next spawn wave
            {
                Talk(SAY_KING_GENN_GREYMANE);
                tSay = urand(10000, 20000);
            }
            else tSay -= diff;

            if (tSummon <= diff) // Time for next spawn wave
            {
                SummonNextWave(); // Activate next spawn wave
                tSummon = urand(3000, 5000); // Reset our spawn timer
            }
            else tSummon -= diff;
        }

        void JustSummoned(Creature* summoned)
        {
            summoned->GetDefaultMovementType();
            summoned->SetReactState(REACT_AGGRESSIVE);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_king_genn_greymane_35550AI(creature);
    }
};

// 35905
class npc_king_greymanes_horse_35905 : public CreatureScript
{
public:
    npc_king_greymanes_horse_35905() : CreatureScript("npc_king_greymanes_horse_35905") { }

    struct npc_king_greymanes_horse_35905AI : public npc_escortAI
    {
        npc_king_greymanes_horse_35905AI(Creature* creature) : npc_escortAI(creature) {}

        uint32 krennansay;
        bool PlayerOn, KrennanOn;

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset()
        {
             krennansay     = 500;//Check every 500ms initially
             PlayerOn       = false;
             KrennanOn      = false;
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                PlayerOn = true;
                if (apply)
                {
                    Start(false, true, who->GetGUID());
                }
            }
            else if (who->GetTypeId() == TYPEID_UNIT)
            {
                KrennanOn = true;
                SetEscortPaused(false);
            }
        }

        void WaypointReached(uint32 i)
        {
            Player* player = GetPlayerForEscort();

            switch(i)
            {
                case 5:
                    Talk(SAY_GREYMANE_HORSE, player);
                    me->GetMotionMaster()->MoveJump(-1679.089f, 1348.42f, 15.31f, 25.0f, 15.0f);
                    if (me->GetVehicleKit()->HasEmptySeat(1))
                    {
                        SetEscortPaused(true);
                        break;
                    }
                    else
                    break;
                case 12:
                    player->ExitVehicle();
                    player->SetClientControl(me, 1);
                    break;
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            if (Player* player = GetPlayerForEscort())
               player->FailQuest(QUEST_SAVE_KRENNAN_ARANAS);
        }

        void OnCharmed(bool /*apply*/)
        {
        }

        void UpdateAI(uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);
            Player* player = GetPlayerForEscort();

            if (PlayerOn)
            {
                player->SetClientControl(me, 0);
                PlayerOn = false;
            }

            if (KrennanOn) // Do Not yell for help after krennan is on
                return;

            if (krennansay <=diff)
            {
                if (Creature* krennan = me->FindNearestCreature(NPC_KRENNAN_ARANAS_TREE, 70.0f, true))
                {
                    krennan->AI()->Talk(SAY_NPC_KRENNAN_ARANAS_TREE, player);
                    krennansay = urand(4000,7000);//Repeat every 4 to 7 seconds
                }
            }
            else
                krennansay -= diff;
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_king_greymanes_horse_35905AI(creature);
    }
};

// 35907
class npc_krennan_aranas_35907 : public CreatureScript
{
public:
    npc_krennan_aranas_35907() : CreatureScript("npc_krennan_aranas_35907") { }

    struct npc_krennan_aranas_35907AI : public ScriptedAI
    {
        npc_krennan_aranas_35907AI(Creature* creature) : ScriptedAI(creature) {}

        bool Say, Move, Cast, KrennanDead;
        uint32 SayTimer;

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset()
        {
            Say             = false;
            Move            = true;
            Cast            = true;
            KrennanDead     = false;
            SayTimer        = 500;
        }

        void UpdateAI(uint32 diff)
        {
            if (Creature* krennan = me->FindNearestCreature(NPC_KRENNAN_ARANAS_TREE, 50.0f))
            {
                if (!KrennanDead)
                {
                    krennan->DespawnOrUnsummon(0);
                    KrennanDead = true;
                }
            }

            if (Creature* horse = me->FindNearestCreature(NPC_GREYMANE_HORSE_P4, 20.0f))//Jump onto horse in seat 2
            {
                if (Cast)
                {
                    DoCast(horse, 84275, true);
                }

                if (me->HasAura(84275))
                {
                    Cast = false;
                }
            }

            if (!me->HasAura(84275) && Move)
            {
                me->NearTeleportTo(KRENNAN_END_X, KRENNAN_END_Y, KRENNAN_END_Z, KRENNAN_END_O);
                Say = true;
                Move = false;
                SayTimer = 500;
            }

            if (Say && SayTimer <= diff)
            {
                Talk(SAY_KRENNAN_C2);
                me->DespawnOrUnsummon(6000);
                Say = false;
            }
            else
                SayTimer -= diff;
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_krennan_aranas_35907AI(creature);
    }
};

// 35914
class npc_commandeered_cannon_35914 : public CreatureScript
{
public:
    npc_commandeered_cannon_35914() : CreatureScript("npc_commandeered_cannon_35914") { }

    struct npc_commandeered_cannon_35914AI : public ScriptedAI
    {
        npc_commandeered_cannon_35914AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tEvent;
        uint8 Count, Phase;
        bool EventStart;

        void Reset()
        {
            tEvent          = 1400;
            Phase           = 0;
            Count           = 0;
            EventStart      = false;
        }

        void UpdateAI(uint32 diff)
        {
            if (!EventStart)
                return;

            if (Count > 2)
            {
                Reset();
                return;
            }

            if (tEvent <= diff)
            {
                switch (Phase)
                {
                case (0):
                    for (int i = 0; i < 12; i++)
                    {
                        me->SummonCreature(NPC_BLOODFANG_WORGEN, -1757.65f + irand(-6, 6), 1384.01f + irand(-6, 6), 19.872f, urand(0, 6), TEMPSUMMON_TIMED_DESPAWN, 5000);
                    }
                    tEvent = 400;
                    Phase++;
                    break;

                case (1):
                    if (Creature* Worgen = me->FindNearestCreature(NPC_BLOODFANG_WORGEN, 50.0f, true))
                    {
                        me->CastSpell(Worgen, SPELL_CANNON_FIRE, true);
                        tEvent = 1700;
                        Phase = 0;
                        Count++;
                    }
                    break;
                }
            } else tEvent -= diff;
        }

        void JustSummoned(Creature* summon)
        {
            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_commandeered_cannon_35914AI(creature);
    }
};

// 35906
class npc_lord_godfrey_35906 : public CreatureScript
{
public:
    npc_lord_godfrey_35906() : CreatureScript("npc_lord_godfrey_35906") { }

    bool OnQuestReward(Player* player, Creature* godfrey, Quest const* quest, uint32 opt)
    {
        if (quest->GetQuestId() == QUEST_SAVE_KRENNAN_ARANAS)
        {
            godfrey->AI()->Talk(SAY_LORD_GODFREY_P4);
            player->RemoveAura(SPELL_WORGEN_BITE);
            godfrey->AddAura(SPELL_INFECTED_BITE, player);
            player->RemoveAura(76642);// Only Infected bite aura should be added
            player->CastSpell(player, SPELL_GILNEAS_CANNON_CAMERA);
            player->SaveToDB();
            if (Creature* cannon = GetClosestCreatureWithEntry(godfrey, NPC_COMMANDEERED_CANNON, 50.0f))
            {
                CAST_AI(npc_commandeered_cannon_35914::npc_commandeered_cannon_35914AI, cannon->AI())->EventStart = true; // Start Event
            }
        }
        return true;
    }
};


// 35552
class npc_lord_darius_crowley_35552 : public CreatureScript
{
public:
    npc_lord_darius_crowley_35552() : CreatureScript("npc_lord_darius_crowley_35552") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const*_Quest)
    {
        if (_Quest->GetQuestId() == QUEST_SACRIFICES)
        {
            if (Creature *horse = player->SummonCreature(35231, player->GetPositionX(), player->GetPositionY(),  player->GetPositionZ(),  player->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
            {
                player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, horse, false);
                CAST_AI(npc_escortAI, (horse->AI()))->Start(false, true, player->GetGUID(), _Quest);
            }
        }
        return true;
    }
};

// 35231
class npc_crowley_horse_35231 : public CreatureScript
{
public:
    npc_crowley_horse_35231() : CreatureScript("npc_crowley_horse_35231") {}

    struct npc_crowley_horse_35231AI : public npc_escortAI
    {
        npc_crowley_horse_35231AI(Creature* creature) : npc_escortAI(creature) {}

        bool CrowleyOn;
        bool CrowleySpawn;
        bool Run;

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset()
        {
            CrowleyOn = false;
            CrowleySpawn = false;
            Run = false;
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                if (apply)
                {
                    Start(false, true, who->GetGUID());
                }
            }
        }

        void WaypointReached(uint32 i)
        {
            Player* player = GetPlayerForEscort();
            Creature* crowley = me->FindNearestCreature(NPC_DARIUS_CROWLEY, 5, true);

            switch(i)
            {
                case 1:
                    player->SetClientControl(me, 0);
                    crowley->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    me->GetMotionMaster()->MoveJump(-1714.02f, 1666.37f, 20.57f, 25.0f, 15.0f);
                    break;
                case 4:
                    crowley->AI()->Talk(SAY_CROWLEY_HORSE_1);
                    break;
                case 10:
                    me->GetMotionMaster()->MoveJump(-1571.32f, 1710.58f, 20.49f, 25.0f, 15.0f);
                    break;
                case 11:
                    crowley->AI()->Talk(SAY_CROWLEY_HORSE_2);
                    break;
                case 16:
                    crowley->AI()->Talk(SAY_CROWLEY_HORSE_2);
                    break;
                case 20:
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    me->getThreatManager().resetAllAggro();
                    player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    player->getThreatManager().resetAllAggro();
                    break;
                case 21:
                    player->SetClientControl(me, 1);
                    player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    player->ExitVehicle();
                    break;
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            if (Player* player = GetPlayerForEscort())
               player->FailQuest(QUEST_SACRIFICES);
        }

        void OnCharmed(bool /*apply*/)
        {
        }

        void UpdateAI(uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);

            if (!CrowleySpawn)
            {
                DoCast(SPELL_SUMMON_CROWLEY);
                if (Creature* crowley = me->FindNearestCreature(NPC_DARIUS_CROWLEY, 5, true))
                {
                    CrowleySpawn = true;
                }
            }

            if (CrowleySpawn && !CrowleyOn)
            {
                Creature* crowley = me->FindNearestCreature(NPC_DARIUS_CROWLEY, 5, true);
                crowley->CastSpell(me, SPELL_RIDE_HORSE, true);//Mount Crowley in seat 1
                CrowleyOn = true;
            }

            if (!Run)
            {
                me->SetSpeed(MOVE_RUN, CROWLEY_SPEED);
                Run = true;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_crowley_horse_35231AI(creature);
    }
};

// 35229
class npc_bloodfang_stalker_35229 : public CreatureScript
{
public:
    npc_bloodfang_stalker_35229() : CreatureScript("npc_bloodfang_stalker_35229") {}

    struct npc_bloodfang_stalker_35229AI : public ScriptedAI
    {
        npc_bloodfang_stalker_35229AI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            tEnrage    = 0;
            tAnimate   = DELAY_ANIMATE;
            Miss  = false;
            willCastEnrage = urand(0, 1);
            BurningReset = 3000;
            Burning = false;
        }

        void UpdateAI(uint32 diff)
        {
            if(me->HasAura(SPELL_THROW_TORCH))
                Burning = true;
            else
                Burning = false;

            if (Burning && BurningReset <= diff)
            {
                me->RemoveAllAuras();
                BurningReset = 5000;
                Burning = false;
            }
            else
                BurningReset -= diff;

            if (!UpdateVictim())
                return;

            if (tEnrage <= diff && willCastEnrage)
            {
                if (me->GetHealthPct() <= 30)
                {
                    me->MonsterTextEmote(-106, 0);
                    DoCast(me, SPELL_ENRAGE);
                    tEnrage = CD_ENRAGE;
                }
            }
            else tEnrage -= diff;

            if (me->GetVictim()->GetTypeId() == TYPEID_PLAYER)
                Miss = false;
            else if (me->GetVictim()->IsPet())
                Miss = false;
            else if (me->GetVictim()->GetEntry() == NPC_NORTHGATE_REBEL_1)
                if (me->GetVictim()->GetHealthPct() < 90)
                    Miss = true;

            if (Miss && tAnimate <= diff)
            {
                me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK_UNARMED);
                me->PlayDistanceSound(SOUND_WORGEN_ATTACK);
                tAnimate = DELAY_ANIMATE;
            }
            else
                tAnimate -= diff;

            if (!Miss)
                DoMeleeAttackIfReady();
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            Creature *horse = me->FindNearestCreature(NPC_CROWLEY_HORSE, 100, true);
            if (spell->Id == SPELL_THROW_TORCH)
            {
                Burning = true;
                if (caster->GetTypeId() == TYPEID_PLAYER && caster->ToPlayer()->GetQuestStatus(QUEST_SACRIFICES) == QUEST_STATUS_INCOMPLETE)
                {
                    caster->ToPlayer()->KilledMonsterCredit(NPC_BLOODFANG_STALKER_CREDIT, 0);
                    me->DespawnOrUnsummon(5000);
                }
            }
        }

    private :
        Player* player;
        uint32 tEnrage;
        uint32 tAnimate;
        uint32 BurningReset;
        bool Miss, willCastEnrage, Burning;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodfang_stalker_35229AI(creature);
    }
};

// 35566
class npc_lord_darius_crowley_35566 : public CreatureScript
{
public:
    npc_lord_darius_crowley_35566() : CreatureScript("npc_lord_darius_crowley_35566") {}

    bool OnQuestReward(Player* player, Creature* creature, const Quest *_Quest, uint32 )
    {
        if (_Quest->GetQuestId() == 14222)
        {
            WorldLocation loc;
            loc.m_mapId       = 654;
            loc.m_positionX   = -1818.4f;
            loc.m_positionY   = 2294.25f;
            loc.m_positionZ   = 42.2135f;
            loc.m_orientation = 3.14f;

            player->SetHomebind(loc, 4786);

            player->CastSpell(player, 94293, true);
            player->CastSpell(player, 68996, true);
            player->CastSpell(player, 69196, true);
            player->CastSpell(player, 72788, true);
            //player->CastSpell(player, 72794, true);
			player->RemoveAura(72872);
            player->TeleportTo(loc);
        }
        return true;
    }
};

class PlayerAlreadyStartWorgenZone2
{
public:
    PlayerAlreadyStartWorgenZone2() { }

    bool operator()(Player* player) const
    {
        return (player->GetQuestStatus(14375) != QUEST_STATUS_NONE || player->HasAura(68630));
    }
};



/*###### Start of DuskHaven ######*/

/*######
## npc_trigger_event_c3
######*/
class npc_trigger_event_c3 : public CreatureScript
{
public:
    npc_trigger_event_c3() : CreatureScript("npc_trigger_event_c3") {}

    struct npc_trigger_event_c3AI : public ScriptedAI
    {
        npc_trigger_event_c3AI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            mui_refreshEvent = 1000;
            canStartEvent = false;
        }

        void UpdateAI(uint32 diff)
        {
            if (mui_refreshEvent <= diff)
            {
                std::list<Player *> _players = me->FindNearestPlayers(10.0f, true);
                _players.remove_if(PlayerAlreadyStartWorgenZone2());
                bool endLoop = true;
                for (std::list<Player *>::const_iterator itr = _players.begin(); itr != _players.end(); itr++)
                {
                    if (!canStartEvent)
                    {
                        canStartEvent = true;
                        break;
                    }
                    endLoop = false;
                    (*itr)->CastSpell((*itr), 68630, true); // wird von 69251 getriggert
                    (*itr)->CastSpell((*itr), 72799, true); // spell Last Stand Complete
                }
                if (!endLoop)
                    canStartEvent = false;
                mui_refreshEvent = 3000;
            }
            else
                mui_refreshEvent -= diff;
        }

    private :
        uint32 mui_refreshEvent;
        bool canStartEvent;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_trigger_event_c3AI(creature);
    }
};

// 36332 wrong
class npc_king_genn_greymane : public CreatureScript
{
public:
    npc_king_genn_greymane() : CreatureScript("npc_king_genn_greymane") {}

    bool OnQuestReward(Player* player, Creature* creature, Quest const* _Quest, uint32)
    {
        if (_Quest->GetQuestId() == 14375)
        {
            player->RemoveAurasDueToSpell(68630);
            player->RemoveAurasDueToSpell(69196);
            player->CastSpell(player, SPELL_FADE_BACK, true);
            player->CastSpell(player, 72799, true);
            creature->DespawnOrUnsummon();
        }
        return true;
    }
};

/*######
## npc_trigger_event_first_c3
######*/
class npc_trigger_event_first_c3 : public CreatureScript
{
public:
    npc_trigger_event_first_c3() : CreatureScript("npc_trigger_event_first_c3") {}

    struct npc_trigger_event_first_c3AI : public ScriptedAI
    {
        npc_trigger_event_first_c3AI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            godfreyGUID = 0;
            gennGUID = 0;
            if (Creature *godfrey = me->SummonCreature(36330, -1844.29f, 2290.67f, 42.30f, 0.32f, TEMPSUMMON_MANUAL_DESPAWN))
                godfreyGUID = godfrey->GetGUID();
            if (Creature *genn = me->SummonCreature(36332, -1844.29f, 2290.67f, 42.30f, 0.32f, TEMPSUMMON_MANUAL_DESPAWN))
            {
                gennGUID = genn->GetGUID();
                genn->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            }
            endEvent = false;
            mui_timer_event = 12500;
            step = 0;
            me->GetMotionMaster()->MovePoint(42, -1819.72f, 2289.23f, 42.28f);
            waitingForEndMovement = false;
        }

        void MovementInform(uint32 type, uint32 point)
        {
			Creature* godfrey = ObjectAccessor::GetCreature(*me, godfreyGUID);
			Creature* genn = ObjectAccessor::GetCreature(*me, gennGUID);
            switch (point)
            {
                case 0:
                    waitingForEndMovement = false;
                    Talk(0);
                    break;
                case 42:
                    me->SetFacingTo(1.75f);
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (waitingForEndMovement)
                return;
            if (mui_timer_event <= diff)
            {
				Creature* godfrey = ObjectAccessor::GetCreature(*me, godfreyGUID);
				Creature* genn = ObjectAccessor::GetCreature(*me, gennGUID);
                if (!genn)
                    if (godfrey)
                        godfrey->DespawnOrUnsummon();
                if (!godfrey || !genn)
                {
                    me->DespawnOrUnsummon();
                    return ;
                }
                switch (step)
                {
                  case 0:
                      me->GetMotionMaster()->MovePoint(step, -1819.80f, 2290.40f, 42.22f);
                      waitingForEndMovement = true;
                      step++;
                      break;
                  case 1:
                      godfrey->GetMotionMaster()->MovePoint(step, -1822.14f, 2295.40f, 42.1f);
                      mui_timer_event = 4000;
                      step++;
                      break;
                  case 2:
                      godfrey->SetFacingToObject(me);
                      godfrey->AI()->Talk(0);
                      mui_timer_event = 4000;
                      step++;
                      break;
                  case 3:
                      genn->GetMotionMaster()->MovePoint(step, -1821.69f, 2292.76f, 42.08f);
                      mui_timer_event = 4000;
                      step++;
                      break;
                  case 4:
                      genn->SetFacingToObject(godfrey);
                      genn->AI()->Talk(0);
                      mui_timer_event = 8000;
                      step++;
                      break;
                  case 5:
                      genn->AI()->Talk(1);
                      genn->SetFacingToObject(me);
                      mui_timer_event = 4000;
                      step++;
                      break;
                  case 6:
                      genn->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                      mui_timer_event = 300000;
                      endEvent = true;
                      step++;
                      break;
                  case 7:
                      godfrey->DespawnOrUnsummon();
                      me->DespawnOrUnsummon();
                      step++;
                  default:
                      break;
                }
            }
            else
                mui_timer_event -= diff;
        }

    private :
        uint64 godfreyGUID;
        uint64 gennGUID;
        bool endEvent;
        uint32 mui_timer_event;
        uint32 step;
        bool waitingForEndMovement;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_trigger_event_first_c3AI(creature);
    }
};

// 196394
class go_mandragore_196394 : public GameObjectScript
{
public:
    go_mandragore_196394() : GameObjectScript("go_mandragore_196394") {}

    bool OnQuestReward(Player* player, GameObject * , Quest const* _Quest, uint32)
    {
        if (_Quest->GetQuestId() == 14320)
        {
            player->SendCinematicStart(168);
            WorldPacket data(SMSG_PLAY_SOUND, 4);
            data << uint32(23676);
            player->GetSession()->SendPacket(&data);
        }
        return true;
    }
};

// 36231 Quest - You Can't Take 'Em Alone - 14348
class npc_horrid_abomination_36231 : public CreatureScript
{
public:
    npc_horrid_abomination_36231() : CreatureScript("npc_horrid_abomination_36231") { }

    enum eHorrid
    {
        SAY_BARREL = 0
    };

    struct npc_horrid_abomination_36231AI : public ScriptedAI
    {
        npc_horrid_abomination_36231AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 DieTimer;

        void Reset ()
        {
            me->ClearUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
            DieTimer = 5000;
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_BARREL_KEG && caster->GetTypeId() == TYPEID_PLAYER && caster->ToPlayer()->GetQuestStatus(QUEST_YOU_CANT_TAKE_EM_ALONE) == QUEST_STATUS_INCOMPLETE)
            {
                caster->ToPlayer()->KilledMonsterCredit(QUEST_14348_KILL_CREDIT, 0);
                me->AddUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
                Talk(SAY_BARREL);

            }
        }

        void UpdateAI(uint32 diff)
        {
            if (DieTimer <= diff)
            {
                if (me->HasAura(SPELL_BARREL_KEG))
                    me->DisappearAndDie();
                else
                    DieTimer = 1000;
            }
            else
                DieTimer -= diff;

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_horrid_abomination_36231AI(creature);
    }
};

// 68555
class spell_keg_placed_68555 : public SpellScriptLoader
{
    public:
        spell_keg_placed_68555() : SpellScriptLoader("spell_keg_placed_68555") {}

        class spell_keg_placed_68555_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_keg_placed_68555_AuraScript);

            void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                tick = urand(1, 4);
                tickcount = 0;
            }

            void HandlePeriodic(AuraEffect const* aurEff)
            {
                PreventDefaultAction();
                if (Unit* caster = GetCaster())
                {
                    if (tickcount > tick)
                    {
                        if (caster->GetTypeId() != TYPEID_PLAYER)
                            return;
                        caster->ToPlayer()->KilledMonsterCredit(36233, 0);
                        if (Unit* target = GetTarget())
                            target->Kill(target);
                    }
                    tickcount++;
                }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_keg_placed_68555_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_keg_placed_68555_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_DUMMY);
            }

        private :
            uint32 tick;
            uint32 tickcount;

        };

        AuraScript* GetAuraScript() const
        {
            return new spell_keg_placed_68555_AuraScript();
        }
};

// 68903
class spell_round_up_horse_68903 : public SpellScriptLoader
{
public:
    spell_round_up_horse_68903() : SpellScriptLoader("spell_round_up_horse_68903") { }

    class spell_round_up_horse_68903_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_round_up_horse_68903_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(68903))
                return false;
            return true;
        }

        void HandleEffectDummy(SpellEffIndex /*effIndex*/)
        {
            if (GetHitUnit()->GetTypeId() != TYPEID_UNIT || GetCaster()->GetTypeId() != TYPEID_PLAYER || GetCaster()->ToPlayer()->GetQuestStatus(14416) != QUEST_STATUS_INCOMPLETE)
                return ;
            GetHitUnit()->ToCreature()->DespawnOrUnsummon();
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_round_up_horse_68903_SpellScript::HandleEffectDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_round_up_horse_68903_SpellScript();
    }
};

/*###### Quest Gasping for Breath  ######*/

// 68735
class spell_rescue_drowning_watchman_68735 : public SpellScriptLoader
{
public:
    spell_rescue_drowning_watchman_68735() : SpellScriptLoader("spell_rescue_drowning_watchman_68735") { }

    class spell_rescue_drowning_watchman_68735_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_rescue_drowning_watchman_68735_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(68735))
                return false;
            return true;
        }

        void HandleEffectDummy(SpellEffIndex /*effIndex*/)
        {
            if (GetCaster()->GetTypeId() != TYPEID_PLAYER || GetHitUnit()->GetTypeId() != TYPEID_UNIT || GetCaster()->GetVehicleKit() == NULL)
                return ;
            if (GetCaster()->ToPlayer()->GetQuestStatus(14395) != QUEST_STATUS_INCOMPLETE)
            {
                GetCaster()->RemoveAurasDueToSpell(68735);
                return;
            }
            if (GetCaster()->IsInWater())
                GetHitUnit()->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, GetCaster(), false);
            else if (GetHitUnit()->GetVehicle())
            {
                GetCaster()->GetVehicleKit()->RemoveAllPassengers();
                GetHitUnit()->RemoveAurasDueToSpell(68730);
                GetHitUnit()->CastSpell(GetHitUnit(), 68442, true);
                GetCaster()->ToPlayer()->KilledMonsterCredit(36440,0);
                GetCaster()->RemoveAurasDueToSpell(68735);
                GetHitUnit()->ToCreature()->DespawnOrUnsummon(5000);
                GetHitUnit()->ToCreature()->AI()->Talk(0);
            }
            else
                GetCaster()->RemoveAurasDueToSpell(68735);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_rescue_drowning_watchman_68735_SpellScript::HandleEffectDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_rescue_drowning_watchman_68735_SpellScript();
    }
};

/*###### Quest Gasping for Breath END  ######*/

/*###### Losing Your Tail ######*/

/*######
## npc_trigger_quest_24616
######*/
class npc_trigger_quest_24616 : public CreatureScript
{
public:
    npc_trigger_quest_24616() : CreatureScript("npc_trigger_quest_24616") { }

    struct npc_trigger_quest_24616AI : public ScriptedAI
    {
        npc_trigger_quest_24616AI(Creature* creature) : ScriptedAI(creature)  { }

        void Reset()
        {
            mui_timerAllowSummon = urand(3000, 5000);
            allowSummon = false;
            playerGUID = 0;
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (who->GetTypeId() != TYPEID_PLAYER)
                return ;
            if (who->ToPlayer()->GetQuestStatus(24616) != QUEST_STATUS_INCOMPLETE || me->FindNearestCreature(37953, 100, false) != NULL)
                return ;
            if (me->IsWithinDistInMap(who, 20.0f))
            {
                allowSummon = true;
                playerGUID = who->GetGUID();
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!allowSummon)
                return;
            if (mui_timerAllowSummon <= diff)
            {
				if (Player *player = ObjectAccessor::GetPlayer(*me, playerGUID))
                    if (me->FindNearestCreature(37953, 100) == NULL)
                        me->CastSpell(player, 70794, true);
                allowSummon = false;
                playerGUID = 0;
                mui_timerAllowSummon = urand(3000, 5000);
            }
            else mui_timerAllowSummon -= diff;
        }

    private :
        uint32 mui_timerAllowSummon;
        bool allowSummon;
        uint64 playerGUID;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_trigger_quest_24616AI(creature);
    }
};

// 38553
class npc_krennan_aranas_38553 : public CreatureScript
{
public:
    npc_krennan_aranas_38553() : CreatureScript("npc_krennan_aranas_38553") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (uiAction)
        {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->CLOSE_GOSSIP_MENU();
            CAST_AI(npc_krennan_aranas_38553::npc_krennan_aranas_38553AI, creature->AI())->StartBattle(player);
            break;

        case GOSSIP_ACTION_INFO_DEF+2:
            player->CLOSE_GOSSIP_MENU();
            CAST_AI(npc_krennan_aranas_38553::npc_krennan_aranas_38553AI, creature->AI())->EndBattle();
            break;
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(QUEST_THE_BATTLE_FOR_GILNEAS_CITY) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "start battle !", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        if (player->IsGameMaster())
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "(GM ONLY) RESET EVENT!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);

        player->SEND_GOSSIP_MENU(2474, creature->GetGUID());

        return true;
    }

    struct npc_krennan_aranas_38553AI : public ScriptedAI
    {
        npc_krennan_aranas_38553AI(Creature *c) : ScriptedAI(c) {}

        uint64 princeGUID;

        void Reset()
        {
            princeGUID = 0;
            if (Creature* newPrince = me->SummonCreature(NPC_PRINCE_LIAM_GREYMANE_BATTLE, -1408.661f, 1260.017f, 36.51123f, 1.79f, TEMPSUMMON_DEAD_DESPAWN, 180000))
                princeGUID = newPrince->GetGUID();
        }

        void StartBattle(Player* player)
        {
			if (Creature* prince = ObjectAccessor::GetCreature(*me, princeGUID))
            {
                if (me->GetDistance2d(prince->GetPositionX(), prince->GetPositionY()) > 50)
                    me->MonsterSay("event already started please wait a minute.", LANG_UNIVERSAL, 0);
                else
                {
                    prince->AI()->DoAction(ACTION_START_EVENT);
                    Talk(0, player);
                }
            }
            else
            {
                princeGUID = 0;
                if (Creature* newPrince = me->SummonCreature(NPC_PRINCE_LIAM_GREYMANE_BATTLE, -1408.661f, 1260.017f, 36.51123f, 1.79f, TEMPSUMMON_DEAD_DESPAWN, 180000))
                {
                    princeGUID = newPrince->GetGUID();
                    prince->AI()->DoAction(ACTION_START_EVENT);
                    Talk(0, player);
                }
            }
        }

        void EndBattle()
        {
			if (Creature* prince = ObjectAccessor::GetCreature(*me, princeGUID))
                prince->DespawnOrUnsummon();
            princeGUID = 0;
            if (Creature* newPrince = me->SummonCreature(NPC_PRINCE_LIAM_GREYMANE_BATTLE, -1408.661f, 1260.017f, 36.51123f, 1.79f, TEMPSUMMON_DEAD_DESPAWN, 180000))
                princeGUID = newPrince->GetGUID();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_krennan_aranas_38553AI(pCreature);
    }
};

/*######
## npc_prince_liam_greymane_gilneas
######*/
class npc_prince_liam_greymane_gilneas : public CreatureScript
{
public:    npc_prince_liam_greymane_gilneas() : CreatureScript("npc_prince_liam_greymane_gilneas") {}

    struct npc_prince_liam_greymane_gilneasAI : public npc_escortAI
    {
        npc_prince_liam_greymane_gilneasAI(Creature *c) : npc_escortAI(c) , summons(me) { }

        void Reset()
        {
            me->CastSpell(me, 72069, true);

            startEvent = false;
            intro = true;
            canFollow = false;
            phase = 1;
            intoTimer = 5000;
            checkTimer = 3000;
            summons.DespawnAll();
            float summon_x = me->GetPositionX() - 6;
            float summon_y = me->GetPositionY() - 4;

            int j = 0;
            int row = 0;

            for (int i = 0; i < 70; i++)
            {
                if (j > 6)
                {
                    j = 0;
                    row++;
                    summon_y -= 3;
                    summon_x = me->GetPositionX() - 6;
                }

                if (Creature * milit = me->SummonCreature(38221, (summon_x+cos(5.42f)*row), (summon_y-sin(5.42f)*j), 36.51123f, 1.79f, TEMPSUMMON_MANUAL_DESPAWN))
                {
                    militiamans[i] = milit->GetGUID();
                    if (i % 2 == 0)
                        milit->AI()->SetData(DATA_GROUP, GROUP_1);
                    else
                        milit->AI()->SetData(DATA_GROUP, GROUP_1); // need to be improved...
                }
                summon_x += 2;
                j++;
            }
            mui_moveTimer = 1000;
        }

        void JustDied(Unit* /*killer*/)
        {
            summons.DespawnAll();
        }

        bool CanFollow() { return canFollow; }

        void EnterCombat(Unit* pWho)
        {
            Talk(10);
        }

        void moveGroup(uint32 group, float x, float y, float z)
        {
            for (int i = 0; i < 70; i++)
				if (Creature *milit = ObjectAccessor::GetCreature(*me, militiamans[i]))
                    if (milit->AI()->GetData(DATA_GROUP) == group)
                    {
                        milit->SetHomePosition(x, y, z, milit->GetOrientation());
                        milit->GetMotionMaster()->MovePoint(0, x, y, z);
                    }
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/)
        {
            summons.Despawn(summon);
        }

        void WaypointReached(uint32 point)
        {
            RefreshMovementMilitary(GROUP_1);
            switch (point)
            {
            case 1:
                moveGroup(GROUP_2, -1487.67f, 1345.69f, 35.56f);
                break;
            case 3:
                moveGroup(GROUP_2, -1551.90f, 1327.45f, 35.56f);
                break;
            case 5:
                Talk(7);
                if (Creature *c = me->SummonCreature(38426, -1549.20f, 1285.96f, 11.78f, 3.47f, TEMPSUMMON_MANUAL_DESPAWN))
                    c->AI()->DoAction(ACTION_START_EVENT);
                SetEscortPaused(true);
                break;
            case 6:
                moveGroup(GROUP_2, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                Talk(8);
                break;
            case 7 :
                moveGroup(GROUP_2, -1657.01f, 1337.70f, 15.135f);
                break;
            case 8: //crowley
                moveGroup(GROUP_2, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                Talk(9);
                break;
            case 10:
            case 11:
            case 12:
                moveGroup(GROUP_2, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                break;
            case 13:
                if (Creature *abo = me->SummonCreature(38331, -1710.466f, 1407.905f, 21.75196f, -1.89314f, TEMPSUMMON_DEAD_DESPAWN, 60000))
                {
                    abo->GetMotionMaster()->MoveJump(-1710.466f, 1407.905f, 21.75196f, 20.0f, 20.0f);
                    if (Creature *darius = me->SummonCreature(38415, -1771.81f, 1341.33f, 19.75, 0.81314f, TEMPSUMMON_MANUAL_DESPAWN))
                        darius->AI()->SetGUID(abo->GetGUID());
                }
                break;
            case 14 :
                moveGroup(GROUP_2, -1761.24f, 1512.72f, 26.22f);
                break;
            case 15:
                for (int i = 0; i < 70; i++)
					if (Creature *milit = ObjectAccessor::GetCreature(*me, militiamans[i]))
                        milit->AI()->SetData(DATA_GROUP, GROUP_1);
                break;
            case 19:
                if (Creature *c = me->FindNearestCreature(38470, 100, true))
                    c->AI()->SetData(0, 1);
                summons.DespawnAll();
                me->DespawnOrUnsummon();
                break;
            }
        }

        void RefreshMovementMilitary(uint32 group)
        {
            float x, y, z;
            for (int i = 0; i < 70; i++)
            {
                float distance = urand(0, 7);
                float angle = frand(-3 * M_PI / 4, 3 * M_PI / 2);
                me->GetNearPoint(me, x, y, z, me->GetObjectSize(), distance, angle);
				if (Creature *milit = ObjectAccessor::GetCreature(*me, militiamans[i]))
                    if (milit->AI()->GetData(DATA_GROUP) == group)
                        milit->SetHomePosition(x, y, z, me->GetOrientation());
            }
        }

        void DoAction(int32 info)
        {
            switch (info)
            {
                case ACTION_START_EVENT:
                    startEvent = true;
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!startEvent)
                return;
            npc_escortAI::UpdateAI(diff);

            if (canFollow)
            {
                if (mui_moveTimer <= diff)
                {
                    RefreshMovementMilitary(GROUP_1);
                    mui_moveTimer = 10000;
                }
                else
                    mui_moveTimer -= diff;
            }

            if (intro)
            {
                if (intoTimer <= diff)
                {
                    switch (phase)
                    {
                    case 1:
                        Talk(0);
                        intoTimer = 10000;
                        break;
                    case 2:
                        Talk(1);
                        intoTimer = 10000;
                        break;
                    case 3:
                        Talk(2);
                        intoTimer = 12000;
                        break;
                    case 4:
                        Talk(3);
                        intoTimer = 10000;
                        break;
                    case 5:
                        Talk(4);
                        intoTimer = 12000;
                        break;
                    case 6:
                        Talk(5);
                        intoTimer = 5500;
                        break;
                    case 7:
                        Talk(6);
                        intoTimer = 1500;
                        Start(true, true);
                        SetDespawnAtEnd(true);
                        break;
                    case 8:
                        me->PlayDistanceSound(22584);
                        for (int i = 0; i < 70; i++)
							if (Creature *milit = ObjectAccessor::GetCreature(*me, militiamans[i]))
                            {
                                milit->HandleEmoteCommand(25);
                                float distance = urand(7, 15);
                                float angle = frand(-3 * M_PI / 4, 3 * M_PI / 4);
                                milit->GetMotionMaster()->MoveFollow(me, distance, angle, MOTION_SLOT_ACTIVE);
                            }
                        intro = false;
                        canFollow = true;
                        break;
                    }
                    phase++;
                } else intoTimer -= diff;
            }

            if (checkTimer <= diff)
            {
                if (!me->HasAura(72069))
                    me->CastSpell(me, 72069, true);
                checkTimer = 3000;
            } else checkTimer -= diff;

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

    private :
        bool intro;
        bool canFollow;
        uint32 phase;
        uint32 intoTimer;
        uint32 checkTimer;
        uint64 militiamans[70];
        uint32 mui_moveTimer;
        bool startEvent;
        SummonList summons;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_prince_liam_greymane_gilneasAI(creature);
    }
};

/*######
## npc_lorna_crowley_event
######*/
class npc_lorna_crowley_event : public CreatureScript
{
public:
    npc_lorna_crowley_event() : CreatureScript("npc_lorna_crowley_event") { }

    struct npc_lorna_crowley_eventAI : public ScriptedAI
    {
        npc_lorna_crowley_eventAI(Creature *c) : ScriptedAI(c) {}

        void Reset()
        {
            eventInProgress = false;
            mui_moveTimer = 5000;
            mui_attack = 8000;
        }

        void MovementInform(uint32 type, uint32 point)
        {
            if (!eventInProgress)
                Talk(0);
            eventInProgress = true;
            me->DespawnOrUnsummon(60000);
        }

        void DoAction(int32 info)
        {
            switch (info)
            {
                case ACTION_START_EVENT:
                    float summon_x = me->GetPositionX();
                    float summon_y = me->GetPositionY();
                    float summon_z = me->GetPositionZ();
                    for (int i = 0; i < 12; i++)
                        if (Creature * milit = me->SummonCreature(38425, summon_x, summon_y, summon_z, 0.0f, TEMPSUMMON_MANUAL_DESPAWN))
                        {
                            if (i == 0 || i == 1 || i == 2)
                                citizenGUID[i] = milit->GetGUID();
                            if (i != 0)
								if (Creature *firstCiti = ObjectAccessor::GetCreature(*me, citizenGUID[0]))
                                    milit->GetMotionMaster()->MoveFollow(firstCiti, urand(0, 5), frand(-3 * M_PI / 8, 3 * M_PI / 8), MOTION_SLOT_ACTIVE);
                        }
					if (Creature *firstCiti = ObjectAccessor::GetCreature(*me, citizenGUID[0]))
                        firstCiti->GetMotionMaster()->MovePoint(0, -1571.79f, 1318.833f, 35.56271f);
                    me->GetMotionMaster()->MovePoint(0, -1571.79f, 1318.833f, 35.56271f);
                    if (Creature *c = me->SummonCreature(38424, -1542.548f, 1290.957f, 6.239825f, 3.490021f, TEMPSUMMON_MANUAL_DESPAWN))
                    {
                        c->GetMotionMaster()->MovePoint(0, -1571.79f, 1318.833f, 35.56271f);
                        canonGUID = c->GetGUID();
                    }
                    if (Creature *c = me->SummonCreature(38424, -1541.036f, 1283.645f, 6.35803f,  3.343305f, TEMPSUMMON_MANUAL_DESPAWN))
                        c->GetMotionMaster()->MovePoint(0, -1571.79f, 1318.833f, 35.56271f);
                    if (Creature *c = me->SummonCreature(38424, -1543.06f,  1287.436f, 7.076612f, 3.446335f, TEMPSUMMON_MANUAL_DESPAWN))
                        c->GetMotionMaster()->MovePoint(0, -1571.79f, 1318.833f, 35.56271f);
                    break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!eventInProgress)
                return;

            if (mui_moveTimer <= diff)
            {
				if (Creature *canon = ObjectAccessor::GetCreature(*me, canonGUID))
                {
					if (Creature *firstCiti = ObjectAccessor::GetCreature(*me, citizenGUID[1]))
                        firstCiti->GetMotionMaster()->MovePoint(1, -1606.97f, 1313.59f, 18.99f);
					if (Creature *firstCiti = ObjectAccessor::GetCreature(*me, citizenGUID[2]))
                        firstCiti->GetMotionMaster()->MovePoint(1, -1610.97f, 1310.59f, 18.11f);
                    me->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, canon, false);
                    canon->GetMotionMaster()->MovePoint(1, -1609.97f, 1312.59f, 18.11f);
                    if (me->ToTempSummon())
                        if (Unit *general = me->ToTempSummon()->GetSummoner())
                        {
                            me->GetMotionMaster()->MoveFollow(general, urand(0, 5), frand(-3 * M_PI / 8, 3 * M_PI / 8), MOTION_SLOT_ACTIVE);
                            if (general->ToCreature())
                                CAST_AI(npc_escortAI, (general->ToCreature()->AI()))->SetEscortPaused(false);
                        }
                }
                mui_moveTimer = 3600000;
            }
            else  mui_moveTimer -= diff;

            if (mui_attack <= diff)
            {
                if (Creature *c = me->FindNearestCreature(38420, 200))
					if (Creature *canon = ObjectAccessor::GetCreature(*me, canonGUID))
                        canon->CastSpell(c, 72013, true);
                mui_attack = 5000;
            }
            else mui_attack -= diff;
        }

    private:
        bool eventInProgress;
        uint32 mui_moveTimer;
        uint32 mui_attack;
        uint64 canonGUID;
        uint64 citizenGUID[2];
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_lorna_crowley_eventAI(pCreature);
    }
};

// 38221
class npc_gilnean_militia_38221 : public CreatureScript
{
public:
    npc_gilnean_militia_38221() : CreatureScript("npc_gilnean_militia_38221") { }

    struct npc_gilnean_militia_38221AI : public ScriptedAI
    {
        npc_gilnean_militia_38221AI(Creature* pCreature) : ScriptedAI(pCreature) { }

        void Reset()
        {
            mui_spell1 = urand(1200, 5100);
            mui_spell2 = urand(2100, 5400);
            me->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
        }

        void AttackStart(Unit* who)
        {
            if (me->Attack(who, true))
                DoStartNoMovement(who);
        }

        void SetData(uint32 uiType, uint32 uiData)
        {
            if (uiType == DATA_GROUP)
                group = uiData;
        }

        uint32 GetData(uint32 uiType) const
        {
            if (uiType == DATA_GROUP)
                return group;
			return 0;
        }

        void JustReachedHome()
        {
            if (group == GROUP_1)
                if (!UpdateVictim())
                    if (me->ToTempSummon())
                        if (Unit *p = me->ToTempSummon()->GetSummoner())
                        {
                            float distance = urand(7, 15);
                            float angle = frand(-3 * M_PI / 4, 3 * M_PI / 2);
                            float x, y, z;
                            me->GetNearPoint(p, x, y, z, p->GetObjectSize(), distance, angle);
                            me->GetMotionMaster()->MoveFollow(p, distance, angle, MOTION_SLOT_ACTIVE);
                        }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;
            if (mui_spell1 <= diff)
            {
                DoCastVictim(15572);
                mui_spell1 = 10000 + urand(1200, 5100);
            }
            else
                mui_spell1 -= diff;

            if (mui_spell2 <= diff)
            {
                DoCastVictim(47168);
                mui_spell2 = 10000 + urand(2100, 5400);
            }
            else
                mui_spell2 -= diff;

            victim = me->GetVictim();
            if (victim && me->GetDistance2d(victim->GetPositionX(), victim->GetPositionY()) > 7)
                DoSpellAttackIfReady(6660);
            else
                DoMeleeAttackIfReady();
        }

    private :
        uint32 mui_spell1;
        uint32 mui_spell2;
        Unit *victim;
        uint32 group;
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_gilnean_militia_38221AI(pCreature);
    }
};

// 38415
class npc_lord_darius_crowley_38415 : public CreatureScript
{
public:
    npc_lord_darius_crowley_38415() : CreatureScript("npc_lord_darius_crowley_38415") { }

    struct npc_lord_darius_crowley_38415AI : public ScriptedAI
    {
        npc_lord_darius_crowley_38415AI(Creature* pCreature) : ScriptedAI(pCreature) { }

        void Reset()
        {
            float angle = 0;
            for (int i = 0; i < 30; i++)
            {
                if (Creature *summon = me->SummonCreature(38348, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_MANUAL_DESPAWN))
                {
                    summon->GetMotionMaster()->MovePoint(0, -1710.466f + cos(angle) * 15, 1407.905f + sin(angle) * 15, 21.75196f);
                    if (i >= 0 && i <= 3)
                        worgenGUID[i] = summon->GetGUID();
                }
                angle += M_PI / 15;
            }
            me->GetMotionMaster()->MovePoint(0, -1710.466f, 1407.905f, 21.75196f);
            aboGUID = 0;
            mui_jumpBoss = 3000;
            moveJumpBoss = true;
        }

        void SetGUID(uint64 guid, int32 type = 0)
        {
            aboGUID = guid;
        }

        void EnterCombat(Unit* pWho)
        {
            if (pWho->GetEntry() == 38348)
                me->CastSpell(pWho, 71921, true);
            Talk(0);
        }

        void UpdateAI(uint32 diff)
        {
            if (moveJumpBoss)
            {
                if (mui_jumpBoss <= diff)
                {
					if (Creature *abo = ObjectAccessor::GetCreature(*me, aboGUID))
                        for (int i = 0; i <= 3; i++)
							if (Creature *worgen = ObjectAccessor::GetCreature(*me, worgenGUID[i]))
                                worgen->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, i + 1, abo, false);
                    moveJumpBoss = false;
                    mui_jumpBoss = 3600000;
                }
                else mui_jumpBoss -= diff;
            }
            if (!UpdateVictim())
                return;
            DoMeleeAttackIfReady();
        }

    private:
        uint64 aboGUID;
        uint32 mui_jumpBoss;
        bool moveJumpBoss;
        uint64 worgenGUID[4];
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_lord_darius_crowley_38415AI(pCreature);
    }
};

// 38348
class npc_worgen_warrior_38348 : public CreatureScript
{
public:
    npc_worgen_warrior_38348() : CreatureScript("npc_worgen_warrior_38348") { }

    struct npc_worgen_warrior_38348AI : public ScriptedAI
    {
        npc_worgen_warrior_38348AI(Creature* pCreature) : ScriptedAI(pCreature) { }

        void EnterCombat(Unit* pWho)
        {
            if (pWho->GetEntry() == 38348)
            {
                me->CastSpell(pWho, 71921, true);
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_worgen_warrior_38348AI(pCreature);
    }
};

/*######
## npc_lady_sylvanas_gilneas
######*/
class npc_lady_sylvanas_gilneas : public CreatureScript
{
public:
    npc_lady_sylvanas_gilneas() : CreatureScript("npc_lady_sylvanas_gilneas") { }

    struct npc_lady_sylvanas_gilneasAI : public ScriptedAI
    {
        npc_lady_sylvanas_gilneasAI(Creature* pCreature) : ScriptedAI(pCreature) { }

        bool outEvent;
        uint32 phase;
        uint32 phaseTimer;

        Creature* liam;
        Creature* genn;

        void Reset()
        {
            outEvent = false;
            phase = 1;
            phaseTimer = 300;
        }

        void EnterCombat(Unit* pWho)
        {

        }

        void DoAction(int32 type)
        {
            if (type == ACTION_START_EVENT)
                outEvent = true;
        }

        void DamageTaken(Unit* who, uint32 &damage)
        {
            if (me->GetHealthPct() <= 20)
            {
                DoAction(ACTION_START_EVENT);
                damage = 0;
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (outEvent)
            {
                if (phaseTimer <= diff)
                {
                    switch (phase)
                    {
                    case 1:
                        phaseTimer = 1000;
                        DoCastAOE(72113);
                        break;
                    case 2:
                        me->SetReactState(REACT_PASSIVE);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        phaseTimer = 2000;
                        break;
                    case 3:
                        if (genn = me->FindNearestCreature(38470, 100))
                            genn->CastSpell(genn, 86141, true);
                        phaseTimer = 2000;
                        break;
                    case 4:
                        if (liam = me->SummonCreature(38474, -1634.634f, 1631.859f, 21.21159f, 4.694936f, TEMPSUMMON_TIMED_DESPAWN, 60000))
                        {
                            liam->AI()->Talk(0);
                            liam->SetReactState(REACT_PASSIVE);
                        }
                        phaseTimer = 2000;
                        break;
                    case 5:
                        liam->GetMotionMaster()->MovePoint(0, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                        liam->AI()->Talk(1);
                        phaseTimer = 6000;
                        break;
                    case 7:
                        liam->AI()->Talk(2);
                        me->CastSpell(liam, 72116, true);
                        me->Kill(liam);
                        genn->RemoveAurasDueToSpell(86141);
                        genn->RemoveAurasDueToSpell(72113);
                        genn->CastSpell(genn, 68442, true);
                        phaseTimer = 2000;
                        break;
                    case 8:
                        Talk(0);
                        phaseTimer = 2000;
                        break;
                    case 9:
                        Talk(1);
                        phaseTimer = 3500;
                        break;
                    case 10:
                        Talk(2);
                        phaseTimer = 2500;
                        break;
                    case 11:
                        me->GetMotionMaster()->MovePoint(0, -1634.634f, 1631.859f, 21.21159f);
                        phaseTimer = 2500;
                        break;
                    case 12:
                        genn->DespawnOrUnsummon();
                        std::list<Player *> players = me->FindNearestPlayers(100.0f, true);
                        for (std::list<Player *>::const_iterator it = players.begin(); it != players.end(); it++)
                            (*it)->KilledMonsterCredit(38854, 0);
                        outEvent = false;
                        me->DespawnOrUnsummon();
                        break;
                    }
                    phase++;
                } else phaseTimer -= diff;
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_lady_sylvanas_gilneasAI(pCreature);
    }
};

// 38611
class npc_lorna_crowley_38611 : public CreatureScript
{
public:
    npc_lorna_crowley_38611() : CreatureScript("npc_lorna_crowley_38611") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == 24902)
            CAST_AI(npc_lorna_crowley_38611::npc_lorna_crowley_38611AI, creature->AI())->StartEncounter(player);
        return true;
    }

    struct npc_lorna_crowley_38611AI : public ScriptedAI
    {
        npc_lorna_crowley_38611AI(Creature *c) : ScriptedAI(c) {}

        uint64 tobiasGUID;

        void Reset()
        {
            tobiasGUID = 0;
        }

        void StartEncounter(Player* pl)
        {
			if (Creature* tobias = ObjectAccessor::GetCreature(*me, tobiasGUID))
                pl->TeleportTo(MAP_GILNEAS, tobias->GetPositionX(), tobias->GetPositionY(), tobias->GetPositionZ(), 5.42f);
            else
            {
                if (Creature* newTobias = me->SummonCreature(NPC_TOBIAS_MISTMANTLE, -1662.616f, 1591.453f, 23.20896f, 3.740811f, TEMPSUMMON_DEAD_DESPAWN, 180000))
                {
                    newTobias->SetPhaseMask(pl->GetPhaseMask(), true);
                    tobiasGUID = newTobias->GetGUID();
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_lorna_crowley_38611AI(pCreature);
    }
};

// 38615
class npc_captured_riding_bat_38615 : public CreatureScript
{
public:
    npc_captured_riding_bat_38615() : CreatureScript("npc_captured_riding_bat_38615") { }

    struct npc_captured_riding_bat_38615AI : public npc_escortAI
    {
        npc_captured_riding_bat_38615AI(Creature* creature) : npc_escortAI(creature) {}

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset()
        {
            _checkQuest = 1000;
            _checkDespawn = 1000;
            isBoarded = false;
            isBoarded2 = false;
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            if (apply)
            {
                if (who->GetTypeId() == TYPEID_PLAYER)
                    if (who->ToPlayer()->GetQuestStatus(24920) != QUEST_STATUS_INCOMPLETE)
                    {
                        me->GetVehicleKit()->RemoveAllPassengers();
                        return ;
                    }
                me->SetCanFly(true);
                me->SetSpeed(MOVE_FLIGHT, 3.0f);
                isBoarded = true;
                Start(false, true, who->GetGUID(), NULL, false, true);
            }
        }

        void WaypointReached(uint32 i)
        {
            me->SetCanFly(true);
        }

        void JustDied(Unit* /*killer*/)
        {
        }

        void OnCharmed(bool /*apply*/)
        {
        }


        void UpdateAI(uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);
        }

        void UpdateEscortAI(const uint32 diff)
        {
            if (isBoarded)
            {
                if (isBoarded2)
                {
                    if (_checkDespawn <= diff)
                    {
                        me->GetVehicleKit()->RemoveAllPassengers();
                        me->DespawnOrUnsummon();
                        _checkDespawn = 1000;
                    }
                    else
                        _checkDespawn -= diff;
                }
                else
                {
                    if (_checkQuest <= diff)
                    {
                        if (me->GetVehicleKit())
                            if (Unit *u = me->GetVehicleKit()->GetPassenger(0))
                                if (u->GetTypeId() == TYPEID_PLAYER)
                                {
                                    Player *player = u->ToPlayer();
                                    if (player->GetQuestStatus(24920) == QUEST_STATUS_COMPLETE)
                                    {
                                        isBoarded2 = true;
                                        _checkDespawn = 70000;
                                        SetEscortPaused(true);
                                        me->GetMotionMaster()->MovePoint(4242, me->GetHomePosition());
                                    }
                                }
                        _checkQuest = 1000;
                    }
                    else
                        _checkQuest -= diff;
                }
            }
        }

    private :
        uint32 _checkQuest;
        uint32 _checkDespawn;
        bool isBoarded;
        bool isBoarded2;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_captured_riding_bat_38615AI(creature);
    }
};

// 36616
class npc_admiral_nightwind_36616 : public CreatureScript
{
public:
    npc_admiral_nightwind_36616() : CreatureScript("npc_admiral_nightwind_36616") { }

	bool OnQuestReward(Player* player, Creature* /*creature*/, Quest const* quest, uint32 /*opt*/)
	{		
        if (quest->GetQuestId() == 14434)
            {
				player->RemoveAllAuras();
				player->TeleportTo(1,8343,801,4,4);
			}
		return true;
	}
};

// 36290 ToDo: this we make better with phase_definition and conditions...
class npc_lord_godfrey_36290 : public CreatureScript
{
public:
    npc_lord_godfrey_36290() : CreatureScript("npc_lord_godfrey_36290") { }

	bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
	{		
        if (quest->GetQuestId() == 14396)
            {
              std::set<uint32> terrainswap;
              std::set<uint32> phaseId;
              std::set<uint32> worldMapSwap;
              terrainswap.insert(655);
              phaseId.insert(1);
              player->GetSession()->SendSetPhaseShift(phaseId, terrainswap, worldMapSwap);
			}

		return true;
	}
};

void AddSC_zone_gilneas_city2()
{
    new spell_attack_lurker_67805();
    new npc_admiral_nightwind_36616();
    new npc_gilneas_city_guard_34916();
    new npc_prince_liam_greymane_34913();
    new go_merchant_square_door_195327();
    new npc_rampaging_worgen_34884();
    new npc_rampaging_worgen_35660();
    new npc_sergeant_cleese_35839();
    new npc_king_genn_greymane_35112();
    new npc_bloodfang_worgen_35118();
    new npc_josiah_avery_35369();
    new npc_josiah_avery_trigger_50415();
    new npc_lorna_crowley_35378();
    new npc_bloodfang_lurker_35463();
    new npc_gilnean_mastiff_35631();
    //new npc_bloodfang_worgen();
    new npc_frightened_citizen_34981();
    new npc_gilnean_royal_guard_35232();
    new npc_mariam_spellwalker_35872();
    new npc_lord_godfrey_35906();
    new npc_tobias_mistmantle_35124();
    new npc_lord_darius_crowley_35566();
    new npc_king_genn_greymane_35550();
    new npc_crowley_horse_35231();
    new npc_horrid_abomination_36231();
    new spell_keg_placed_68555();
    new npc_king_greymanes_horse_35905();
    new npc_krennan_aranas_35907();
    new npc_bloodfang_stalker_35229();
    new npc_lord_darius_crowley_35552();
    new npc_trigger_event_c3();
    new npc_trigger_event_first_c3();
    new npc_king_genn_greymane();
	
    new npc_lord_darius_crowley_35077();
    new npc_worgen_runt_35188();
    new npc_worgen_alpha_35170();
    new npc_worgen_runt_35456();
    new npc_worgen_alpha_35167();

    new go_mandragore_196394();
    new spell_rescue_drowning_watchman_68735();
    new spell_round_up_horse_68903();
    new npc_trigger_quest_24616();

    new npc_krennan_aranas_38553();
    new npc_prince_liam_greymane_gilneas();
    new npc_gilnean_militia_38221();
    new npc_lord_darius_crowley_38415();
    new npc_worgen_warrior_38348();
    new npc_lady_sylvanas_gilneas();
    new npc_lorna_crowley_38611();

    new npc_lorna_crowley_event();
    new npc_captured_riding_bat_38615();
    new npc_commandeered_cannon_35914();
    new npc_lord_godfrey_35115();
    new npc_lord_godfrey_36290();
}

