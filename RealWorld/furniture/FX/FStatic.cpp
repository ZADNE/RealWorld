#include <RealWorld/furniture/FX/FStatic.hpp>

#include <RealEngine/graphics/SpriteBatch.hpp>
#include <RealEngine/resources/ResourceManager.hpp>

RE::SpriteBatch* FStatic::p_spriteBatch = nullptr;
LightManipulator FStatic::p_lightManipulator = {};

FStatic::FStatic(const glm::ivec2& botLeftBc, size_t totalIndex):
	p_botLeftBc(botLeftBc),
	p_totalIndex(totalIndex), 
	p_sprite(FDB::getTexture(p_totalIndex), FDB::getSprite(p_totalIndex), 0.0f, FDB::getImageSpeed(p_totalIndex)) {

}

FStatic::FStatic(const glm::ivec2& botLeftBc, F_TYPE type, size_t specificIndex):
	p_botLeftBc(botLeftBc),
	p_totalIndex(FDB::getTotalIndex(type, specificIndex)),
	p_sprite(FDB::getTexture(p_totalIndex), FDB::getSprite(p_totalIndex), 0.0f, FDB::getImageSpeed(p_totalIndex)) {

}

FStatic::~FStatic(){

}

void FStatic::build(){

}

void FStatic::step() {
	p_sprite.step();
}

void FStatic::draw() {
	p_spriteBatch->addSprite(p_sprite, glm::vec2(p_botLeftBc * ivec2_BLOCK_SIZE), 5);
}

void FStatic::destroy(){

}

void FStatic::initStatics(RE::SpriteBatch* spriteBatch, LightManipulator lightManipulator) {
	p_spriteBatch = spriteBatch;
	p_lightManipulator = lightManipulator;
}
