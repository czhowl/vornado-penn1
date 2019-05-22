#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define STEP 10
#define SCALE 2

class characterDisplayApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
	static void prepare(App::Settings *settings);

  private:
	gl::Texture2dRef		mTexture;
	Surface8u				mSurface;
	const string			dots[11] = { " ", ".", ",", "*", "x", "#", "8", "%", "$", "&", "@" };
	
	Font					mFont;
	vector<string>			mDisplay;
};

void characterDisplayApp::setup()
{
	auto img = loadImage(loadAsset("example1.jpg"));

	mSurface = Surface8u(img);
	ivec2 bound = mSurface.getSize();
	//getWindow()->setSize(bound * SCALE);
	Surface::Iter iter = mSurface.getIter(mSurface.getBounds());
	for (int y = 0; y < bound.y; y += STEP) {
		for (int x = 0; x < bound.x; x += STEP) {
			auto pixel = mSurface.getPixel(ivec2(x, y));
			//float l = .2126 * pow(pixel.r, 2.2) + .7152 * pow(pixel.g, 2.2) + .0722 * pow(pixel.b, 2.2);
			float l = pixel.r;
			int c = (int)(l / 255.0f * 10.0f);
			//console() << x + y * bound.x << "   " << bound.x * bound.y << endl;
			mDisplay.push_back(dots[c]);
		}
	}
	//for (char c : mDisplay) {
	//	console() << c << endl;
	//}

	mFont = Font(loadAsset("FSEX300.ttf"), 30.0f);
	
}

void characterDisplayApp::mouseDown( MouseEvent event )
{
}

void characterDisplayApp::update()
{
}

void characterDisplayApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	//gl::Texture2dRef mTex = gl::Texture::create(mSurface);
	//gl::draw(mTex);
	ivec2 bound = mSurface.getSize();
	//int size = mDisplay.size();
	string s = "";
	for (int y = 0; y * STEP < bound.y; y++) {
		for (int x = 0; x * STEP < bound.x; x++) {
			ivec2 pos = ivec2(x, y) * STEP * SCALE;
			s += mDisplay[x + y * bound.x / STEP];
			//gl::drawString(, pos, Color::white(), mFont);
		}
		s += '\n';
	}
	gl::pushMatrices();
	gl::scale(2.0, 1.0);
	gl::drawString(s, glm::vec2(10.0f, 10.0f), Color(1, 1, 1), mFont);
	gl::popMatrices();
	gl::drawString(to_string(App::get()->getAverageFps()), glm::vec2(10.0f, 10.0f), Color(1,0,0), Font("Arial", 12.0f));
}

void characterDisplayApp::prepare(App::Settings *settings)
{
	settings->setWindowSize(1920, 1080);
	//    settings->setFullScreen();
}

CINDER_APP(characterDisplayApp, RendererGl(RendererGl::Options().msaa(16)), &characterDisplayApp::prepare)
