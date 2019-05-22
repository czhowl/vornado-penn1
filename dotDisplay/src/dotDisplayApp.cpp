#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class dotDisplayApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void dotDisplayApp::setup()
{
}

void dotDisplayApp::mouseDown( MouseEvent event )
{
}

void dotDisplayApp::update()
{
}

void dotDisplayApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( dotDisplayApp, RendererGl )
