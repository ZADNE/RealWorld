#pragma once
#include <algorithm>

class Health{
public:
	Health(float hp, float maxRelOveheal = 1.0f) : m_hp(hp), m_hpMax(hp), m_maxRelOverheal(maxRelOveheal) {

	}

	bool damage(float damage) {//Returns whether the object died
		m_hp -= damage;
		m_hp = (std::min)(m_hp, m_hpMax * m_maxRelOverheal);//Avoiding overheal
		if (m_hp <= 0.0f) {
			return true;
		}
		return false;
	};
private:
	float m_hp;
	float m_hpMax;
	float m_maxRelOverheal;
};