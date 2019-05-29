#pragma once

#include "cinder/app/App.h"
#include "cinder/audio/audio.h"
#include "cinder/gl/gl.h"

#include "cinder/Rand.h"

typedef std::shared_ptr<class Wave> WaveRef;

class Wave {
public:
	static WaveRef create(float offset) {
		auto wave = std::make_shared<Wave>();
		wave->setup(offset);
		return wave;
	}

	Wave();

	void setup(float offset);
	void update(std::vector<float> depthArray);
	void draw();
	void					compileShaders();
	void					createMesh();
	void					renderDisplacementMap();
	void					renderNormalMap();
	void					renderRippleMap(std::vector<float> depthArray);
	float					mAmplitudeTarget;

	ci::gl::FboRef				mDispMapFbo;
	ci::gl::FboRef				mRippleMapFbo;
	ci::gl::FboRef				mNormalMapFbo;
	float						mRippleAmplitude;
private:
	ci::gl::GlslProgRef			mMeshShader;
	ci::gl::VboMeshRef			mVboMesh;
	ci::gl::BatchRef			mBatch;

	
	ci::gl::GlslProgRef			mDispMapShader;
	ci::gl::GlslProgRef			mRippleMapShader;
	
	ci::gl::GlslProgRef			mNormalMapShader;

	float					mAmplitude;
	float					mOffset;
	float					mTimeOffset;
	glm::vec2				mPrevMouse;
	float					mDumping;
};