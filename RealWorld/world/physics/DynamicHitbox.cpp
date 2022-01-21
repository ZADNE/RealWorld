#include <RealWorld/world/physics/DynamicHitbox.hpp>

#include <algorithm>

#include <RealEngine/utility/utility.hpp>

#include <RealWorld/world/World.hpp>
#include <RealWorld/world/TDB.hpp>
#include <RealWorld/world/physics/position_conversions.hpp>


DynamicHitbox::DynamicHitbox(const glm::ivec2& positionPx, const glm::ivec2& dimensionPx, const glm::ivec2& offsetPx/* = glm::ivec2(0, 0)*/) :
	Hitbox(positionPx, dimensionPx - glm::ivec2(1, 1), offsetPx),
	m_velocityPx(0){

}

DynamicHitbox::DynamicHitbox(const glm::ivec2& positionPx, const glm::ivec2& dimensionPx, World* world, const glm::ivec2& offsetPx/* = glm::ivec2(0, 0)*/) :
	Hitbox(positionPx, dimensionPx - glm::ivec2(1, 1), world, offsetPx),
	m_velocityPx(0) {

}

DynamicHitbox::~DynamicHitbox(){

}

void DynamicHitbox::setPosition(const glm::ivec2& positionPx){
	p_botLeftPx = positionPx - p_offsetPx;
	m_justChanged = true;
}

void DynamicHitbox::setPositionX(int positionPx){
	p_botLeftPx.x = positionPx - p_offsetPx.x;
	m_justChanged = true;
}

void DynamicHitbox::setPositionY(int positionPx){
	p_botLeftPx.y = positionPx - p_offsetPx.y;
	m_justChanged = true;
}

void DynamicHitbox::setVelocity(const glm::vec2& velocityPx) {
	m_velocityPx = velocityPx;
	m_justChanged = true;
}

void DynamicHitbox::setVelocityX(float velocityPx){
	m_velocityPx.x = velocityPx;
	m_justChanged = true;
}

void DynamicHitbox::setVelocityY(float velocityPx){
	m_velocityPx.y = velocityPx;
	m_justChanged = true;
}

void DynamicHitbox::setFriction(const glm::vec2& frictionPx){
	m_frictionPx = frictionPx;
	m_justChanged = true;
}

void DynamicHitbox::addVelocity(const glm::vec2& velocityPx){
	m_velocityPx += velocityPx;
	m_justChanged = true;
}

void DynamicHitbox::addVelocityX(float velocityPx){
	m_velocityPx.x += velocityPx;
	m_justChanged = true;
}

void DynamicHitbox::addVelocityY(float velocityPx){
	m_velocityPx.y += velocityPx;
	m_justChanged = true;
}

void DynamicHitbox::limitVelocity(const glm::vec2& velocityPx){
	m_velocityPx = glm::clamp(m_velocityPx, -velocityPx, velocityPx);
}

void DynamicHitbox::limitVelocityX(float velocityPx){
	m_velocityPx.x = rmath::clamp(m_velocityPx.x, -velocityPx, velocityPx);
}

void DynamicHitbox::limitVelocityY(float velocityPx){
	m_velocityPx.y = rmath::clamp(m_velocityPx.y, -velocityPx, velocityPx);
}

glm::vec2 DynamicHitbox::getVelocity() const{
	return m_velocityPx;
}

glm::vec2 DynamicHitbox::getFriction() const{
	return m_frictionPx;
}

void DynamicHitbox::step(){
	//Gravity
	m_velocityPx += p_world->getGravity();
	//Friction
	if (!m_justChanged) {
		m_velocityPx -= m_frictionPx * glm::sign(m_velocityPx);
	}

	glm::ivec2 vel = glm::ivec2(glm::ceil(m_velocityPx));

	//Collision checking X
	if (p_world->getMax(chunk::BLOCK_VALUES::BLOCK,
		pxToBc(p_botLeftPx + glm::ivec2(vel.x, 0)), pxToBc(p_botLeftPx + glm::ivec2(vel.x, 0) + p_dimensionPx)) > LAST_NONSOLIDBLOCK) {
		while (p_world->getMax(chunk::BLOCK_VALUES::BLOCK, pxToBc(p_botLeftPx + glm::ivec2(rmath::sign(vel.x), 0)), pxToBc(p_botLeftPx + glm::ivec2(rmath::sign(vel.x), 0) + p_dimensionPx)) <= LAST_NONSOLIDBLOCK) {
			p_botLeftPx.x += rmath::sign(vel.x);
		}
		m_velocityPx.x = 0.0f;
	}
	p_botLeftPx.x += (int)m_velocityPx.x;

	//Collision checking Y
	if (p_world->getMax(chunk::BLOCK_VALUES::BLOCK, pxToBc(p_botLeftPx + glm::ivec2(0, vel.y)), pxToBc(p_botLeftPx + glm::ivec2(0, vel.y) + p_dimensionPx)) > LAST_NONSOLIDBLOCK) {
		while (p_world->getMax(chunk::BLOCK_VALUES::BLOCK, pxToBc(p_botLeftPx + glm::ivec2(0, rmath::sign(vel.y))), pxToBc(p_botLeftPx + glm::ivec2(0, rmath::sign(vel.y)) + p_dimensionPx)) <= LAST_NONSOLIDBLOCK) {
			p_botLeftPx.y += rmath::sign(vel.y);
		}
		m_velocityPx.y = 0.0f;
	}
	p_botLeftPx.y += (int)m_velocityPx.y;

	m_justChanged = false;
}