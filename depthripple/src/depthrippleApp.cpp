#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "cinder/params/Params.h"
#include "cinder/Perlin.h"

#include <librealsense2/rs.hpp>

using namespace ci;
using namespace ci::app;
using namespace std;

class depthrippleApp : public App {
public:
	void setup() override;
	void mouseDown(MouseEvent event) override;
	void update() override;
	void draw() override;
	static void prepare(Settings *settings);
	void keyDown(KeyEvent event) override;

private:
	void						createMesh();
	void						compileShaders();
	void						renderDisplacementMap();
	void						renderNormalMap();
	void						dropRipple(vec2 pos);
	void						updateRipple();

	params::InterfaceGlRef		mParams;

	gl::VboMeshRef				mVboMesh;
	gl::GlslProgRef				mMeshShader;
	gl::BatchRef				mBatch;

	gl::FboRef					mDispMapFbo;
	gl::GlslProgRef				mDispMapShader;

	gl::FboRef					mNormalMapFbo;
	gl::GlslProgRef				mNormalMapShader;

	//gl::FboRef					mRippleFbo;
	gl::FboRef					mRippleFboA;
	gl::FboRef					mRippleFboB;

	gl::GlslProgRef				mDropShader;
	gl::GlslProgRef				mRippleShader;

	//gl::Texture2dRef			mSkyTexture;
	gl::TextureCubeMapRef		mCubeMap;
	gl::Texture2dRef			mWaterBottom;

	CameraPersp					mCamera;
	CameraUi					mCameraUi;

	float						mAmplitude;
	float						mAmplitudeTarget;
	float						mStrength;
	float						mRadius;
	vec3						mSunDir;

	bool						mDrawTex;

	Perlin						mPerlin;
	vec2						mWalker;
	bool						mWalk;

	// sensing

	rs2::pipeline			realsense;
	rs2::colorizer			color_map;
	rs2::temporal_filter	temp_filter;

	gl::Texture2dRef		mDepthTexture;
	gl::Texture2dRef		mPrevDepthTexture;

	vector<int>				depthArray;
	vector<float>			prevDepthArray;

	float					far = 1.5;
	float					alpha = 0.0f;
	float					scale = 8.0f;
};

void depthrippleApp::setup()
{
	//mSkyTexture = gl::Texture2d::create(loadImage(loadAsset("sky.jpg")));
	const ImageSourceRef box[6] = {
		loadImage(loadAsset("xpos.jpg")),
		loadImage(loadAsset("xneg.jpg")),
		loadImage(loadAsset("ypos.jpg")),
		loadImage(loadAsset("ypos.jpg")),
		loadImage(loadAsset("zpos.jpg")),
		loadImage(loadAsset("zneg.jpg"))
	};
	const ImageSourceRef box2 = loadImage(loadAsset("skybox.jpg"));
	//mCubeMap = gl::TextureCubeMap::create(box, gl::TextureCubeMap::Format().magFilter(GL_LINEAR));
	mCubeMap = gl::TextureCubeMap::create(box2, gl::TextureCubeMap::Format().mipmap());

	mWaterBottom = gl::Texture2d::create(loadImage(loadAsset("tilesLT2.jpg")));

	mCamera = CameraPersp(768, 768, 10.0f, 0.01f, 50000.0f);
	mCameraUi = CameraUi(&mCamera, getWindow(), -1);
	mCamera.lookAt(vec3(0.0f, 10000.0f, 0.1f), vec3(0.0f, 0.0f, 0.0f));
	mCamera.setFov(1);
	mAmplitude = 0.0f;
	mAmplitudeTarget = 2.0f;
	mStrength = 1.0f;
	mRadius = 0.1f;
	mWalk = true;
	mWalker = vec2(0.5, 0.5);

	mDrawTex = true;
	mParams = params::InterfaceGl::create("Params", ivec2(220, 220));
	mParams->addParam("amplitude", &mAmplitudeTarget).max(100.0).min(0.0).step(0.01f);
	mParams->addParam("texture", &mDrawTex);
	mParams->addParam("strength", &mStrength).max(10.0).min(0.0).step(0.01f);
	mParams->addParam("drop size", &mRadius).max(0.5).min(0.0).step(0.01f);
	mParams->addParam("Walker", &mWalk);
	mParams->addParam("far clip", &far).max(2.0).min(0.0).step(0.01f);

	compileShaders();

	createMesh();

	gl::Fbo::Format fmt;
	fmt.enableDepthBuffer(false);

	// use a single channel (red) for the displacement map
	fmt.setColorTextureFormat(gl::Texture2d::Format().wrap(GL_CLAMP_TO_EDGE).internalFormat(GL_R32F));
	mDispMapFbo = gl::Fbo::create(512, 512, fmt);
	fmt.setColorTextureFormat(gl::Texture2d::Format().internalFormat(GL_RG32F));
	mRippleFboA = gl::Fbo::create(512, 512, fmt);
	mRippleFboB = gl::Fbo::create(512, 512, fmt);

	// use 3 channels (rgb) for the normal map
	fmt.setColorTextureFormat(gl::Texture2d::Format().wrap(GL_CLAMP_TO_EDGE).internalFormat(GL_RGB32F));
	mNormalMapFbo = gl::Fbo::create(512, 512, fmt);
	//fmt.setColorTextureFormat(gl::Texture2d::Format().internalFormat(GL_RG32F));
	//mRippleFbo = gl::Fbo::create(512, 512, fmt);

	mSunDir = normalize(vec3(0.0f, 2.0f, 0.0f));

	mPerlin.setSeed(clock());

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

void depthrippleApp::mouseDown(MouseEvent event)
{
}

void depthrippleApp::update()
{
	rs2::frameset fs = realsense.wait_for_frames();
	rs2::frame depth = fs.get_depth_frame();
	rs2::frame filtered = depth;
	rs2::decimation_filter  dec_filter;
	dec_filter.set_option(RS2_OPTION_FILTER_MAGNITUDE, scale);
	filtered = dec_filter.process(filtered);
	rs2::threshold_filter thr_filter(0.15f, far);
	filtered = thr_filter.process(filtered);
	temp_filter.set_option(rs2_option::RS2_OPTION_FILTER_SMOOTH_DELTA, 100.0f);
	temp_filter.set_option(rs2_option::RS2_OPTION_FILTER_SMOOTH_ALPHA, alpha);
	filtered = temp_filter.process(filtered);
	//filtered = disparity_to_depth.process(filtered);
	auto colorized_depth = color_map.colorize(filtered);
	int width = colorized_depth.get_width();
	int height = colorized_depth.get_height();

	Surface sf = Surface((uint8_t *)colorized_depth.get_data(), width, height, 3 * width, SurfaceChannelOrder::RGB);
	mDepthTexture = gl::Texture::create(sf);

	int i = 0;
	//float d = sf.getPixel(ivec2(40, 24)).r;
	//console() << d << endl;
	for (int y = 6; y < 48; y += 6) {
		for (int x = 1; x < 80; x += 2) {
			float d = sf.getPixel(ivec2(x, y)).r;
			
			if (abs(d - prevDepthArray[i]) > 10) {
				depthArray[i] = 1;
				dropRipple(vec2(float(x) / 80.0, float(y) / 48.0));
			}

			prevDepthArray[i] = d;
			//console() << depthArray[i] << "  ";
			i++;
		}
		//console() << endl;;
	}

	// update

	mAmplitude += 0.02f * (mAmplitudeTarget - mAmplitude);

	updateRipple();

	renderDisplacementMap();

	renderNormalMap();

	float t = getElapsedSeconds();
	if (mWalk) {
		mWalker = mPerlin.dfBm(t * 0.1, t * 0.1) * 0.5f + 0.5f;
		dropRipple(mWalker);
	}
}

void depthrippleApp::draw()
{
	gl::clear();// Color(1.0, 0.95, 0.95));
	//gl::enableDepthRead();
	//gl::enableDepthWrite();
	// if enabled, show the displacement and normal maps
	if (false) {
		gl::color(Color(0.05f, 0.05f, 0.05f));
		gl::draw(mDispMapFbo->getColorTexture(), vec2(0));
		gl::color(Color(1, 1, 1));
		gl::draw(mNormalMapFbo->getColorTexture(), vec2(256, 0));
	}

	// setup the 3D camera
	gl::pushMatrices();
	gl::setMatrices(mCamera);

	// setup render states
	//gl::enableAdditiveBlending();
	//gl::enableAlphaBlending();
	if (mDispMapFbo && mNormalMapFbo && mMeshShader) {
		// bind the displacement and normal maps, each to their own texture unit
		gl::ScopedTextureBind tex0(mDispMapFbo->getColorTexture(), uint8_t(0));
		gl::ScopedTextureBind tex1(mNormalMapFbo->getColorTexture(), uint8_t(1));
		gl::ScopedTextureBind tex2(mCubeMap, uint8_t(2));
		gl::ScopedTextureBind tex3(mWaterBottom, uint8_t(3));

		// render our mesh using vertex displacement
		gl::ScopedGlslProg shader(mMeshShader);
		mMeshShader->uniform("uTexDisplacement", 0);
		mMeshShader->uniform("uTexNormal", 1);
		mMeshShader->uniform("uSkyBox", 2);
		mMeshShader->uniform("uWaterBottom", 3);
		mMeshShader->uniform("uLightDir", mSunDir);
		mMeshShader->uniform("uEyePos", mCamera.getEyePoint());
		mMeshShader->uniform("uAmplitude", mAmplitude);

		gl::color(Color::white());
		mBatch->draw();
	}

	// clean up after ourselves
	//gl::disableWireframe();
	//gl::disableAlphaBlending();

	gl::popMatrices();

	if (mDrawTex) {
		gl::color(Color(1, 1, 1));
		//gl::draw(mRippleFbo->getColorTexture(), vec2(0));
		gl::draw(mRippleFboA->getColorTexture(), vec2(0));
		//gl::draw(mRippleFboB->getColorTexture(), vec2(512, 0));
		gl::draw(mDepthTexture, vec2(50, 0));
	}

	mParams->draw();
	//gl::draw(mWaterBottom);
}

void depthrippleApp::dropRipple(vec2 pos)
{
	// bind frame buffer
	gl::ScopedFramebuffer fbo(mRippleFboB);

	// setup viewport and matrices
	gl::ScopedViewport viewport(0, 0, mRippleFboB->getWidth(), mRippleFboB->getHeight());

	gl::pushMatrices();
	gl::setMatricesWindow(mRippleFboB->getSize());

	gl::ScopedTextureBind tex0(mRippleFboA->getColorTexture());

	// render the displacement map
	gl::ScopedGlslProg shader(mDropShader);
	mDropShader->uniform("uTex0", 0);
	mDropShader->uniform("uCenter", pos);
	mDropShader->uniform("uRadius", mRadius);
	mDropShader->uniform("uStrength", mStrength);

	gl::drawSolidRect(mRippleFboB->getBounds());

	// clean up after ourselves
	gl::popMatrices();

	std::swap(mRippleFboA, mRippleFboB);
}

void depthrippleApp::updateRipple()
{
	gl::ScopedFramebuffer fbo(mRippleFboB);

	// setup viewport and matrices
	gl::ScopedViewport viewport(0, 0, mRippleFboB->getWidth(), mRippleFboB->getHeight());

	gl::pushMatrices();
	gl::setMatricesWindow(mRippleFboB->getSize());

	gl::ScopedTextureBind tex0(mRippleFboA->getColorTexture());

	// render the displacement map
	gl::ScopedGlslProg shader(mRippleShader);
	mRippleShader->uniform("uTex0", 0);
	mRippleShader->uniform("uDelta", vec2(1.0f / float(mRippleFboB->getWidth()), 1.0f / float(mRippleFboB->getHeight())));

	gl::drawSolidRect(mRippleFboB->getBounds());

	// clean up after ourselves
	gl::popMatrices();

	std::swap(mRippleFboA, mRippleFboB);
}

void depthrippleApp::renderDisplacementMap()
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

		gl::ScopedTextureBind tex0(mRippleFboB->getColorTexture());
		// render the displacement map
		gl::ScopedGlslProg shader(mDispMapShader);
		mDispMapShader->uniform("uTex0", 0);
		mDispMapShader->uniform("uTime", float(getElapsedSeconds()));
		mDispMapShader->uniform("uAmplitude", mAmplitude);

		gl::drawSolidRect(mDispMapFbo->getBounds());

		// clean up after ourselves
		gl::popMatrices();
	}

	//std::swap(mRippleFboA, mRippleFboB);
}

void depthrippleApp::renderNormalMap()
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

void depthrippleApp::prepare(Settings *settings)
{
	settings->setTitle("Planar 3D Ripple");
	settings->setWindowSize(768, 768);
	settings->disableFrameRate();
}

void depthrippleApp::createMesh()
{
	// create vertex, normal and texcoord buffers
	const int  RES_X = 200;
	const int  RES_Z = 200;
	const vec3 size = vec3(200.0f, 1.0f, 200.0f);

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

void depthrippleApp::compileShaders()
{
	try {
		// this shader will render all colors using a change in hue
		//mBackgroundShader = gl::GlslProg::create(loadAsset("background.vert"), loadAsset("background.frag"));
		// this shader will render a displacement map to a floating point texture, updated every frame
		mDispMapShader = gl::GlslProg::create(loadAsset("displacement_map.vert"), loadAsset("displacement_map.frag"));
		// this shader will create a normal map based on the displacement map
		mNormalMapShader = gl::GlslProg::create(loadAsset("normal_map.vert"), loadAsset("normal_map.frag"));
		// this shader will use the displacement and normal maps to displace vertices of a mesh
		//mMeshShader = gl::GlslProg::create(loadAsset("mesh.vert"), loadAsset("mesh.frag"));
		mMeshShader = gl::GlslProg::create(loadAsset("shader.vert"), loadAsset("shader.frag"));

		mDropShader = gl::GlslProg::create(loadAsset("drop.vert"), loadAsset("drop.frag"));

		mRippleShader = gl::GlslProg::create(loadAsset("ripple.vert"), loadAsset("ripple.frag"));

		createMesh();
	}
	catch (const std::exception &e) {
		console() << e.what() << std::endl;
	}
}

void depthrippleApp::keyDown(KeyEvent event)
{
	switch (event.getCode()) {
	case KeyEvent::KEY_s:
		// reload shaders
		compileShaders();
		break;
	case KeyEvent::KEY_f:
		getWindow()->setFullScreen();
		break;
	case KeyEvent::KEY_d:
		vec2 mousePos = getWindow()->getMousePos();
		dropRipple(vec2(mousePos.x / float(getWindow()->getWidth()), mousePos.y / float(getWindow()->getHeight())));
		break;
	}
}

CINDER_APP( depthrippleApp, RendererGl )
