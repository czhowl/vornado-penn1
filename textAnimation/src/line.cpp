#include "line.h"
#include <math.h>
#define _USE_MATH_DEFINES
using namespace ci;
using namespace ci::app;
using namespace std;

void TypeLine::setup(Font f, int line) {
	mLine = line;
	mTextureFont = gl::TextureFont::create(f);

	for (int i = 0; i < LENGTH; i++) {
		mBounds.push_back(Rectf(i * SIZE, mLine * (SIZE + 1), (i + 1) * SIZE, SIZE / 2 + mLine * (SIZE + 1)));
		mBounds.push_back(Rectf(i * SIZE, SIZE / 2 + mLine * (SIZE + 1), (i + 1) * SIZE, SIZE + mLine * (SIZE + 1)));
	}

	for (int i = 0; i < LENGTH; i++) {
		mStatus[i] = 2;
		mPrevStatus[i] = 2;
	}
	while (mString.length() < LENGTH) {
		mString += " ";
		mPrevString += " ";
	}
	mCharDelay = 2;
	mInterval = 4;
	mIsAnimate = false;

	mLineDelay = 5;

	mCurrFrame = 0;
}

void TypeLine::animate(string s) {
	if (!mIsAnimate) {
		if (s.length() > LENGTH) s = s.substr(0, LENGTH);
		else while (s.length() < LENGTH) s += " ";
		mString = s;
		for (int i = 0; i < LENGTH; i++) {
			mPrevStatus[i] = mStatus[i];
			mStatus[i] = 0;
		}
		mIsAnimate = true;
		//mAnimationStart = getElapsedFrames();
		mCurrFrame = 0;
	}
}

void TypeLine::update() {
	if (mIsAnimate && (mCurrFrame % mInterval == 0) && mCurrFrame / mInterval > mLineDelay * mLine) { // animating && hit the frame && hit that line
		mIsAnimate = false;
		for (int i = 0; i < LENGTH; i++) {
			//if (mStatus[i] != 2 && f - mAnimationStart - mLineDelay * mLine >= (LENGTH - (f + i) % LENGTH - 1) * mCharDelay) {
			int pos = LENGTH / 2 + cos(i * M_PI) * ((i + 1) / 2); //spawn from middle
			//int pos = i; //regular pattern left to right;
			console() << pos << endl;
			if (mStatus[pos] != 2 && mCurrFrame / mInterval - mLineDelay * mLine >= i / 5 * mCharDelay) { // not finish animation && hit the frame && hit that line
				mStatus[pos]++;
				mPrevStatus[pos]--;
				//i = 20;
				mIsAnimate = true;
			}
		}
	}
	console() << "===========================" << endl;
	if (!mIsAnimate) {
		mPrevString = mString;
	}
	mCurrFrame++;
}

void TypeLine::draw() {
	for (int i = 0; i < LENGTH; i++) {
		if (mStatus[i] == 1) {
			mTextureFont->drawString(string(1, mString[i]), mBounds[i * 2], vec2(-OFFSET, 0));
		}
		else if (mStatus[i] == 2) {
			mTextureFont->drawString(string(1, mString[i]), mBounds[i * 2], vec2(-OFFSET, 0));
			mTextureFont->drawString(string(1, mString[i]), mBounds[i * 2 + 1], vec2(-OFFSET, -SIZE / 2));
		}

		if (mPrevStatus[i] == 1) {
			mTextureFont->drawString(string(1, mPrevString[i]), mBounds[i * 2 + 1], vec2(-OFFSET, -SIZE / 2));
		}
		else if (mPrevStatus[i] == 2) {
			mTextureFont->drawString(string(1, mPrevString[i]), mBounds[i * 2], vec2(-OFFSET, 0));
			mTextureFont->drawString(string(1, mPrevString[i]), mBounds[i * 2 + 1], vec2(-OFFSET, -SIZE / 2));
		}
	}
}