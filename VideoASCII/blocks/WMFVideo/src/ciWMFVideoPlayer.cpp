#include "ciWMFVideoPlayerUtils.h"
#include "ciWMFVideoPlayer.h"

#include "cinder/app/App.h"
#include "cinder/gl/Texture.h"
#include "cinder/Log.h"

using namespace std;
using namespace ci;
using namespace ci::app;

LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );
// Message handlers

ciWMFVideoPlayer::ScopedVideoTextureBind::ScopedVideoTextureBind( const ciWMFVideoPlayer& video, uint8_t textureUnit )
	: mCtx( gl::context() )
	, mTarget( video.mTex->getTarget() )
	, mTextureUnit( textureUnit )
	, mPlayer( video.mPlayer )
{
	mPlayer->mEVRPresenter->lockSharedTexture();
	mCtx->pushTextureBinding( mTarget, video.mTex->getId(), mTextureUnit );
}

ciWMFVideoPlayer::ScopedVideoTextureBind::~ScopedVideoTextureBind()
{
	mCtx->popTextureBinding( mTarget, mTextureUnit );
	mPlayer->mEVRPresenter->unlockSharedTexture();
}

int  ciWMFVideoPlayer::mInstanceCount = 0;

ciWMFVideoPlayer::ciWMFVideoPlayer()
	: mPlayer( NULL )
	, mVideoFill( VideoFill::FILL )
{
	if( mInstanceCount == 0 )  {
		HRESULT hr = MFStartup( MF_VERSION );

		if( !SUCCEEDED( hr ) ) {
			//ofLog(OF_LOG_ERROR, "ciWMFVideoPlayer: Error while loading MF");
		}
	}

	mId = mInstanceCount;
	mInstanceCount++;
	this->InitInstance();

	mWaitForLoadedToPlay = false;
	mSharedTextureCreated = false;

	// Make sure the video is closed before the rendering context is lost.
	auto window = app::App::get()->getWindow();

	if( window ) {
		mWinCloseConnection = window->getSignalClose().connect( std::bind( &ciWMFVideoPlayer::close, this ) );
	}
}

ciWMFVideoPlayer::~ciWMFVideoPlayer()
{
	mWinCloseConnection.disconnect();

	if( mPlayer ) {
		mPlayer->Shutdown();
		//if (mSharedTextureCreated) mPlayer->mEVRPresenter->releaseSharedTexture();
		SafeRelease( &mPlayer );
	}

	CI_LOG_I( "Player " << mId << " Terminated" );
	mInstanceCount--;

	if( mInstanceCount == 0 ) {
		MFShutdown();
		CI_LOG_I( "Shutting down MF" );
	}
}

void ciWMFVideoPlayer::forceExit()
{
	if( mInstanceCount != 0 ) {
		CI_LOG_I( "Shutting down MF some ciWMFVideoPlayer remains" );
		MFShutdown();
	}
}

bool ciWMFVideoPlayer::loadMovie( const fs::path& filePath, const string& audioDevice )
{
	if( !mPlayer ) {
		//ofLogError("ciWMFVideoPlayer") << "Player not created. Can't open the movie.";
		return false;
	}

	//DWORD fileAttr = GetFileAttributesW( filePath.c_str() );
	//if (fileAttr == INVALID_FILE_ATTRIBUTES)
	//{
	//	CI_LOG_E( "The video file could not be found: '" << filePath );
	//	//ofLog(OF_LOG_ERROR,"ciWMFVideoPlayer:" + s.str());
	//	return false;
	//}

	//CI_LOG_I( "Videoplayer[" << mId << "] loading " << name );

	HRESULT hr = S_OK;
	string s = filePath.string();
	std::wstring w( s.length(), L' ' );
	std::copy( s.begin(), s.end(), w.begin() );

	std::wstring a( audioDevice.length(), L' ' );
	std::copy( audioDevice.begin(), audioDevice.end(), a.begin() );

	hr = mPlayer->OpenURL( w.c_str(), a.c_str() );

	//	CI_LOG_D(GetPlayerStateString(mPlayer->GetState()));

	if( !mSharedTextureCreated ) {
		mWidth = mPlayer->getWidth();
		mHeight = mPlayer->getHeight();

		gl::Texture::Format format;
		format.setInternalFormat( GL_RGBA );
		format.setTargetRect();
		format.loadTopDown( true );
		mTex = gl::Texture::create( mWidth, mHeight, format );
		mPlayer->mEVRPresenter->createSharedTexture( mWidth, mHeight, mTex->getId() );
		mSharedTextureCreated = true;
	}
	else {
		if( ( mWidth != mPlayer->getWidth() ) || ( mHeight != mPlayer->getHeight() ) ) {
			mPlayer->mEVRPresenter->releaseSharedTexture();

			mWidth = mPlayer->getWidth();
			mHeight = mPlayer->getHeight();

			gl::Texture::Format format;
			format.setInternalFormat( GL_RGBA );
			format.setTargetRect();
			format.loadTopDown( true );
			mTex = gl::Texture::create( mWidth, mHeight, format );
			mPlayer->mEVRPresenter->createSharedTexture( mWidth, mHeight, mTex->getId() );
		}
	}

	mWaitForLoadedToPlay = false;
	return true;
}

ci::ImageSourceRef ciWMFVideoPlayer::draw( int x, int y, int w, int h )
{
	ImageSourceRef s;

	if( !mPlayer ) {
		return s;
	}

	bool success = mPlayer->mEVRPresenter->lockSharedTexture();
	
	if( mTex ) {
		

		Rectf destRect = Rectf( float(x), float(y), float(x + w), float(y + h) );

		switch( mVideoFill ) {
			case VideoFill::FILL:
				gl::draw( mTex, destRect );
				break;

			case VideoFill::ASPECT_FIT:
				gl::draw( mTex, Rectf( mTex->getBounds() ).getCenteredFit( destRect, true ) ) ;
				break;

			case VideoFill::CROP_FIT:
				gl::draw( mTex, Area( destRect.getCenteredFit( mTex->getBounds(), true ) ), destRect );
				break;
		}

	}

	success = mPlayer->mEVRPresenter->unlockSharedTexture();

	return s;
}

bool ciWMFVideoPlayer::isPlaying() const
{
	return mPlayer->GetState() == STARTED;
}

bool ciWMFVideoPlayer::isStopped() const
{
	return ( mPlayer->GetState() == STOPPED || mPlayer->GetState() == PAUSED );
}

bool ciWMFVideoPlayer::isPaused() const
{
	return mPlayer->GetState() == PAUSED;
}

void ciWMFVideoPlayer::close()
{
	mPlayer->Shutdown();
}

void ciWMFVideoPlayer::update()
{
	if( !mPlayer ) { return; }

	if( ( mWaitForLoadedToPlay ) && mPlayer->GetState() == PAUSED ) {
		mWaitForLoadedToPlay = false;
		mPlayer->Play();
	}

	return;
}

void ciWMFVideoPlayer::play()
{
	if( !mPlayer ) { return; }

	if( mPlayer->GetState()  == OPEN_PENDING ) { mWaitForLoadedToPlay = true; }

	mPlayer->Play();
}

void ciWMFVideoPlayer::stop()
{
	mPlayer->Stop();
}

void ciWMFVideoPlayer::pause()
{
	mPlayer->Pause();
}

double ciWMFVideoPlayer::getPosition() const
{
	return mPlayer->getPosition();
}

double ciWMFVideoPlayer::getFrameRate() const
{
	return mPlayer->getFrameRate();
}

double ciWMFVideoPlayer::getDuration() const
{
	return mPlayer->getDuration();
}

void ciWMFVideoPlayer::setPosition( double pos )
{
	mPlayer->setPosition( pos );
}

void ciWMFVideoPlayer::stepForward()
{
	if( mPlayer->GetState() == STOPPED ) {
		return;
	}

	mPlayer->Pause();
	double fps = mPlayer->getFrameRate();
	double step = 1.0 / fps;
	double currentVidPos = mPlayer->getPosition();
	double targetVidPos = currentVidPos + step;

	if  (mPlayer->GetState() == PAUSED) {
		play();
	}

	mPlayer->setPosition(mPlayer->getPosition() + step);
	mPlayer->Pause();
}
float ciWMFVideoPlayer::getSpeed()
{
	return mPlayer->GetPlaybackRate();
}

bool ciWMFVideoPlayer::setSpeed( float speed, bool useThinning )
{
	//according to MSDN playback must be stopped to change between forward and reverse playback and vice versa
	//but is only required to pause in order to shift between forward rates
	float curRate = getSpeed();
	HRESULT hr = S_OK;
	bool resume = isPlaying();

	if( curRate >= 0 && speed >= 0 ) {
		if( !isPaused() ) {
			mPlayer->Pause();
		}

		hr = mPlayer->SetPlaybackRate( useThinning, speed );

		if( resume ) {
			mPlayer->Play();
		}
	}
	else {
		//setting to a negative doesn't seem to work though no error is thrown...
		/*float position = getPosition();
		if(isPlaying())
		mPlayer->Stop();
		hr = mPlayer->SetPlaybackRate(useThinning, speed);
		if(resume){
		mPlayer->Play();
		mPlayer->setPosition(position);
		}*/
	}

	if( hr == S_OK ) {
		return true;
	}
	else {
		if( hr == MF_E_REVERSE_UNSUPPORTED ) {
			cout << "The object does not support reverse playback." << endl;
		}

		if( hr == MF_E_THINNING_UNSUPPORTED ) {
			cout << "The object does not support thinning." << endl;
		}

		if( hr == MF_E_UNSUPPORTED_RATE ) {
			cout << "The object does not support the requested playback rate." << endl;
		}

		if( hr == MF_E_UNSUPPORTED_RATE_TRANSITION ) {
			cout << "The object cannot change to the new rate while in the running state." << endl;
		}

		return false;
	}
}

PresentationEndedSignal& ciWMFVideoPlayer::getPresentationEndedSignal()
{
	assert( mPlayer );
	return mPlayer->getPresentationEndedSignal();
}

int ciWMFVideoPlayer::getHeight() const
{
	return mPlayer->getHeight();
}
int ciWMFVideoPlayer::getWidth() const
{
	return mPlayer->getWidth();
}
void ciWMFVideoPlayer::setLoop( bool isLooping )
{
	mIsLooping = isLooping;
	mPlayer->setLooping( isLooping );
}

//-----------------------------------
// Prvate Functions
//-----------------------------------

// Handler for Media Session events.
void ciWMFVideoPlayer::OnPlayerEvent( HWND hwnd, WPARAM pUnkPtr )
{
	HRESULT hr = mPlayer->HandleEvent( pUnkPtr );

	if( FAILED( hr ) ) {
		//ofLogError("ciWMFVideoPlayer", "An error occurred.");
	}
}

LRESULT CALLBACK WndProcDummy( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	ciWMFVideoPlayer *impl = NULL;

	// If the message is WM_NCCREATE we need to hide 'this' in the window long.
	if( message == WM_NCCREATE ) {
		impl = reinterpret_cast<ciWMFVideoPlayer*>( ( (LPCREATESTRUCT)lParam )->lpCreateParams );
		::SetWindowLongPtr( hwnd, GWLP_USERDATA, (__int3264)(LONG_PTR)impl );
	}
	else
		impl = reinterpret_cast<ciWMFVideoPlayer*>( ::GetWindowLongPtr( hwnd, GWLP_USERDATA ) );

	switch( message ) {
		case WM_CREATE: {
			return DefWindowProc( hwnd, message, wParam, lParam );
		}

		default: {
			if( !impl ) {
				return DefWindowProc( hwnd, message, wParam, lParam );
			}

			return impl->WndProc( hwnd, message, wParam, lParam );
		}
	}

	return 0;
}

LRESULT  ciWMFVideoPlayer::WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message ) {
		case WM_DESTROY:
			PostQuitMessage( 0 );
			break;

		case WM_APP_PLAYER_EVENT:
			OnPlayerEvent( hwnd, wParam );
			break;

		default:
			return DefWindowProc( hwnd, message, wParam, lParam );
	}

	return 0;
}

//  Create the application window.
BOOL ciWMFVideoPlayer::InitInstance()
{
	PCWSTR szWindowClass = L"MFBASICPLAYBACK" ;
	HWND hwnd;
	WNDCLASSEX wcex;

	//   g_hInstance = hInst; // Store the instance handle.
	// Register the window class.
	ZeroMemory( &wcex, sizeof( WNDCLASSEX ) );
	wcex.cbSize         = sizeof( WNDCLASSEX );
	wcex.style          = CS_HREDRAW | CS_VREDRAW  ;

	wcex.lpfnWndProc    =  WndProcDummy;
	//  wcex.hInstance      = hInst;
	wcex.hbrBackground  = ( HBRUSH )( BLACK_BRUSH );
	// wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_MFPLAYBACK);
	wcex.lpszClassName  = szWindowClass;

	if( RegisterClassEx( &wcex ) == 0 ) {
		// return FALSE;
	}

	// Create the application window.
	hwnd = CreateWindow( szWindowClass, L"", WS_OVERLAPPEDWINDOW,
	                     CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, NULL, reinterpret_cast<LPVOID>( this ) );

	if( hwnd == 0 ) {
		return FALSE;
	}

	HRESULT hr = CPlayer::CreateInstance( hwnd, hwnd, &mPlayer );

	LONG style2 = ::GetWindowLong( hwnd, GWL_STYLE );
	style2 &= ~WS_DLGFRAME;
	style2 &= ~WS_CAPTION;
	style2 &= ~WS_BORDER;
	style2 &= WS_POPUP;
	LONG exstyle2 = ::GetWindowLong( hwnd, GWL_EXSTYLE );
	exstyle2 &= ~WS_EX_DLGMODALFRAME;
	::SetWindowLong( hwnd, GWL_STYLE, style2 );
	::SetWindowLong( hwnd, GWL_EXSTYLE, exstyle2 );

	mHWNDPlayer = hwnd;
	UpdateWindow( hwnd );

	return TRUE;
}

