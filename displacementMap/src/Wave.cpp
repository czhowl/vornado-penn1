#include "Wave.h"

using namespace ci;
using namespace ci::app;
using namespace std;
// ====================================================================== Wave

#define VERTEXNUM 100
#define MAPNUM 800

Wave::Wave()
{ }

// ======================================================================= Draw

void Wave::createMesh()
{
	// create vertex, normal and texcoord buffers
	const int  RES_X = 600;
	const int  RES_Z = VERTEXNUM;
	const vec3 size = vec3(1000.0f, 1.0f, 560.0f);

	std::vector<vec3> positions(RES_X * RES_Z);
	std::vector<vec3> normals(RES_X * RES_Z);
	std::vector<vec2> texcoords(RES_X * RES_Z);

	int i = 0;
	for (int x = 0; x < RES_X; ++x) {
		for (int z = 0; z < RES_Z; ++z) {
			const float u = float(x) / RES_X;
			const float v = float(z) / RES_Z;
			positions[i] = size * vec3(u - 0.5f, 0.0f, v - 0.5f);
			normals[i] = vec3(0, 1, 0);
			texcoords[i] = vec2(u, v);

			i++;
		}
	}

	// create index buffer
	vector<uint16_t> indices;
	indices.reserve(6 * (RES_X - 1) * (RES_Z - 1));

	for (int x = 0; x < RES_X - 1; ++x) {
		for (int z = 0; z < RES_Z - 1; ++z) {
			uint16_t i = x * RES_Z + z;

			indices.push_back(i);
			indices.push_back(i + 1);
			indices.push_back(i + RES_Z);
			indices.push_back(i + RES_Z);
			indices.push_back(i + 1);
			indices.push_back(i + RES_Z + 1);
		}
	}

	// construct vertex buffer object
	gl::VboMesh::Layout layout;
	layout.attrib(geom::POSITION, 3);
	layout.attrib(geom::NORMAL, 3);
	layout.attrib(geom::TEX_COORD_0, 2);

	mVboMesh = gl::VboMesh::create(positions.size(), GL_TRIANGLES, { layout }, indices.size());
	mVboMesh->bufferAttrib(geom::POSITION, positions.size() * sizeof(vec3), positions.data());
	mVboMesh->bufferAttrib(geom::NORMAL, normals.size() * sizeof(vec3), normals.data());
	mVboMesh->bufferAttrib(geom::TEX_COORD_0, texcoords.size() * sizeof(vec2), texcoords.data());
	mVboMesh->bufferIndices(indices.size() * sizeof(uint16_t), indices.data());

	// create a batch for better performance
	mBatch = gl::Batch::create(mVboMesh, mMeshShader);
}


void Wave::setup(float offset)
{
	mOffset = offset;
	mAmplitude = 0.0f;
	mAmplitudeTarget = 30.0f;
	// create the frame buffer objects for the displacement map and the normal map
	gl::Fbo::Format fmt;
	fmt.enableDepthBuffer(false);

	// use a single channel (red) for the displacement map
	fmt.setColorTextureFormat(gl::Texture2d::Format().wrap(GL_CLAMP_TO_EDGE).internalFormat(GL_R32F));
	mDispMapFbo = gl::Fbo::create(MAPNUM, MAPNUM, fmt);
	fmt.setColorTextureFormat(gl::Texture2d::Format().wrap(GL_CLAMP_TO_EDGE).internalFormat(GL_RG32F));
	mRippleMapFbo = gl::Fbo::create(MAPNUM, MAPNUM, fmt);
	// use 3 channels (rgb) for the normal map
	fmt.setColorTextureFormat(gl::Texture2d::Format().wrap(GL_CLAMP_TO_EDGE).internalFormat(GL_RGB32F));
	mNormalMapFbo = gl::Fbo::create(MAPNUM, MAPNUM, fmt);

	compileShaders();
	createMesh();

	mTimeOffset = Rand::randFloat(-1000.0f, 1000.0f);
}

// ===================================================================== Update

void Wave::update()
{
	mAmplitude += 0.02f * (mAmplitudeTarget - mAmplitude);

	// render displacement map
	renderRippleMap();
	renderDisplacementMap();
	
	// render normal map
	renderNormalMap();
}

// ======================================================================= Draw

void Wave::draw()
{
	gl::enableAdditiveBlending();

	if (mDispMapFbo && mNormalMapFbo && mMeshShader) {
		// bind the displacement and normal maps, each to their own texture unit
		gl::ScopedTextureBind tex0(mDispMapFbo->getColorTexture(), uint8_t(0));
		gl::ScopedTextureBind tex1(mNormalMapFbo->getColorTexture(), uint8_t(1));

		// render our mesh using vertex displacement
		gl::ScopedGlslProg shader(mMeshShader);
		mMeshShader->uniform("uTexDisplacement", 0);
		mMeshShader->uniform("uTexNormal", 1);

		mMeshShader->uniform("uOffset", mOffset);

		gl::color(Color::white());
		mBatch->draw();
		//gl::draw(mVboMesh);
	}
	gl::disableAlphaBlending();
}

// ======================================================================= Draw

void Wave::compileShaders()
{
	try {
		// this shader will render all colors using a change in hue
		mMeshShader = gl::GlslProg::create(loadAsset("meshShader.vert"), loadAsset("meshShader.frag"));
		// this shader will render a displacement map to a floating point texture, updated every frame
		mDispMapShader = gl::GlslProg::create(loadAsset("displacement_map.vert"), loadAsset("displacement_map.frag"));
		mRippleMapShader = gl::GlslProg::create(loadAsset("ripple_map.vert"), loadAsset("ripple_map.frag"));
		// this shader will create a normal map based on the displacement map
		mNormalMapShader = gl::GlslProg::create(loadAsset("normal_map.vert"), loadAsset("normal_map.frag"));
		createMesh();
	}
	catch (const std::exception &e) {
		console() << e.what() << std::endl;
	}
}

// ======================================================================= Draw

void Wave::renderDisplacementMap()
{
	if (mDispMapShader && mDispMapFbo) {
		// bind frame buffer
		gl::ScopedFramebuffer fbo(mDispMapFbo);

		// setup viewport and matrices
		gl::ScopedViewport viewport(0, 0, mDispMapFbo->getWidth(), mDispMapFbo->getHeight());

		gl::pushMatrices();
		gl::setMatricesWindow(mDispMapFbo->getSize());

		// clear the color buffer
		gl::clear();

		// render the displacement map
		gl::ScopedTextureBind tex0(mRippleMapFbo->getColorTexture());
		gl::ScopedGlslProg shader(mDispMapShader);
		mRippleMapShader->uniform("uTexRipple", 0);
		mDispMapShader->uniform("uTime", float(getElapsedSeconds() + mTimeOffset));
		mDispMapShader->uniform("uAmplitude", mAmplitude);
		gl::drawSolidRect(mDispMapFbo->getBounds());

		// clean up after ourselves
		gl::popMatrices();
	}
}

void Wave::renderRippleMap() {
	if (mRippleMapShader && mRippleMapFbo) {
		// bind frame buffer
		gl::ScopedFramebuffer fbo(mRippleMapFbo);

		// setup viewport and matrices
		gl::ScopedViewport viewport(0, 0, mRippleMapFbo->getWidth(), mRippleMapFbo->getHeight());

		gl::pushMatrices();
		gl::setMatricesWindow(mRippleMapFbo->getSize());

		// render the displacement map
		gl::ScopedTextureBind tex0(mRippleMapFbo->getColorTexture());


		gl::ScopedGlslProg shader(mRippleMapShader);
		mRippleMapShader->uniform("uTexDisplacement", 0);
		mRippleMapShader->uniform("uTime", float(getElapsedSeconds() + mTimeOffset));
		vec2 mousePos = getWindow()->getMousePos();
		mRippleMapShader->uniform("uMouse", float(mousePos.x / getWindowWidth()));
		if (mPrevMouse.x != mousePos.x) {
			mRippleMapShader->uniform("uRipple", int(1));
		}
		else {
			mRippleMapShader->uniform("uRipple", int(0));
		}
		gl::drawSolidRect(mRippleMapFbo->getBounds());

		// clean up after ourselves
		gl::popMatrices();

		mPrevMouse = mousePos;
	}
}

// ======================================================================= Draw

void Wave::renderNormalMap()
{
	if (mNormalMapShader && mNormalMapFbo) {
		// bind frame buffer
		gl::ScopedFramebuffer fbo(mNormalMapFbo);

		// setup viewport and matrices
		gl::ScopedViewport viewport(0, 0, mNormalMapFbo->getWidth(), mNormalMapFbo->getHeight());

		gl::pushMatrices();
		gl::setMatricesWindow(mNormalMapFbo->getSize());

		// clear the color buffer
		gl::clear();

		// bind the displacement map
		gl::ScopedTextureBind tex0(mDispMapFbo->getColorTexture());

		// render the normal map
		gl::ScopedGlslProg shader(mNormalMapShader);
		mNormalMapShader->uniform("uTex0", 0);
		mNormalMapShader->uniform("uAmplitude", 4.0f);

		const Area bounds = mNormalMapFbo->getBounds();
		gl::drawSolidRect(bounds);

		// clean up after ourselves
		gl::popMatrices();
	}
}


// ======================================================================== End