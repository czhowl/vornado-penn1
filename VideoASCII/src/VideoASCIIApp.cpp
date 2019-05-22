#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

//#include "ciWMFVideoPlayer.h"

#include "opencv2/opencv.hpp"

using namespace ci;
using namespace cv;
using namespace ci::app;
using namespace std;

#define STEP 10
#define SCALE 1

class VideoASCIIApp : public App {
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

	//ciWMFVideoPlayer		mVideo1;
	ivec2					mBound;

	VideoCapture			mCap;
};

void VideoASCIIApp::setup()
{
	auto img = loadImage(loadAsset("example1.jpg"));

	mSurface = Surface8u(img);
	
	//getWindow()->setSize(bound * SCALE);
	Surface::Iter iter = mSurface.getIter(mSurface.getBounds());
	
	//for (char c : mDisplay) {
	//	console() << c << endl;
	//}
	//mCap = VideoCapture("../assets/kda.mp4");
	mCap = VideoCapture(0);
	if (!mCap.isOpened()) {
		cout << "Error opening video stream or file" << endl;
	}
	string videoPath = getAssetPath("kda.mp4").string();
	//mVideo1.loadMovie(videoPath, "Headphones");
	//mVideo1.play();
	//mVideo1.getPresentationEndedSignal().connect([]() {
	//	ci::app::console() << "Video finished playing!" << std::endl;
	//});

	mFont = Font(loadAsset("FSEX300.ttf"), 30.0f);

	mBound = ivec2(0, 0);

}

void VideoASCIIApp::mouseDown(MouseEvent event)
{
}

void VideoASCIIApp::update()
{
	//mVideo1.update();
	
	
	Mat frame;
	mCap >> frame;
	//console() << int(frame.step[0]) << endl;
	//auto sf = Surface::create( (uint8_t *)frame.data, frame.cols, frame.rows, frame.step[0], SurfaceChannelOrder( GL_BGR ) );

	mTexture = gl::Texture::create((void *)frame.data, GL_BGR, frame.cols, frame.rows, gl::Texture::Format().loadTopDown());

	Surface sf = Surface( mTexture->createSource());
	mBound = sf.getSize();
	mDisplay.clear();
	for (int y = 0; y < mBound.y; y += STEP) {
		for (int x = 0; x < mBound.x; x += STEP) {
			auto pixel = sf.getPixel(ivec2(x, y));
			//console() << pixel << endl;
			//float l = .2126 * pow(pixel.r, 2.2) + .7152 * pow(pixel.g, 2.2) + .0722 * pow(pixel.b, 2.2);
			float l = pixel.r;
			int c = (int)(l / 255.0f * 10.0f);
			//console() << x + y * bound.x << "   " << bound.x * bound.y << endl;
			mDisplay.push_back(dots[c]);
		}
	}
	//writeImage("image.jpg", sf);
	//console() << sf.getPixel(ivec2(500, 300)) << endl;
}

void VideoASCIIApp::draw()
{
	gl::clear(Color(0, 0, 0));
	//gl::draw(mTexture);
	//mVideo1.draw(0, 0);

	string s = "";
	for (int y = 0; y * STEP < mBound.y; y++) {
		for (int x = 0; x * STEP < mBound.x; x++) {
			ivec2 pos = ivec2(x, y) * STEP * SCALE;
			s += mDisplay[x + y * mBound.x / STEP];
			//gl::drawString(, pos, Color::white(), mFont);
		}
		s += '\n';
	}

	//console() << s << endl;

	gl::pushMatrices();
	gl::scale(2.0, 1.0);
	gl::drawString(s, glm::vec2(10.0f, 10.0f), Color(1, 1, 1), mFont);
	gl::popMatrices();
	gl::drawString(to_string(App::get()->getAverageFps()), glm::vec2(10.0f, 10.0f), Color(1, 0, 0), Font("Arial", 12.0f));
}

void VideoASCIIApp::prepare(App::Settings *settings)
{
	settings->setWindowSize(1920, 1080);
	//    settings->setFullScreen();

	settings->setFrameRate( 30 );
}

CINDER_APP(VideoASCIIApp, RendererGl(RendererGl::Options().msaa(16)), &VideoASCIIApp::prepare)