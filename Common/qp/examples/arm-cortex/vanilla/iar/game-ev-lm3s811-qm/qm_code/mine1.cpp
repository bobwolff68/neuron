//////////////////////////////////////////////////////////////////////////////
// Model: game.qm
// File:  ./qm_code/mine1.cpp
//
// This file has been generated automatically by QP Modeler (QM).
// DO NOT EDIT THIS FILE MANUALLY.
//
// Please visit www.state-machine.com/qm for more information.
//////////////////////////////////////////////////////////////////////////////
#include "qp_port.h"
#include "bsp.h"
#include "game.h"

Q_DEFINE_THIS_FILE

// encapsulated delcaration of the Mine1 HSM ---------------------------------
// $(AOs::Mine1) .............................................................
class Mine1 : public QHsm {
private:
    uint8_t m_x;
    uint8_t m_y;
    uint8_t m_exp_ctr;

public:
    Mine1() : QHsm((QStateHandler)&Mine1::initial) {
    }

protected:
    static QState initial(Mine1 *me, QEvent const *e);
    static QState unused(Mine1 *me, QEvent const *e);
    static QState used(Mine1 *me, QEvent const *e);
    static QState planted(Mine1 *me, QEvent const *e);
    static QState exploding(Mine1 *me, QEvent const *e);
};

// local objects -------------------------------------------------------------
static Mine1 l_mine1[GAME_MINES_MAX];                // a pool of type-1 mines

                                // helper macro to provide the ID of this mine
#define MINE_ID(me_)    ((me_) - l_mine1)

//............................................................................
QHsm *Mine1_getInst(uint8_t id) {
    Q_REQUIRE(id < GAME_MINES_MAX);
    return &l_mine1[id];
}
// Mine1 class definition ----------------------------------------------------
// $(AOs::Mine1) .............................................................

// $(AOs::Mine1::Statechart) .................................................
// @(/2/3/4/0)
QState Mine1::initial(Mine1 *me, QEvent const *e) {
    static uint8_t dict_sent;
    if (!dict_sent) {
        // object dictionaries for Mine1 pool...
        QS_OBJ_DICTIONARY(&l_mine1[0]);
        QS_OBJ_DICTIONARY(&l_mine1[1]);
        QS_OBJ_DICTIONARY(&l_mine1[2]);
        QS_OBJ_DICTIONARY(&l_mine1[3]);
        QS_OBJ_DICTIONARY(&l_mine1[4]);

        // function dictionaries for Mine1 HSM...
        QS_FUN_DICTIONARY(&Mine1::initial);
        QS_FUN_DICTIONARY(&Mine1::unused);
        QS_FUN_DICTIONARY(&Mine1::used);
        QS_FUN_DICTIONARY(&Mine1::planted);
        QS_FUN_DICTIONARY(&Mine1::exploding);

        dict_sent = 1;
    }

    // local signals...
    QS_SIG_DICTIONARY(MINE_PLANT_SIG,    me);
    QS_SIG_DICTIONARY(MINE_DISABLED_SIG, me);
    QS_SIG_DICTIONARY(MINE_RECYCLE_SIG,  me);
    QS_SIG_DICTIONARY(SHIP_IMG_SIG,      me);
    QS_SIG_DICTIONARY(MISSILE_IMG_SIG,   me);
    return Q_TRAN(&Mine1::unused);
}
// $(AOs::Mine1::Statechart::unused) .........................................
QState Mine1::unused(Mine1 *me, QEvent const *e) {
    switch (e->sig) {
        // @(/2/3/4/1/0)
        case MINE_PLANT_SIG: {
            me->m_x = ((ObjectPosEvt const *)e)->x;
            me->m_y = ((ObjectPosEvt const *)e)->y;
            return Q_TRAN(&Mine1::planted);
        }
    }
    return Q_SUPER(&QHsm::top);
}
// $(AOs::Mine1::Statechart::used) ...........................................
QState Mine1::used(Mine1 *me, QEvent const *e) {
    switch (e->sig) {
        // @(/2/3/4/2)
        case Q_EXIT_SIG: {
            // tell the Tunnel that this mine is becoming disabled
            MineEvt *mev = Q_NEW(MineEvt, MINE_DISABLED_SIG);
            mev->id = MINE_ID(me);
            AO_Tunnel->postFIFO(mev);
            return Q_HANDLED();
        }
        // @(/2/3/4/2/0)
        case MINE_RECYCLE_SIG: {
            return Q_TRAN(&Mine1::unused);
        }
    }
    return Q_SUPER(&QHsm::top);
}
// $(AOs::Mine1::Statechart::used::planted) ..................................
QState Mine1::planted(Mine1 *me, QEvent const *e) {
    switch (e->sig) {
        // @(/2/3/4/2/1/0)
        case TIME_TICK_SIG: {
            // @(/2/3/4/2/1/0/0)
            if (me->m_x >= GAME_SPEED_X) {
                ObjectImageEvt *oie;

                me->m_x -= GAME_SPEED_X; // move the mine 1 step

                // tell the Tunnel to draw the Mine
                oie = Q_NEW(ObjectImageEvt, MINE_IMG_SIG);
                oie->x   = me->m_x;
                oie->y   = me->m_y;
                oie->bmp = MINE1_BMP;
                AO_Tunnel->postFIFO(oie);
                return Q_HANDLED();
            }
            // @(/2/3/4/2/1/0/1)
            else {
                return Q_TRAN(&Mine1::unused);
            }
        }
        // @(/2/3/4/2/1/1)
        case SHIP_IMG_SIG: {
            uint8_t x = (uint8_t)((ObjectImageEvt const *)e)->x;
            uint8_t y = (uint8_t)((ObjectImageEvt const *)e)->y;
            uint8_t bmp = (uint8_t)((ObjectImageEvt const *)e)->bmp;
            // @(/2/3/4/2/1/1/0)
            if (do_bitmaps_overlap(MINE1_BMP, me->m_x, me->m_y, bmp, x, y)) {
                // Hit event with the type of the Mine1
                static MineEvt const mine1_hit(HIT_MINE_SIG, 1);
                AO_Ship->postFIFO(&mine1_hit);

                // go straight to 'disabled' and let the Ship do the exploding
                return Q_TRAN(&Mine1::unused);
            }
            break;
        }
        // @(/2/3/4/2/1/2)
        case MISSILE_IMG_SIG: {
            uint8_t x = (uint8_t)((ObjectImageEvt const *)e)->x;
            uint8_t y = (uint8_t)((ObjectImageEvt const *)e)->y;
            uint8_t bmp = (uint8_t)((ObjectImageEvt const *)e)->bmp;
            // @(/2/3/4/2/1/2/0)
            if (do_bitmaps_overlap(MINE1_BMP, me->m_x, me->m_y, bmp, x, y)) {
                // Score event with the score for destroying Mine1
                static ScoreEvt const mine1_destroyed(DESTROYED_MINE_SIG, 25);
                AO_Missile->postFIFO(&mine1_destroyed);
                return Q_TRAN(&Mine1::exploding);
            }
            break;
        }
    }
    return Q_SUPER(&Mine1::used);
}
// $(AOs::Mine1::Statechart::used::exploding) ................................
QState Mine1::exploding(Mine1 *me, QEvent const *e) {
    switch (e->sig) {
        // @(/2/3/4/2/2)
        case Q_ENTRY_SIG: {
            me->m_exp_ctr = 0;
            return Q_HANDLED();
        }
        // @(/2/3/4/2/2/0)
        case TIME_TICK_SIG: {
            // @(/2/3/4/2/2/0/0)
            if ((me->m_x >= GAME_SPEED_X) && (me->m_exp_ctr < 15)) {
                ObjectImageEvt *oie;

                ++me->m_exp_ctr; // advance the explosion counter
                me->m_x -= GAME_SPEED_X; // move explosion by 1 step

                // tell the Game to render the current stage of Explosion
                oie = Q_NEW(ObjectImageEvt, EXPLOSION_SIG);
                oie->x   = me->m_x + 1; // x of explosion
                oie->y   = (int8_t)((int)me->m_y - 4 + 2); // y of explosion
                oie->bmp = EXPLOSION0_BMP + (me->m_exp_ctr >> 2);
                AO_Tunnel->postFIFO(oie);
                return Q_HANDLED();
            }
            // @(/2/3/4/2/2/0/1)
            else {
                return Q_TRAN(&Mine1::unused);
            }
        }
    }
    return Q_SUPER(&Mine1::used);
}

