#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "cinder/params/Params.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class planerippleApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
	static void prepare(Settings *settings);
	void keyDown(KeyEvent event) override;

  private:
	void						createMesh();
	void						compileShaders();
	void						renderDisplacementMap();
	void						renderNormalMap();

	params::InterfaceGlRef		mParams;

	gl::VboMeshRef				mVboMesh;
	gl::GlslProgRef				mMeshShader;
	gl::BatchRef				mBatch;

	gl::FboRef					mDispMapFbo;
	gl::GlslProgRef				mDispMapShader;

	gl::FboRef					mNormalMapFbo;
	gl::GlslProgRef				mNormalMapShader;

	//gl::Texture2dRef			mSkyTexture;
	gl::TextureCubeMapRef		mCubeMap;
	gl::Texture2dRef			mWaterBottom;

	CameraPersp					mCamera;
	CameraUi					mCameraUi;

	float						mAmplitude;
	float						mAmplitudeTarget;

	vec3						mSunDir;
};

void planerippleApp::setup()
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

	mCameraUi = CameraUi(&mCamera, getWindow(), -1);
	mCamera.lookAt(vec3(0.0f, 250.0f, 0.1f), vec3(0.0f, 0.0f, 0.0f));

	mAmplitude = 0.0f;
	mAmplitudeTarget = 2.0f;

	mParams = params::InterfaceGl::create("Params", ivec2(220, 220));
	mParams->addParam("amplitude", &mAmplitudeTarget).max(100.0).min(0.0).step(0.01f);

	compileShaders();

	createMesh();

	gl::Fbo::Format fmt;
	fmt.enableDepthBuffer(false);

	// use a single channel (red) for the displacement map
	fmt.setColorTextureFormat(gl::Texture2d::Format().wrap(GL_CLAMP_TO_EDGE).internalFormat(GL_R32F));
	mDispMapFbo = gl::Fbo::create(512, 512, fmt);

	// use 3 channels (rgb) for the normal map
	fmt.setColorTextureFormat(gl::Texture2d::Format().wrap(GL_CLAMP_TO_EDGE).internalFormat(GL_RGB32F));
	mNormalMapFbo = gl::Fbo::create(512, 512, fmt);

	mSunDir = normalize(vec3(0.0f, 2.0f, 0.0f));
}

void planerippleApp::mouseDown( MouseEvent event )
{
}

void planerippleApp::update()
{
	mAmplitude += 0.02f * (mAmplitudeTarget - mAmplitude);

	renderDisplacementMap();

	renderNormalMap();
}

void planerippleApp::draw()
{
	gl::clear();

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

		gl::color(Color::white());
		mBatch->draw();
	}

	// clean up after ourselves
	gl::disableWireframe();
	//gl::disableAlphaBlending();

	gl::popMatrices();

	mParams->draw();
	//gl::draw(mWaterBottom);
}

void planerippleApp::renderDisplacementMap()
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

		gl::drawSolidRect(mDispMapFbo->getBounds());

		// clean up after ourselves
		gl::popMatrices();
	}
}

void planerippleApp::renderNormalMap()
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

void planerippleApp::prepare(Settings *settings)
{
	settings->setTitle("Planar 3D Ripple");
	settings->setWindowSize(768, 768);
	settings->disableFrameRate();
}

void planerippleApp::createMesh()
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

void planerippleApp::compileShaders()
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
		createMesh();
	}
	catch (const std::exception &e) {
		console() << e.what() << std::endl;
	}
}

void planerippleApp::keyDown(KeyEvent event)
{
	switch (event.getCode()) {
	case KeyEvent::KEY_s:
		// reload shaders
		compileShaders();
		break;
	case KeyEvent::KEY_f:
		getWindow()->setFullScreen();
		break;
	}
}


CINDER_APP( planerippleApp, RendererGl(RendererGl::Options().msaa(16)), &planerippleApp::prepare)
