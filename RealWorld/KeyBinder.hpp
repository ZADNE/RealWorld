#pragma once
#ifndef KB_BINDING_LIST

#define KB_BINDING_LIST(m)   \
    m(INV, SWITCHSTATE)  \
    m(INV, SWAPCURSOR)  \
    m(INV, MOVEPORTION)  \
    m(INV, PREVSLOT)  \
    m(INV, RIGHTSLOT)  \
    m(INV, LEFTSLOT)  \
    m(INV, SLOT0)  \
    m(INV, SLOT1)  \
    m(INV, SLOT2)  \
	m(INV, SLOT3)  \
    m(INV, SLOT4)  \
    m(INV, SLOT5)  \
    m(INV, SLOT6)  \
    m(INV, SLOT7)  \
    m(INV, SLOT8)  \
    m(INV, SLOT9)  \
    m(INV, USEPRIMARY)  \
    m(INV, USESECONDARY)  \
    m(CRAFT, ONE)  \
    m(CRAFT, SOME)  \
    m(CRAFT, ROLL_RIGHT)  \
    m(CRAFT, ROLL_LEFT)  \
    m(CRAFT, CANCEL)  \
    m(PLAYER, JUMP)  \
    m(PLAYER, LEFT)  \
    m(PLAYER, RIGHT)  \
    m(DEBUG, ENDGAME)  \
    m(DEBUG, WORLDDRAW)  \
    m(DEBUG, WORLDDARKNESS)  \

#define KB_DEFAULT_LIST {	\
		RE::RKey::E,	\
		RE::RKey::LMB,	\
		RE::RKey::RMB,	\
		RE::RKey::Q,	\
		RE::RKey::DMW,	\
		RE::RKey::UMW,	\
		RE::RKey::K1,	\
		RE::RKey::K2,	\
		RE::RKey::K3,	\
		RE::RKey::K4,	\
		RE::RKey::K5,	\
		RE::RKey::K6,	\
		RE::RKey::K7,	\
		RE::RKey::K8,	\
		RE::RKey::K9,	\
		RE::RKey::K0,	\
		RE::RKey::LMB,	\
		RE::RKey::RMB,	\
		RE::RKey::LMB,	\
		RE::RKey::RMB,	\
		RE::RKey::DMW,	\
		RE::RKey::UMW,	\
		RE::RKey::RMB,	\
		RE::RKey::Space,	\
		RE::RKey::A,	\
		RE::RKey::D,	\
		RE::RKey::Escape,	\
		RE::RKey::P,	\
		RE::RKey::O, }	\

#endif // !KB_BINDING_LIST
#include <RealEngine/KeyBinder.hpp>