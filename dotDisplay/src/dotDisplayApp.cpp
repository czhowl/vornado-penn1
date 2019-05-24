#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "opencv2/opencv.hpp"

using namespace ci;
using namespace cv;
using namespace ci::app;
using namespace std;

#define STEP 2
#define SCALE 15.0f

#define WIDTH 256
#define HEIGHT 448

#define OFFSET 5

#define OFFSETSM 3
#define OFFSETLG 4

class dotDisplayApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
	static void prepare(App::Settings *settings);

private:
	gl::Texture2dRef		mTexture;
	Surface8u				mSurface;
	const char				dots[11] = { ' ', '.', ',', '*', 'x', '#', '8', '%', '$', '&', '@' };

	Font					mFont;
	vector<char>			mDisplay;

	//ciWMFVideoPlayer		mVideo1;
	ivec2					mBound;

	VideoCapture			mCap;
};

void dotDisplayApp::setup()
{
	//mCap = VideoCapture("../assets/kda.mp4");

	mCap = VideoCapture(0);
	if (!mCap.isOpened()) {
		cout << "Error opening video stream or file" << endl;
	}

	mFont = Font(loadAsset("FSEX300.ttf"), SCALE);

	mBound = ivec2(0, 0);

}

void dotDisplayApp::mouseDown(MouseEvent event)
{
}

void dotDisplayApp::update()
{
	//mVideo1.update();


	Mat frame;
	mCap >> frame;
	//auto sf = Surface::create( (uint8_t *)frame.data, frame.cols, frame.rows, frame.step[0], SurfaceChannelOrder( GL_BGR ) );

	mTexture = gl::Texture::create((void *)frame.data, GL_BGR, frame.cols, frame.rows, gl::Texture::Format().loadTopDown());

	Surface sf = Surface(mTexture->createSource());
	mBound = sf.getSize();
	mDisplay.clear();
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			auto pixel = sf.getPixel(ivec2(x, y + 21) * STEP);
			float l = pixel.r;
			int c = (int)(l / 255.0f * 10.0f);
			mDisplay.push_back(dots[c]);
		}
	}
}

void dotDisplayApp::draw()
{
	gl::clear(Color(0, 0, 0));
	//gl::draw(mTexture);

	string w = "WELCOME TO PENN 1                             ";

	for (int i = 0; i < w.size(); i++) {
		//console() << mDisplay[i] << "   " << w[i] << endl;
		mDisplay[i] = w[i];
	}

	gl::pushMatrices();
	//gl::scale(0.97, 97);
	gl::color(1,1,1);
	gl::begin(GL_POINTS);
	string s = "";
	
	int posX = OFFSET;
	int posY = OFFSET;
	int lastX = OFFSET;
	int lastY = OFFSET;
	int paddingX = 5;
	int paddingY = 5;
	for (int y = 0; y < HEIGHT; y++) {
		lastX = OFFSET;
		paddingY = y % 8 == 0 ? OFFSETLG : OFFSETSM;
		posY = lastY + paddingY;
		lastY = posY;
		for (int x = 0; x < WIDTH; x++) {
			s += mDisplay[x + y * WIDTH];

			paddingX = x % 8 == 0 ? OFFSETLG : OFFSETSM;
			posX = lastX + paddingX;
			gl::vertex(posX, posY);
			lastX = posX;
		}
		s += '\n';
	}
	gl::end();
	gl::popMatrices();
	gl::drawString(to_string(App::get()->getAverageFps()), glm::vec2(10.0f, 10.0f), Color(1, 0, 0), Font("Arial", 12.0f));
}

void dotDisplayApp::prepare(App::Settings *settings)
{
	settings->setWindowSize(1920, 1080);

	settings->setFrameRate(30);
}

CINDER_APP(dotDisplayApp, RendererGl(RendererGl::Options().msaa(16)), &dotDisplayApp::prepare)
