#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "cinder/params/Params.h"

#include "Wave.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define VERTEXNUM 100
#define MAPNUM 800

#define WAVENUM 10

class displacementMapApp : public App {
public:
	void setup() override;
	void mouseDown(MouseEvent event) override;
	void update() override;
	void draw() override;
	void keyDown(KeyEvent event) override;
	static void prepare(App::Settings *settings);

private:
	bool					mMockUp;
	gl::Texture2dRef		mTextureWall;

	gl::GlslProgRef			mBackgroundShader;

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

	WaveRef					mWave[WAVENUM];
};

void displacementMapApp::setup()
{
	for (int i = 0; i < WAVENUM; i++) {
		mWave[i] = Wave::create(i * 30.0f - 250.0f);
		mWave[i]->mRippleAmplitude = 1 - i * 0.1;
	}
	

	auto img = loadImage(loadAsset("wall2.png"));
	mTextureWall = gl::Texture::create(img);

	ivec2 bound = mTextureWall->getSize();

	//getWindow()->setSize(bound / 1);

	mMockUp = false;
	mAmplitudeTarget = 20.0f;
	// Camera
	const vec2 windowSize = toPixels(getWindowSize());
	mCamera = CameraPersp(windowSize.x, windowSize.y, 10.0f, 0.01f, 50000.0f);
	mCameraO = CameraOrtho(0, windowSize.x, windowSize.y, 0, 0.01f, 10000.0f);
	mCamera.lookAt(vec3(-63.445f, 8432.912f, 29803.748f), vec3(0.0f, 0.0f, 0.0f));
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
	mFov = 1;
	mCamera.setFov(mFov);
	mParams->addParam("FOV", &mFov).max(100.0).min(0.0).step(1.0f).updateFn([&]() { mCamera.setFov(mFov); });
}

void displacementMapApp::mouseDown( MouseEvent event )
{
}

void displacementMapApp::update()
{
	for (int i = 0; i < WAVENUM; i++) {
		mWave[i]->update();
		mWave[i]->mAmplitudeTarget = mAmplitudeTarget * (1 - i * 0.1);
	}

	//console() << mCamera.getViewDirection << "    " << mCamera.getEyePoint() << endl;
}

void displacementMapApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 

	float t = mTime;

	//if (true) {
	//	gl::ScopedGlslProg    scpProg(mBackgroundShader);
	//	mBackgroundShader->uniform("uResolution", vec2(getWindowWidth(), getWindowHeight()));
	//	vec2 mousePos = getWindow()->getMousePos();
	//	//console() << mousePos.x / getWindowWidth() << endl;
	//	mBackgroundShader->uniform("uMouse", vec2(float(mousePos.x), float(mousePos.y)));
	//	mBackgroundShader->uniform("uTime", t);
	//	Rectf	window = getWindowBounds();
	//	gl::drawSolidRect(Rectf(0.0, 0.0, getWindowWidth(), getWindowHeight()));
	//}

	gl::pushMatrices();
	gl::setMatrices(mCamera);

	for (int i = 0; i < WAVENUM; i++) {
		mWave[i]->draw();
	}

	gl::popMatrices();

	if (mDrawTexture) {
		gl::color(Color(0.05f, 0.05f, 0.05f));
		gl::draw(mWave[0]->mRippleMapFbo->getColorTexture(), vec2(0));
		gl::color(Color(1, 1, 1));
		gl::draw(mWave[0]->mNormalMapFbo->getColorTexture(), vec2(MAPNUM + 1, 0));
	}

	mParams->draw();

	//console() << mCamera.getEyePoint() << endl;
}

void displacementMapApp::keyDown(KeyEvent event)
{
	switch (event.getCode()) {
	case KeyEvent::KEY_s:
		// reload shaders
		for (int i = 0; i < WAVENUM; i++) {
			mWave[i]->compileShaders();
		}
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