#include <RealWorld/world/WorldDrawer.hpp>

#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>

#include <RealEngine/graphics/Vertex.hpp>
#include <RealEngine/graphics/Surface.hpp>
#include <RealEngine/graphics/UniformManager.hpp>
#include <RealEngine/utility/utility.hpp>

#include <RealWorld/world/ChunkManager.hpp>
#include <RealWorld/metadata.hpp>
#include <RealWorld/shaders/shaders.hpp>
#include <RealWorld/div.hpp>
#include <RealWorld/rendering/Vertex.hpp>

struct WorldDrawUniforms {
	glm::mat4 viewMat;
	glm::mat4 viewsizePxMat;
	glm::mat4 viewsizeLightingBcMat;
};


WorldDrawer::WorldDrawer() {

}

WorldDrawer::~WorldDrawer() {
	//Tile drawing
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
	//Lighting global
	glDeleteVertexArrays(1, &m_VAOLighting);
	glDeleteBuffers(1, &m_VBOLighting);
	//Static lights
	glDeleteVertexArrays(1, &m_VAO_SLs);
	glDeleteBuffers(1, &m_VBO_SLs);
	//Dynamic lights
	glDeleteVertexArrays(1, &m_VAO_DLs);
	glDeleteBuffers(1, &m_VBO_DLs);
}

void WorldDrawer::init(const glm::uvec2& viewSizePx, const glm::mat4& viewMatrix, ChunkManager* chunkHandler) {
	m_chunkHandler = chunkHandler;
	reloadViewSize(viewSizePx);
	initShaders(viewMatrix);
	createGLObjects();
	generateMesh();

	//Lighting
	createGLObjectsLighting();
	generateMeshLighting();
	m_SurLighting.resize({m_viewsizeLightingBc}, 3);
	createGLObjectsStaticLights();
	createGLObjectsDynamicLights();

	TEX_UNIT_DIAPHRAGM.setActive();
	m_SurLighting.bindTexture(0);

	TEX_UNIT_LIGHTING.setActive();
	m_SurLighting.bindTexture(1);

	TEX_UNIT_LIGHT_FINISHED.setActive();
	m_SurLighting.bindTexture(2);

	TEX_UNIT_VOLATILE.setActive();

	//For dynamic lights
	glEnable(GL_POINT_SPRITE);/*TODO*/
	glEnable(GL_PROGRAM_POINT_SIZE);
	glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
	glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, 2.0f);


	TEX_UNIT_BLOCK_ATLAS.setActive();
	m_blockAtlasTex = RE::RM::getTexture("blockAtlas");
	m_blockAtlasTex->bind();

	TEX_UNIT_WALL_ATLAS.setActive();
	m_wallAtlasTex = RE::RM::getTexture("wallAtlas");
	m_wallAtlasTex->bind();

	TEX_UNIT_VOLATILE.setActive();
}

void WorldDrawer::setTarget(const glm::ivec2& worldDimBc, RE::TextureProxy worldTexture) {
	m_worldDimBc = worldDimBc;

	TEX_UNIT_WORLD_TEXTURE.setActive();
	worldTexture.bind();
	TEX_UNIT_VOLATILE.setActive();
}

void WorldDrawer::resizeView(const glm::uvec2& newViewSizePx, const glm::mat4& newViewMatrix) {
	reloadViewSize(newViewSizePx);
	updateUniformsAfterResize(newViewMatrix);
	generateMesh();
	generateMeshLighting();

	m_SurLighting.resize({m_viewsizeLightingBc}, 3);

	TEX_UNIT_DIAPHRAGM.setActive();
	m_SurLighting.bindTexture(0);

	TEX_UNIT_LIGHTING.setActive();
	m_SurLighting.bindTexture(1);

	TEX_UNIT_LIGHT_FINISHED.setActive();
	m_SurLighting.bindTexture(2);

	TEX_UNIT_VOLATILE.setActive();
}

LightManipulator WorldDrawer::getLightManipulator() {
	return LightManipulator(this);
}

DestroyTicket<StaticLight> WorldDrawer::staticLight_add(const StaticLight& staticLight) {
	staticLight_reserve(1u);
	m_staticLights.push_back(staticLight);
	m_staticLights.back().posBc.y = m_worldDimBc.y - staticLight.posBc.y;
	//Updating the VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_SLs);
	glBufferSubData(GL_ARRAY_BUFFER, (m_staticLights.size() - 1u) * sizeof(StaticLight), sizeof(StaticLight), &m_staticLights.back());
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	DestroyTicket<StaticLight> newTicket = DestroyTicket<StaticLight>{m_nextTicket};
	++m_nextTicket;//Increasing the counter so that next ticket has higher ID
	m_destroyIndexes.emplace(newTicket, m_staticLights.size() - 1u);

	return newTicket;
}

void WorldDrawer::staticLight_remove(DestroyTicket<StaticLight> ticket) {
	auto iterator = m_destroyIndexes.find(ticket);
	if (iterator == m_destroyIndexes.end()) {
		return;//Ticket is invalid (the light was already destroyed)
	}

	m_staticLights[m_destroyIndexes[ticket]] = m_staticLights.back();//Moving last light to the place of removed light

	auto lastIt = m_destroyIndexes.rbegin();

	if (lastIt->second != iterator->second) {//If not removing last element
		lastIt->second = iterator->second;//Updating destroy index so that it points to the changed position of the light
	}

	//Updating the VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_SLs);
	glBufferSubData(GL_ARRAY_BUFFER, iterator->second * sizeof(StaticLight), sizeof(StaticLight), &m_staticLights[iterator->second]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	m_staticLights.pop_back();//Popping last light (it was moved to the ticket's index position)
	m_destroyIndexes.erase(iterator);//Removing the ticket from map
}

void WorldDrawer::staticLight_reserve(size_t add) {
	staticLight_reserveInternal(m_staticLights.size() + add);
}

void WorldDrawer::dynamicLight_add(const DynamicLight& dynamicLight) {
	m_dynamicLights.push_back(dynamicLight);
	m_dynamicLights.back().posPx.y = m_worldDimBc.y * vec2_BLOCK_SIZE.y - dynamicLight.posPx.y;
}

void WorldDrawer::dynamicLight_add(const glm::vec2& posPx, RE::Colour col, GLfloat dir, GLfloat cone) {
	m_dynamicLights.emplace_back(posPx, col, dir, cone);
	m_dynamicLights.back().posPx.y = m_worldDimBc.y * vec2_BLOCK_SIZE.y - posPx.y;
}

void WorldDrawer::beginStep(const glm::vec2& botLeftPx) {
	m_currentTime++;
	m_botLeftPx = botLeftPx;
	m_botLeftBc = floor_div((glm::ivec2)m_botLeftPx, ivec2_BLOCK_SIZE).quot;
	//Force activation of chunks
	glm::ivec2 chunkHandlerBotleft = m_botLeftBc - glm::ivec2((int)light::MAX_RANGE, (int)light::MAX_RANGE);
	m_chunkHandler->forceActivationOfChunks(chunkHandlerBotleft, chunkHandlerBotleft + static_cast<glm::ivec2>(m_viewsizeLightingBc));

	m_botLeftBc -= glm::ivec2((int)light::MAX_RANGE, -(int)light::MAX_RANGE);
	m_botLeftBc.y = m_worldDimBc.y - m_botLeftBc.y - m_viewsizeBc.y;


	//Process world texture to diaphragm and light
	m_SurLighting.setTarget();
	glBlendFunc(GL_ONE, GL_ZERO);
	glBindVertexArray(m_VAOLighting);
	m_worldToLight->use();
	m_worldToLight->setUniform(WDS::LOC_POSITION, static_cast<glm::vec2>(m_botLeftBc));
	float f = /*rmath::clamp(sin(m_currentTime / m_dayLength), (10.0f/256.0f), 1.0f)*/1.0f;
	auto background = m_backgroundColour;
	glClearColor(XYZW(background));
	m_worldToLight->setUniform(WDS::LOC_WORLD_TO_LIGHT_DAYLIGHT, glm::vec4(0.0f, 0.0f, 0.0f, f * 0.1f));
	m_worldToLight->setUniform(WDS::LOC_WORLD_TO_LIGHT_DIAPHRAGMS, glm::vec4(0.8f, 0.7f, 0.4f, 0.0f));
	glDrawArrays(GL_TRIANGLES, 0, 6);
	auto fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	m_worldToLight->unuse();
	glBindVertexArray(0);

	//Sum static lights
	glBindVertexArray(m_VAO_SLs);
	m_sumSL->use();
	glBlendFunci(0, GL_ONE, GL_ONE);
	glColorMaski(0, GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
	m_sumSL->setUniform(WDS::LOC_POSITION, m_botLeftBc);
	glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 100000000);
	glDrawArrays(GL_POINTS, 0, (GLsizei)m_staticLights.size());
	fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	m_sumSL->unuse();


	//Add static lights
	m_addSL->use();
	glColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
	glBlendFuncSeparatei(0, GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
	glBlendFunci(1, GL_ONE, GL_ONE);
	m_addSL->setUniform(WDS::LOC_POSITION, m_botLeftBc);
	glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 100000000);
	glDrawArrays(GL_POINTS, 0, (GLsizei)m_staticLights.size());
	m_addSL->unuse();
	glBindVertexArray(0);
	m_SurLighting.resetTarget();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	//Clear dynamic lights
	m_VBO_DLs_size = m_dynamicLights.size();
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_DLs);
	glBufferData(GL_ARRAY_BUFFER, m_VBO_DLs_size * sizeof(DynamicLight), NULL, GL_STREAM_DRAW);//Orphaning the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	m_dynamicLights.clear();
}

void WorldDrawer::endStep() {
	//Dynamic lights
	m_SurLighting.setTarget();
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_DLs);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_VBO_DLs_size * sizeof(DynamicLight), m_dynamicLights.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(m_VAO_DLs);

	m_addDL->use();
	glBlendFunc(GL_ONE, GL_ONE);
	m_addDL->setUniform(WDS::LOC_POSITION, static_cast<glm::vec2>(m_botLeftBc));
	glDrawArrays(GL_POINTS, 0, (GLsizei)m_VBO_DLs_size);
	auto fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	m_addDL->unuse();
	glBindVertexArray(0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Combine diaphragm and lighting texture to the finished texture
	glDisable(GL_BLEND);
	glBindVertexArray(m_VAOLighting);
	m_combineLighting->use();
	glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 100000000);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	m_combineLighting->unuse();
	glBindVertexArray(0);
	m_SurLighting.resetTarget();
	glEnable(GL_BLEND);
}

void WorldDrawer::drawWorld() {
	//Tiles
	glBindVertexArray(m_VAO);
	m_standardDraw->use();
	m_standardDraw->setUniform(WDS::LOC_POSITION, m_botLeftPx);
	glDrawArrays(GL_TRIANGLES, 0, m_viewsizeBc.x * m_viewsizeBc.y * 6);
	m_standardDraw->unuse();
	glBindVertexArray(0);
}

void WorldDrawer::coverWithDarkness() {
	//Draw the finished lighting
	glBindVertexArray(m_VAOLighting);
	m_finalLighting->use();
	m_finalLighting->setUniform(WDS::LOC_POSITION, glm::mod(m_botLeftPx, vec2_BLOCK_SIZE));
	glDrawArrays(GL_TRIANGLES, 6, 6);
	m_finalLighting->unuse();
	glBindVertexArray(0);
}

void WorldDrawer::reloadViewSize(const glm::uvec2& viewSizePx) {
	m_viewsizePx = (glm::vec2)viewSizePx;
	glm::uvec2 a = glm::uvec2(1u, 1u) + glm::uvec2(ceil(m_viewsizePx / vec2_BLOCK_SIZE));
	m_viewsizeBc = a;
	m_viewsizeLightingBc = a + glm::uvec2(light::MAX_RANGE * 2, light::MAX_RANGE * 2);
	m_viewsizePxLarger = (glm::vec2)m_viewsizeBc * vec2_BLOCK_SIZE;
}

void WorldDrawer::generateMesh() {
	std::vector<VertexPOUV> vertexData;
	vertexData.reserve((size_t)m_viewsizeBc.x * m_viewsizeBc.y * 6u);

	float xx, yy;
	//Creating mesh
	for (ulong x = 0; x < m_viewsizeBc.x; x++) {
		for (ulong y = 0; y < m_viewsizeBc.y; y++) {
			xx = (float)x * vec2_BLOCK_SIZE.x;
			yy = (float)y * vec2_BLOCK_SIZE.y;
			//BOT-LEFT triangle
			vertexData.emplace_back(xx, yy, 0.0f, 1.0f / float_NUMBER_OF_BLOCK_VARIATIONS);
			vertexData.emplace_back(xx + vec2_BLOCK_SIZE.x, yy, 1.0f / float_NUMBER_OF_BLOCKS, 1.0f / float_NUMBER_OF_BLOCK_VARIATIONS);
			vertexData.emplace_back(xx, yy + vec2_BLOCK_SIZE.y, 0.0f, 0.0f);
			//TOP-RIGHT triangle
			vertexData.emplace_back(xx + vec2_BLOCK_SIZE.x, yy + vec2_BLOCK_SIZE.y, 1.0f / float_NUMBER_OF_BLOCKS, 0.0f);
			vertexData.emplace_back(xx + vec2_BLOCK_SIZE.x, yy, 1.0f / float_NUMBER_OF_BLOCKS, 1.0f / float_NUMBER_OF_BLOCK_VARIATIONS);
			vertexData.emplace_back(xx, yy + vec2_BLOCK_SIZE.y, 0.0f, 0.0f);
		}
	}

	//Uploading mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPOUV) * vertexData.size(), &vertexData[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void WorldDrawer::createGLObjects() {
	if (m_VAO == 0) {//If doesn't exist already
		glGenVertexArrays(1, &m_VAO);
	}
	if (m_VBO == 0) {//If doesn't exist already
		glGenBuffers(1, &m_VBO);
	}
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

	glEnableVertexAttribArray(RE::ATTR_POSITION);//Position
	glVertexAttribPointer(RE::ATTR_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(VertexPOUV), (void*)offsetof(VertexPOUV, position));
	glEnableVertexAttribArray(RE::ATTR_UV);//UV
	glVertexAttribPointer(RE::ATTR_UV, 2, GL_FLOAT, GL_FALSE, sizeof(VertexPOUV), (void*)offsetof(VertexPOUV, uv));

	glBindVertexArray(0);
}

void WorldDrawer::generateMeshLighting() {
	VertexPOUV vertexData[12];
	//Block
	vertexData[0].setPosition(0.0f, 0.0f);
	vertexData[1].setPosition((float)m_viewsizeLightingBc.x, (float)m_viewsizeLightingBc.y);
	vertexData[2].setPosition(0.0f, (float)m_viewsizeLightingBc.y);
	vertexData[3].setPosition(0.0f, 0.0f);
	vertexData[4].setPosition((float)m_viewsizeLightingBc.x, (float)m_viewsizeLightingBc.y);
	vertexData[5].setPosition((float)m_viewsizeLightingBc.x, 0.0f);

	//Pixel
	glm::vec2 minn = glm::vec2((float)light::MAX_RANGE, (float)light::MAX_RANGE) / (glm::vec2)m_viewsizeLightingBc;
	glm::vec2 maxx = glm::vec2(1.0f, 1.0f) - minn;

	vertexData[6].setPosition(0.0f, 0.0f);
	vertexData[6].setUV(minn.x, maxx.y);
	vertexData[7].setPosition(m_viewsizePxLarger);
	vertexData[7].setUV(maxx.x, minn.y);
	vertexData[8].setPosition(0.0f, m_viewsizePxLarger.y);
	vertexData[8].setUV(minn.x, minn.y);
	vertexData[9].setPosition(0.0f, 0.0f);
	vertexData[9].setUV(minn.x, maxx.y);
	vertexData[10].setPosition(m_viewsizePxLarger);
	vertexData[10].setUV(maxx.x, minn.y);
	vertexData[11].setPosition(m_viewsizePxLarger.x, 0.0f);
	vertexData[11].setUV(maxx.x, maxx.y);


	//Uploading mesh and setting attributes
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLighting);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), &vertexData[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void WorldDrawer::createGLObjectsLighting() {
	if (m_VAOLighting == 0) {//If doesn't exist already
		glGenVertexArrays(1, &m_VAOLighting);
	}
	if (m_VBOLighting == 0) {//If doesn't exist already
		glGenBuffers(1, &m_VBOLighting);
	}
	glBindVertexArray(m_VAOLighting);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLighting);

	glEnableVertexAttribArray(RE::ATTR_POSITION);//Position
	glVertexAttribPointer(RE::ATTR_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(VertexPOUV), (void*)offsetof(VertexPOUV, position));
	glEnableVertexAttribArray(RE::ATTR_UV);//UV
	glVertexAttribPointer(RE::ATTR_UV, 2, GL_FLOAT, GL_FALSE, sizeof(VertexPOUV), (void*)offsetof(VertexPOUV, uv));

	glBindVertexArray(0);
}

void WorldDrawer::createGLObjectsStaticLights() {
	if (m_VAO_SLs == 0) {//If doesn't exist already
		glGenVertexArrays(1, &m_VAO_SLs);
	}
	if (m_VBO_SLs == 0) {//If doesn't exist already
		glGenBuffers(1, &m_VBO_SLs);
	}
	glBindVertexArray(m_VAO_SLs);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_SLs);

	glEnableVertexAttribArray(RE::ATTR_POSITION);//Position
	glVertexAttribPointer(RE::ATTR_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(StaticLight), (void*)offsetof(StaticLight, posBc));
	glEnableVertexAttribArray(RE::ATTR_COLOUR);//Colour
	glVertexAttribPointer(RE::ATTR_COLOUR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(StaticLight), (void*)offsetof(StaticLight, col));
	glEnableVertexAttribArray(2u);//Direction
	glVertexAttribPointer(2u, 1, GL_FLOAT, GL_FALSE, sizeof(StaticLight), (void*)offsetof(StaticLight, dir));
	glEnableVertexAttribArray(3u);//Cone
	glVertexAttribPointer(3u, 1, GL_FLOAT, GL_FALSE, sizeof(StaticLight), (void*)offsetof(StaticLight, cone));

	glBindVertexArray(0);
}

void WorldDrawer::createGLObjectsDynamicLights() {
	if (m_VAO_DLs == 0) {//If doesn't exist already
		glGenVertexArrays(1, &m_VAO_DLs);
	}
	if (m_VBO_DLs == 0) {//If doesn't exist already
		glGenBuffers(1, &m_VBO_DLs);
	}
	glBindVertexArray(m_VAO_DLs);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_DLs);

	glEnableVertexAttribArray(RE::ATTR_POSITION);//Position
	glVertexAttribPointer(RE::ATTR_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(DynamicLight), (void*)offsetof(DynamicLight, posPx));
	glEnableVertexAttribArray(RE::ATTR_COLOUR);//Colour
	glVertexAttribPointer(RE::ATTR_COLOUR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(DynamicLight), (void*)offsetof(DynamicLight, col));
	glEnableVertexAttribArray(2u);//Direction
	glVertexAttribPointer(2u, 1, GL_FLOAT, GL_FALSE, sizeof(DynamicLight), (void*)offsetof(DynamicLight, dir));
	glEnableVertexAttribArray(3u);//Cone
	glVertexAttribPointer(3u, 1, GL_FLOAT, GL_FALSE, sizeof(DynamicLight), (void*)offsetof(DynamicLight, cone));

	glBindVertexArray(0);
}

void WorldDrawer::initShaders(const glm::mat4& viewMatrix) {
	//Uniform Buffer
	RE::UniformManager::std.addUniformBuffer("WorldDrawUniforms", sizeof(WorldDrawUniforms));
	RE::UniformManager::std.addShader("WorldDrawUniforms", m_standardDraw.get());
	RE::UniformManager::std.addShader("WorldDrawUniforms", m_finalLighting.get());
	RE::UniformManager::std.addShader("WorldDrawUniforms", m_combineLighting.get());
	RE::UniformManager::std.addShader("WorldDrawUniforms", m_worldToLight.get());
	RE::UniformManager::std.addShader("WorldDrawUniforms", m_sumSL.get());
	RE::UniformManager::std.addShader("WorldDrawUniforms", m_addSL.get());
	RE::UniformManager::std.addShader("WorldDrawUniforms", m_addDL.get());
	updateUniformsAfterResize(viewMatrix);

	//STANDARD DRAW
	m_standardDraw->setUniform(WGS::LOC_WORLD_TEXTURE, TEX_UNIT_WORLD_TEXTURE.index());
	m_standardDraw->setUniform("blockTexture", 1);
	m_standardDraw->setUniform("wallTexture", 2);
	m_standardDraw->setUniform("blockSizePx", ivec2_BLOCK_SIZE);
	m_standardDraw->setUniform("lastBlock_lastVar", glm::ivec2((GLint)float_NUMBER_OF_BLOCKS, -1 * (GLint)float_NUMBER_OF_BLOCK_VARIATIONS));

	//FINAL LIGHTING
	m_finalLighting->setUniform(shaders::LOC_BASE_TEXTURE, TEX_UNIT_LIGHT_FINISHED.index());

	//COMBINE LIGHTING
	m_combineLighting->setUniform(WDS::LOC_DIAPHRAGM, TEX_UNIT_DIAPHRAGM.index());
	m_combineLighting->setUniform(WDS::LOC_LIGHTING, TEX_UNIT_LIGHTING.index());

	//WORLD TO LIGHT
	m_worldToLight->setUniform(WDS::LOC_WORLD_TEXTURE, TEX_UNIT_WORLD_TEXTURE.index());

	//SUM STATIC LIGHT

	//ADD STATIC LIGHT
	m_addSL->setUniform(WDS::LOC_DIAPHRAGM, TEX_UNIT_DIAPHRAGM.index());

	//ADD DYNAMIC LIGHT
	m_addDL->setUniform(WDS::LOC_DIAPHRAGM, TEX_UNIT_DIAPHRAGM.index());
	m_addDL->setUniform("blockSizePx", vec2_BLOCK_SIZE);
}

void WorldDrawer::updateUniformsAfterResize(const glm::mat4& newViewMatrix) {
	WorldDrawUniforms wdu;
	wdu.viewMat = newViewMatrix;
	wdu.viewsizePxMat = glm::ortho(0.0f, m_viewsizePx.x, 0.0f, m_viewsizePx.y);
	wdu.viewsizeLightingBcMat = glm::ortho(0.0f, (float)m_viewsizeLightingBc.x, 0.0f, (float)m_viewsizeLightingBc.y);
	RE::UniformManager::std.setUniformBuffer("WorldDrawUniforms", 0u, sizeof(glm::mat4) * 3, &wdu);
}

void WorldDrawer::staticLight_reserveInternal(size_t newCapacity) {
	size_t cap = m_staticLights.capacity();
	m_staticLights.reserve(newCapacity);
	if (cap < m_staticLights.capacity()) {//If its capacity was truly increased 
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO_SLs);
		glBufferData(GL_ARRAY_BUFFER, m_staticLights.capacity() * sizeof(StaticLight), NULL, GL_DYNAMIC_DRAW);//Resizing the VBO
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_staticLights.size() * sizeof(StaticLight), m_staticLights.data());//Uploading whole data
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}