#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "cinder/params/Params.h"

#include "Wave.h"
#include <librealsense2/rs.hpp>

#include "opencv2/opencv.hpp"

#define VERTEXNUM 100
#define MAPNUM 800

#define WAVENUM 10

#define MOTIONRES 500

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace cv;

class depthwaveApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
	void keyDown(KeyEvent event) override;
	static void prepare(App::Settings *settings);

  private:

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

	rs2::pipeline			realsense;
	gl::Texture2dRef		mDepthTexture;
	gl::Texture2dRef		mPrevDepthTexture;
	
	bool					mRSDepthFrameReady = false;
	uint16_t*				data = nullptr;

	Surface16u sf;
	gl::GlslProgRef			simpleShader;

	vector<int>				depthArray;
	vector<float>			prevDepthArray;

	rs2::colorizer			color_map;
	rs2::temporal_filter	temp_filter;
	
	float					far = 2.0;
	float					alpha = 0.0f;
	float					scale = 8.0f;
	Mat						frame1, prvs;
};

void depthwaveApp::setup()
{
	for (int i = 0; i < WAVENUM; i++) {
		mWave[i] = Wave::create(i * 30.0f - 250.0f);
		mWave[i]->mRippleAmplitude = 1 - i * 0.1;
	}

	mAmplitudeTarget = 60.0f;
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
	mFov = 2;
	mCamera.setFov(mFov);
	mParams->addParam("FOV", &mFov).max(100.0).min(0.0).step(1.0f).updateFn([&]() { mCamera.setFov(mFov); });
	mParams->addParam("far", &far).max(3.0).min(0.15).step(0.01f);
	mParams->addParam("alpha", &alpha).max(1.0).min(0.0).step(0.01f);
	mParams->addParam("scale", &scale).max(100.0).min(1.0).step(1.0f);
	mBackgroundShader = gl::GlslProg::create(loadAsset("backgroundShader.vert"), loadAsset("backgroundShader.frag"));

	simpleShader = gl::GlslProg::create(loadAsset("simple.vert"), loadAsset("simple.frag"));

	

	//auto callback = [&](const rs2::frame& frame)
	//{
	//	std::lock_guard<std::mutex> lock(mutex);
	//	if (rs2::frameset fs = frame.as<rs2::frameset>())
	//	{
	//		// With callbacks, all synchronized stream will arrive in a single frameset
	//		/*for (const rs2::frame& f : fs)
	//			counters[f.get_profile().unique_id()]++;*/
	//		rs2::depth_frame mRSDepthFrame = fs.get_depth_frame();
	//		data = (uint16_t *)mRSDepthFrame.get_data();
	//		int width = mRSDepthFrame.get_width();
	//		int height = mRSDepthFrame.get_height();
	//		sf = Surface16u(data, width, height, 4 * width, SurfaceChannelOrder(GL_R16UI));
	//		mRSDepthFrameReady = true;
	//	}
	//	else
	//	{
	//		// Stream that bypass synchronization (such as IMU) will produce single frames
	//		//counters[frame.get_profile().unique_id()]++;
	//	}
	//};

	//rs2::pipeline_profile profile = realsense.start(callback);
	rs2::config cfg;

	//Add desired streams to configuration
	//cfg.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, 30);
	cfg.enable_stream(RS2_STREAM_DEPTH, 640, 360, RS2_FORMAT_Z16, 60);
	rs2::pipeline_profile profile = realsense.start(cfg);
	color_map = rs2::colorizer(2);
	
	depthArray.clear();
	//mDepthTexture = gl::Texture::create(colorized_depth.get_data(), GL_RGB, width, height, gl::Texture::Format().loadTopDown());
	for (int y = 6; y < 48; y += 6) {
		for (int x = 1; x < 80; x += 2) {
			depthArray.push_back(0);
			prevDepthArray.push_back(0.0f);
		}
	}
}

void depthwaveApp::mouseDown( MouseEvent event )
{
}

void depthwaveApp::update()
{
	

	rs2::frameset fs = realsense.wait_for_frames();
	rs2::frame depth = fs.get_depth_frame();
	rs2::frame filtered = depth;
	rs2::decimation_filter  dec_filter;
	dec_filter.set_option(RS2_OPTION_FILTER_MAGNITUDE, scale);
	filtered = dec_filter.process(filtered);
	rs2::threshold_filter thr_filter(0.15f, far);
	filtered = thr_filter.process(filtered);
	//rs2::frame color_frame = fs.get_color_frame();
	//rs2::disparity_transform depth_to_disparity(true);
	//rs2::disparity_transform disparity_to_depth(false);
	//filtered = depth_to_disparity.process(filtered);
	temp_filter.set_option(rs2_option::RS2_OPTION_FILTER_SMOOTH_DELTA, 100.0f);
	temp_filter.set_option(rs2_option::RS2_OPTION_FILTER_SMOOTH_ALPHA, alpha);
	filtered = temp_filter.process(filtered);
	//filtered = disparity_to_depth.process(filtered);
	auto colorized_depth = color_map.colorize(filtered);
	int width = colorized_depth.get_width();
	int height = colorized_depth.get_height();
	//console() << width << " " << height << endl;
	//uint16_t* data = (uint16_t*)depth.get_data();
	
	Mat color(Size(640, 480), CV_8UC3, (void*)colorized_depth.get_data(), Mat::AUTO_STEP);
	//mDepthTexture = gl::Texture::create((void *)color.data, GL_RGB, color.cols, color.rows, gl::Texture::Format().loadTopDown());

	//Mat color(Size(640, 480), CV_16U, (void*)depth.get_data(), Mat::AUTO_STEP);
	//color.convertTo(color, CV_8UC3);
	//equalizeHist(color, color);
	//applyColorMap(color, color, COLORMAP_JET);
	//mDepthTexture = gl::Texture::create((void *)color.data, GL_BGR, color.cols, color.rows, gl::Texture::Format().loadTopDown());

	//depthArray.clear();
	//mDepthTexture = gl::Texture::create(colorized_depth.get_data(), GL_RGB, width, height);// , gl::Texture::Format().loadTopDown());
	Surface sf = Surface((uint8_t *)colorized_depth.get_data(), width, height, 3 * width, SurfaceChannelOrder::RGB);
	mDepthTexture = gl::Texture::create(sf);

	int i = 0;
	for (int y = 6; y < 48; y += 6) {
		for (int x = 1; x < 80; x += 2) {
			float d = sf.getPixel(ivec2(x,y)).r;
			
			if (abs(d - prevDepthArray[i]) > 10) {
				depthArray[i] = 1;
			}
			else {
				depthArray[i] = 0;
			}
			
			prevDepthArray[i] = d;
			//console() << depthArray[i] << "  ";
			i++;
		}
		//console() << endl;;
		
	}
	//console() << depthArray.size() << endl;
	//console() << endl;;
	


	//if( mRSDepthFrameReady ) {
	//	console() << sf.getPixel(ivec2(1280 / 2, 720 / 2)).r << endl;
	//	mRSDepthFrameReady = false;
	//}

	for (int i = 0; i < WAVENUM; i++) {
		mWave[i]->update(depthArray);
		mWave[i]->mAmplitudeTarget = mAmplitudeTarget * (1 - i * 0.08);
	}

	//mPrevDepthTexture = gl::Texture::create(mDepthTexture->getWidth(), mDepthTexture->getHeight());

	//ci::gl::CopyImageSubData(mDepthTexture, mPrevDepthTexture);
}

void depthwaveApp::draw()
{
	gl::clear(Color(0, 0, 0));

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

	for (int i = 0; i < WAVENUM; i++) {
		mWave[i]->draw();
	}

	gl::popMatrices();

	if (mDrawTexture) {
		gl::color(Color(0.05f, 0.05f, 0.05f));
		gl::draw(mWave[0]->mRippleMapFbo->getColorTexture(), vec2(0));
		gl::color(Color(1, 1, 1));
		gl::draw(mWave[0]->mNormalMapFbo->getColorTexture(), vec2(MAPNUM + 1, 0));
		gl::pushMatrices();
		gl::scale(5.0f, 5.0f);
		gl::draw(mDepthTexture, vec2(50, 0));
		gl::popMatrices();
	}

	mParams->draw();
	

	//if (true) {
	//	gl::ScopedGlslProg    scpProg(simpleShader);
	//	gl::ScopedTextureBind tex0(mDepthTexture, uint8_t(0));
	//	simpleShader->uniform("uTex", 0);
	//	Rectf	window = getWindowBounds();
	//	gl::drawSolidRect(Rectf(0.0, 0.0, getWindowWidth(), getWindowHeight()));
	//}
;	//auto texture = gl::Texture2d::create(sf);
	//gl::draw(texture);
	
	// load shader
	//gl::drawSolidRect(getWindowBounds());

	gl::drawString(to_string(App::get()->getAverageFps()), glm::vec2(10.0f, 10.0f), Color::white(), Font("Arial", 12.0f));
}

void depthwaveApp::keyDown(KeyEvent event)
{
	switch (event.getCode()) {
	case KeyEvent::KEY_s:
		// reload shaders
		for (int i = 0; i < WAVENUM; i++) {
			mWave[i]->compileShaders();
		}
		mBackgroundShader = gl::GlslProg::create(loadAsset("backgroundShader.vert"), loadAsset("backgroundShader.frag"));
		getWindow()->setFullScreen(false);
		break;
	case KeyEvent::KEY_f:
		getWindow()->setFullScreen();
		break;
	}
}

void depthwaveApp::prepare(App::Settings *settings)
{
	settings->setWindowSize(768, 768);
	//    settings->setFullScreen();
}

CINDER_APP(depthwaveApp, RendererGl(RendererGl::Options().msaa(16)), &depthwaveApp::prepare)
