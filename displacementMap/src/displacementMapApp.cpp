#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"

#include "cinder/params/Params.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define VERTEXNUM 100
#define MAPNUM 800

class displacementMapApp : public App {
public:
	void setup() override;
	void mouseDown(MouseEvent event) override;
	void update() override;
	void draw() override;
	void keyDown(KeyEvent event) override;
	static void prepare(App::Settings *settings);

private:
	bool					compileShaders();
	void					createMesh();
	void					renderDisplacementMap();
	void					renderNormalMap();
	bool					mMockUp;
	gl::Texture2dRef		mTextureWall;

	gl::GlslProgRef			mBackgroundShader;
	gl::GlslProgRef			mMeshShader;
	gl::VboMeshRef			mVboMesh;
	gl::BatchRef			mBatch;

	gl::FboRef				mDispMapFbo;
	gl::GlslProgRef			mDispMapShader;

	gl::FboRef				mNormalMapFbo;
	gl::GlslProgRef			mNormalMapShader;

	float					mAmplitude;
	float					mAmplitudeTarget;
	float					mTime;
	float					mTime2;

	CameraPersp				mCamera;
	CameraOrtho				mCameraO;
	CameraUi				mCamUi;

	bool					mDrawTexture;
	float					mFov;

	ci::params::InterfaceGlRef	mParams;
};

void displacementMapApp::setup()
{
	auto img = loadImage(loadAsset("wall2.png"));
	mTextureWall = gl::Texture::create(img);

	ivec2 bound = mTextureWall->getSize();

	// create the frame buffer objects for the displacement map and the normal map
	gl::Fbo::Format fmt;
	fmt.enableDepthBuffer(false);

	// use a single channel (red) for the displacement map
	fmt.setColorTextureFormat(gl::Texture2d::Format().wrap(GL_CLAMP_TO_EDGE).internalFormat(GL_R32F));
	mDispMapFbo = gl::Fbo::create(MAPNUM, MAPNUM, fmt);

	// use 3 channels (rgb) for the normal map
	fmt.setColorTextureFormat(gl::Texture2d::Format().wrap(GL_CLAMP_TO_EDGE).internalFormat(GL_RGB32F));
	mNormalMapFbo = gl::Fbo::create(MAPNUM, MAPNUM, fmt);

	mAmplitude = 0.0f;
	mAmplitudeTarget = 10.0f;

	//getWindow()->setSize(bound / 1);
	compileShaders();
	createMesh();
	mMockUp = false;
	// Camera
	const vec2 windowSize = toPixels(getWindowSize());
	mCamera = CameraPersp(windowSize.x, windowSize.y, 10.0f, 0.01f, 10000.0f);
	mCameraO = CameraOrtho(0, windowSize.x, windowSize.y, 0, 0.01f, 10000.0f);
	mCamera.lookAt(vec3(0.0f, 0.0f, 80.0f), vec3(0.0f, 0.0f, 0.0f));
	mCameraO.lookAt(vec3(0.0f, 0.0f, 80.0f), vec3(0.0f, 0.0f, 0.0f));
	mCamUi = CameraUi(&mCamera, getWindow(), -1);
	mTime = 0;
	mTime2 = 0;
	mDrawTexture = false;
	mParams = params::InterfaceGl::create("Params", ivec2(220, 220));
	mParams->addParam("amplitude", &mAmplitudeTarget).max(100.0).min(0.0).step(0.1f);
	mParams->addParam("bg", &mTime).max(100.0).min(0.0).step(0.01f);
	mParams->addParam("flt", &mTime2).max(10.0).min(0.0).step(0.01f);
	mParams->addParam("draw texture", &mDrawTexture);
	mFov = 11;
	mCamera.setFov(mFov);
	mParams->addParam("FOV", &mFov).max(100.0).min(0.0).step(1.0f).updateFn([&]() { mCamera.setFov(mFov); });
}

void displacementMapApp::mouseDown( MouseEvent event )
{
}

void displacementMapApp::update()
{
	mAmplitude += 0.02f * (mAmplitudeTarget - mAmplitude);

	// render displacement map
	renderDisplacementMap();

	// render normal map
	renderNormalMap();
}

void displacementMapApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 

	float t = mTime;

	if (true) {
		gl::ScopedGlslProg    scpProg(mBackgroundShader);
		mBackgroundShader->uniform("uResolution", vec2(getWindowWidth(), getWindowHeight()));
		vec2 mousePos = getWindow()->getMousePos();
		//console() << mousePos.x / getWindowWidth() << endl;
		mBackgroundShader->uniform("uMouse", vec2(float(mousePos.x), float(mousePos.y)));
		mBackgroundShader->uniform("uTime", t);
		Rectf	window = getWindowBounds();
		gl::drawSolidRect(Rectf(0.0, 0.0, getWindowWidth(), getWindowHeight()));
	}
	gl::pushMatrices();
	gl::setMatrices(mCamera);

	gl::enableAdditiveBlending();

	if (mDispMapFbo && mNormalMapFbo && mMeshShader) {
		// bind the displacement and normal maps, each to their own texture unit
		gl::ScopedTextureBind tex0(mDispMapFbo->getColorTexture(), uint8_t(0));
		gl::ScopedTextureBind tex1(mNormalMapFbo->getColorTexture(), uint8_t(1));

		// render our mesh using vertex displacement
		gl::ScopedGlslProg shader(mMeshShader);
		mMeshShader->uniform("uTexDisplacement", 0);
		mMeshShader->uniform("uTexNormal", 1);
		mMeshShader->uniform("uTime", mTime2);

		gl::color(Color::white());
		mBatch->draw();
	}
	gl::disableAlphaBlending();
	gl::popMatrices();

	if (mDrawTexture) {
		gl::color(Color(0.05f, 0.05f, 0.05f));
		gl::draw(mDispMapFbo->getColorTexture(), vec2(0));
		gl::color(Color(1, 1, 1));
		gl::draw(mNormalMapFbo->getColorTexture(), vec2(MAPNUM + 1, 0));
	}
	mParams->draw();
}

void displacementMapApp::renderDisplacementMap()
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
		gl::ScopedGlslProg shader(mDispMapShader);
		mDispMapShader->uniform("uTime", float(getElapsedSeconds()));
		mDispMapShader->uniform("uAmplitude", mAmplitude);
		vec2 mousePos = getWindow()->getMousePos();
		mDispMapShader->uniform("uMouse", float(mousePos.x / getWindowWidth()));
		gl::drawSolidRect(mDispMapFbo->getBounds());

		// clean up after ourselves
		gl::popMatrices();
	}
}

void displacementMapApp::renderNormalMap()
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

bool displacementMapApp::compileShaders()
{
	try {
		// this shader will render all colors using a change in hue
		mBackgroundShader = gl::GlslProg::create(loadAsset("backgroundShader.vert"), loadAsset("backgroundshader.frag"));
		mMeshShader = gl::GlslProg::create(loadAsset("meshShader.vert"), loadAsset("meshShader.frag"));
		// this shader will render a displacement map to a floating point texture, updated every frame
		mDispMapShader = gl::GlslProg::create(loadAsset("displacement_map.vert"), loadAsset("displacement_map.frag"));
		// this shader will create a normal map based on the displacement map
		mNormalMapShader = gl::GlslProg::create(loadAsset("normal_map.vert"), loadAsset("normal_map.frag"));
		createMesh();
	}
	catch (const std::exception &e) {
		console() << e.what() << std::endl;
		return false;
	}

	return true;
}

void displacementMapApp::createMesh()
{
	// create vertex, normal and texcoord buffers
	const int  RES_X = 600;
	const int  RES_Z = VERTEXNUM;
	const vec3 size = vec3(1000.0f, 1.0f, 100.0f);

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

void displacementMapApp::keyDown(KeyEvent event)
{
	switch (event.getCode()) {
	case KeyEvent::KEY_s:
		// reload shaders
		compileShaders();
		break;
	case KeyEvent::KEY_m:
		// reload shaders
		mMockUp = !mMockUp;
		break;
	}
}

void displacementMapApp::prepare(App::Settings *settings)
{
	settings->setWindowSize(1920, 1080);
	//    settings->setFullScreen();
}

CINDER_APP( displacementMapApp, RendererGl(RendererGl::Options().msaa(16)), &displacementMapApp::prepare)