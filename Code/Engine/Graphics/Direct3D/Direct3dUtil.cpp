#include "Direct3dUtil.h"
#include "../../Asserts/Asserts.h"
#include "../../Logging/Logging.h"
#include <DXGI.h>

namespace {
	// This is the main window handle from Windows
	HWND s_renderingWindow;
	// These are D3D interfaces
	ID3D11Device* s_direct3dDevice;
	ID3D11DeviceContext* s_direct3dImmediateContext;
	IDXGISwapChain* s_swapChain;
}

bool eae6320::Graphics::Direct3dUtil::InitializeState(const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight) {
	IDXGIAdapter* const useDefaultAdapter = NULL;
	const D3D_DRIVER_TYPE useHardwareRendering = D3D_DRIVER_TYPE_HARDWARE;
	const HMODULE dontUseSoftwareRendering = NULL;
	unsigned int flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
	{
#ifdef EAE6320_GRAPHICS_ISDEVICEDEBUGINFOENABLED
		flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	}
	D3D_FEATURE_LEVEL* const useDefaultFeatureLevels = NULL;
	const unsigned int requestedFeatureLevelCount = 0;
	const unsigned int sdkVersion = D3D11_SDK_VERSION;
	DXGI_SWAP_CHAIN_DESC swapChainDescription = { 0 };
	{
		{
			DXGI_MODE_DESC& bufferDescription = swapChainDescription.BufferDesc;

			bufferDescription.Width = i_resolutionWidth;
			bufferDescription.Height = i_resolutionHeight;
			{
				DXGI_RATIONAL& refreshRate = bufferDescription.RefreshRate;

				refreshRate.Numerator = 0;	// Refresh as fast as possible
				refreshRate.Denominator = 1;
			}
			bufferDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			bufferDescription.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			bufferDescription.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		}
		{
			DXGI_SAMPLE_DESC& multiSamplingDescription = swapChainDescription.SampleDesc;

			multiSamplingDescription.Count = 1;
			multiSamplingDescription.Quality = 0;	// Anti-aliasing is disabled
		}
		swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDescription.BufferCount = 1;
		swapChainDescription.OutputWindow = s_renderingWindow;
		swapChainDescription.Windowed = TRUE;
		swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDescription.Flags = 0;
	}
	D3D_FEATURE_LEVEL highestSupportedFeatureLevel;
	const HRESULT result = D3D11CreateDeviceAndSwapChain(useDefaultAdapter, useHardwareRendering, dontUseSoftwareRendering,
		flags, useDefaultFeatureLevels, requestedFeatureLevelCount, sdkVersion, &swapChainDescription,
		&s_swapChain, &s_direct3dDevice, &highestSupportedFeatureLevel, &s_direct3dImmediateContext);
	if (SUCCEEDED(result))
	{
		return true;
	}
	else
	{
		EAE6320_ASSERT(false);
		eae6320::Logging::OutputError("Direct3D failed to create a Direct3D11 device with HRESULT %#010x", result);
		return false;
	}
	return true;
}

ID3D11DeviceContext* eae6320::Graphics::Direct3dUtil::getDirect3dContext() {
	return s_direct3dImmediateContext;
}


void eae6320::Graphics::Direct3dUtil::setDirect3dContext(ID3D11DeviceContext* i_direct3dContext) {
	s_direct3dImmediateContext = i_direct3dContext;
}

void eae6320::Graphics::Direct3dUtil::setDirect3dDevice(ID3D11Device* i_direct3dDevice) {
	s_direct3dDevice = i_direct3dDevice;
}

ID3D11Device* eae6320::Graphics::Direct3dUtil::getDirect3dDevice() {
	return s_direct3dDevice;
}

IDXGISwapChain* eae6320::Graphics::Direct3dUtil::getDXGISwapChain() {
	return s_swapChain;
}

void eae6320::Graphics::Direct3dUtil::setDXGISwapChain(IDXGISwapChain* i_swapChain) {
	s_swapChain = i_swapChain;
}

HWND eae6320::Graphics::Direct3dUtil::getRenderWindow() {
	return s_renderingWindow;
}

void eae6320::Graphics::Direct3dUtil::setRenderWindow(HWND i_hwnd) {
	s_renderingWindow = i_hwnd;
}
