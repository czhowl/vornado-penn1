#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace std;

#define SIZE 57
#define LENGTH 33
#define OFFSET 4

class TypeLine {
public:
	void setup(ci::Font f, int line);
	void update();
	void draw();

	void animate(string s);

	void setLineDelay(int l) { mLineDelay = l; };
	void setInterval(int i) { mInterval = i; };
	void setCharDelay(int d) { mCharDelay = d; };

	string				mPrevString = " ";
	string				mString = " ";

	int					mAnimationStart;

	int					mCharDelay;
	int					mLineDelay;
	int					mInterval;
	bool				mIsAnimate;

	uint8_t				mStatus[LENGTH];
	uint8_t				mPrevStatus[LENGTH];


private:
	ci::gl::TextureFontRef	mTextureFont;

	
	vector<ci::Rectf>		mBounds;

	int					mLine;
};