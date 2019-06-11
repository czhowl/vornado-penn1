#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

#include "line.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define SIZE 48
#define LINES 18

class textAnimationApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

	Font					mFont;
	TypeLine				line[LINES];
	params::InterfaceGlRef	mParams;

	int						mInterval;
	int						mDelay;
	int						mLineDelay;
	string					mString = "          ";
};

void textAnimationApp::setup()
{
	mFont = Font(loadAsset("PennSansCompound2.otf"), 80);

	for (int i = 0; i < LINES; i++) {
		line[i].setup(mFont, i);
	}
	mParams = params::InterfaceGl::create(getWindow(), "App parameters", toPixels(ivec2(500, 200)));
	mInterval = 4;
	mDelay = 2;
	mLineDelay = 5;
	mParams->addParam("Frame Inteval", &mInterval).updateFn([this] {
		for (int i = 0; i < LINES; i++) {
			line[i].setInterval(mInterval);
		}
	});
	mParams->addParam("Frame Delay", &mDelay).updateFn([this] {
		for (int i = 0; i < LINES; i++) {
			line[i].setCharDelay(mDelay);
		}
	});
	mParams->addParam("Frame Line Delay", &mLineDelay).updateFn([this] {
		for (int i = 0; i < LINES; i++) {
			line[i].setLineDelay(mLineDelay);
		}
	});
	mParams->addParam("String", &mString).updateFn([this] {
		for (int i = 0; i < LINES; i++) {
			line[i].animate(mString);
		}
	});
}

void textAnimationApp::mouseDown( MouseEvent event )
{
}

void textAnimationApp::update()
{
	for (int i = 0; i < LINES; i++) {
		line[i].update();
	}
}

void textAnimationApp::draw()
{
	gl::clear( Color( 1,1,1 ) );
	//gl::drawSolidRect(rect);
	//gl::drawString("PENN", vec2(0), Color::black(), mFont);
	//gl::color(1, 1, 1);
	//gl::drawSolidRect(Rectf(0, 0, SIZE, SIZE / 2));
	gl::color(0, 0, 0);
	for (int i = 0; i < LINES; i++) {
		line[i].draw();
	}
	
	mParams->draw();
}

CINDER_APP( textAnimationApp, RendererGl )
