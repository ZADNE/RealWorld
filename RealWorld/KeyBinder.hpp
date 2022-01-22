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
		RE::Key::E,	\
		RE::Key::LMB,	\
		RE::Key::RMB,	\
		RE::Key::Q,	\
		RE::Key::DMW,	\
		RE::Key::UMW,	\
		RE::Key::K1,	\
		RE::Key::K2,	\
		RE::Key::K3,	\
		RE::Key::K4,	\
		RE::Key::K5,	\
		RE::Key::K6,	\
		RE::Key::K7,	\
		RE::Key::K8,	\
		RE::Key::K9,	\
		RE::Key::K0,	\
		RE::Key::LMB,	\
		RE::Key::RMB,	\
		RE::Key::LMB,	\
		RE::Key::RMB,	\
		RE::Key::DMW,	\
		RE::Key::UMW,	\
		RE::Key::RMB,	\
		RE::Key::Space,	\
		RE::Key::A,	\
		RE::Key::D,	\
		RE::Key::Escape,	\
		RE::Key::P,	\
		RE::Key::O, }	\

#endif // !KB_BINDING_LIST
#include <RealEngine/user_input/KeyBinder.hpp>