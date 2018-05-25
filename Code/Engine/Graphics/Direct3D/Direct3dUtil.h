#ifndef EAE6320_DIRECT_3D_UTIL_H
#define EAE6320_DIRECT_3D_UTIL_H

#include <D3D11.h>
#include <DXGI.h>

namespace eae6320 {
	namespace Graphics {
		namespace Direct3dUtil {

			bool InitializeState(const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight);

			ID3D11DeviceContext* getDirect3dContext();
			void setDirect3dContext(ID3D11DeviceContext* i_direct3dContext);
			
			void setDirect3dDevice(ID3D11Device* i_direct3dDevice);
			ID3D11Device* getDirect3dDevice();
			
			IDXGISwapChain* getDXGISwapChain();
			void setDXGISwapChain(IDXGISwapChain* i_swapChain);
			
			HWND getRenderWindow();
			void setRenderWindow(HWND i_hwnd);

		} // namespaceDirect3dUtil
	}// namespace Graphics
}// namespace eae6320

#endif // !"EAE6320_DIRECT_3D_UTIL_H"
