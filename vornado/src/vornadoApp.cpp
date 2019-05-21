#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "cinder/Perlin.h"
#include "cinder/Rand.h"

#include <astra/astra.hpp>

using namespace ci;
using namespace ci::app;
using namespace std;

#define TESTSIZE 300

class vornadoApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
	void keyDown(KeyEvent event) override;

  private:
	bool					compileShaders();
	bool					mMockUp;
	gl::Texture2dRef		mTextureWall;

	gl::GlslProgRef			mShader;

	Perlin					mPerlin;
};

void vornadoApp::setup()
{
	// load wall mock up
	auto img = loadImage(loadAsset("wall2.png"));
	mTextureWall = gl::Texture::create(img);

	ivec2 bound = mTextureWall->getSize();
	getWindow()->setSize(bound / 1);

	// shader
	mShader = gl::GlslProg::create(loadAsset("shader.vert"), loadAsset("shader.frag"));

	mMockUp = false;

	mPerlin.setSeed(clock());
}

void vornadoApp::mouseDown( MouseEvent event )
{
}

void vornadoApp::update()
{
}

void vornadoApp::draw()
{
	float t = getElapsedSeconds();

	gl::clear( Color( 1, 1, 1 ) ); 


	if (true) {
		gl::ScopedGlslProg    scpProg(mShader);
		mShader->uniform("uResolution", vec2(getWindowWidth(), getWindowHeight()));
		vec2 mousePos = getWindow()->getMousePos();
		//console() << mousePos.x / getWindowWidth() << endl;
		mShader->uniform("uMouse", vec2(float(mousePos.x), float(mousePos.y)));
		mShader->uniform("uTime", t);
		mShader->uniform("uNoise", mPerlin.noise(t));
		Rectf	window = getWindowBounds();
		gl::drawSolidRect(Rectf(0.0, 0.0, getWindowWidth(), getWindowHeight()));
	}

	// wall mock up

	if(mMockUp){
		gl::pushMatrices();
		gl::scale(1.0f / 1, 1.0f / 1);
		gl::draw(mTextureWall);
		gl::popMatrices();
	}
	if (false) {
		gl::ScopedGlslProg    scpProg(mShader);
		mShader->uniform("uResolution", vec2(getWindowWidth(), getWindowHeight()));
		vec2 mousePos = getWindow()->getMousePos();
		//console() << mousePos.x / getWindowWidth() << endl;
		mShader->uniform("uMouse", vec2(float(mousePos.x), float(mousePos.y)));
		mShader->uniform("uTime", t);
		mShader->uniform("uNoise", mPerlin.noise(t));
		gl::drawSolidRect(Rectf(0.0, getWindowHeight() - TESTSIZE, TESTSIZE, getWindowHeight()));
	}
}

bool vornadoApp::compileShaders()
{
	try {
		// this shader will render all colors using a change in hue
		mShader = gl::GlslProg::create(loadAsset("shader.vert"), loadAsset("shader.frag"));
	}
	catch (const std::exception &e) {
		console() << e.what() << std::endl;
		return false;
	}

	return true;
}

void vornadoApp::keyDown(KeyEvent event)
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

CINDER_APP( vornadoApp, RendererGl )
