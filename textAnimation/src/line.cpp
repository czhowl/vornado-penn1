#include "line.h"

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
		mAnimationStart = getElapsedFrames();
	}
}

void TypeLine::update() {
	int f = getElapsedFrames();
	if (mIsAnimate && (f % mInterval == 0) && (f - mAnimationStart > mLineDelay * mLine)) {
		mIsAnimate = false;
		for (int i = 0; i < LENGTH; i++) {
			if (mStatus[i] != 2 && f - mAnimationStart - mLineDelay * mLine >= i * mCharDelay) {
				mStatus[i]++;
				mPrevStatus[i]--;
				//i = 20;
				mIsAnimate = true;
			}
		}
	}
	if (!mIsAnimate) {
		mPrevString = mString;
	}
}

void TypeLine::draw() {
	for (int i = 0; i < LENGTH; i++) {
		if (mStatus[i] == 1) {
			mTextureFont->drawString(string(1, mString[i]), mBounds[i * 2], vec2(-9, 0));
		}
		else if (mStatus[i] == 2) {
			mTextureFont->drawString(string(1, mString[i]), mBounds[i * 2], vec2(-9, 0));
			mTextureFont->drawString(string(1, mString[i]), mBounds[i * 2 + 1], vec2(-9, -SIZE / 2));
		}

		if (mPrevStatus[i] == 1) {
			mTextureFont->drawString(string(1, mPrevString[i]), mBounds[i * 2 + 1], vec2(-9, -SIZE / 2));
		}
		else if (mPrevStatus[i] == 2) {
			mTextureFont->drawString(string(1, mPrevString[i]), mBounds[i * 2], vec2(-9, 0));
			mTextureFont->drawString(string(1, mPrevString[i]), mBounds[i * 2 + 1], vec2(-9, -SIZE / 2));
		}
	}
}