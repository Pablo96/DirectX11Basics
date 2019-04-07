#include "main.hpp"


#define GLFW_STATIC
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>



void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);


int main(int in_varc, char** in_varv)
{
	// WINDOW CREATION (GLFW)
	GLFWwindow* window = nullptr;
	HWND win32_window = nullptr;
	{
		if (!glfwInit())
		{
			std::cerr << "ERROR: couldnt init glfw." << std::endl;
			return 1;
		}

		// tell glfw we want to use core profile(no legacy stuff)
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
		glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
		// NO API since we wont use opengl
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window = glfwCreateWindow(WIDTH, HEIGHT, TITLE, NULL, NULL);
		if (!window)
		{
			std::cerr << "ERROR: couldnt create window." << std::endl;
			glfwTerminate();
			return 1;
		}

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		glfwSetCursorPosCallback(window, mouse_callback);

		// Win32 window handler (yeeey glfw did it for us)
		win32_window = glfwGetWin32Window(window);
	}

	//////////////////////////////////////////////////////////////////////////
	//					DIRECTX 11 STUFF
	//////////////////////////////////////////////////////////////////////////
	/*
	 * Here we say the minimum level of hardware our app support.
	 * We will support minimum 10.1 and the Current should be 11.1
	 */
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_11_1
	};

	// I think i will use Direct2D for menues. Not in this example but just as reminder.
	// This flag adds support for surfaces with a color-channel ordering different
	// from the API default. It is required for compatibility with Direct2D.
	UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	// This enable debug info for directX
#if defined(DEBUG) || defined(_DEBUG)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	//////////////////////////////////////////////////////////////////////////
	// CREATE THE DIRECT3D 11 API DEVICE OBJECT AND A CORRESPONDING CONTEXT.
	//////////////////////////////////////////////////////////////////////////
	Microsoft::WRL::ComPtr<ID3D11Device>        device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;


	D3D11CreateDevice(
		nullptr,                    // Specify nullptr to use the default adapter.
		D3D_DRIVER_TYPE_HARDWARE,   // Create a device using the hardware graphics driver.
		0,                          // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
		deviceFlags,                // Set debug and Direct2D compatibility flags.
		levels,                     // List of feature levels this app can support.
		ARRAYSIZE(levels),          // Size of the list above.
		D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
		&device,                    // Returns the Direct3D device created.
		&m_featureLevel,            // Returns feature level of device created.
		&context                    // Returns the device immediate context.
	);

	//////////////////////////////////////////////////////////////////////////
	//				CREATE THE SWAP CHAIN
	//////////////////////////////////////////////////////////////////////////
	
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(DXGI_SWAP_CHAIN_DESC));
	{
		// Buffer filled with 0 size then guess it set to the one we want
		sd.Windowed = TRUE;
		sd.BufferCount = 2;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		sd.SampleDesc.Count = 1;	//multisampling setting
		sd.SampleDesc.Quality = 0;	//vendor-specific flag
		sd.OutputWindow = win32_window;
	}

	// Create the DXGI device object to use in other factories, such as Direct2D.
	Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDevice;
	device.As(&dxgiDevice);

	// Create swap chain.
	Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
	Microsoft::WRL::ComPtr<IDXGIFactory> factory;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;

	HRESULT hr = dxgiDevice->GetAdapter(&adapter);

	if (SUCCEEDED(hr))
	{
		adapter->GetParent(IID_PPV_ARGS(&factory));

		hr = factory->CreateSwapChain(
			device.Get(),
			&sd,
			&swapChain
		);
	}

	//////////////////////////////////////////////////////////////////////////
	//				CREATE A RENDER TARGET FOR DRAWING
	//////////////////////////////////////////////////////////////////////////

	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer = nullptr;
	swapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(void**)&backBuffer);
	
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> targetView;
	device->CreateRenderTargetView(
		backBuffer.Get(),
		nullptr,
		targetView.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<D3D11_TEXTURE2D_DESC> m_bbDesc;
	backBuffer->GetDesc(m_bbDesc.Get());

	// if you don't supply this view no per-pixel depth testing is performed
	CD3D11_TEXTURE2D_DESC depthStencilDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		static_cast<UINT> (m_bbDesc->Width),
		static_cast<UINT> (m_bbDesc->Height),
		1, // This depth stencil view has only one texture.
		1, // Use a single mipmap level.
		D3D11_BIND_DEPTH_STENCIL
	);

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencil;

	device->CreateDepthStencilView(
		depthStencil.Get(),
		&depthStencilViewDesc,
		&depthStencilView
	);


	Microsoft::WRL::ComPtr<D3D11_VIEWPORT> viewport;
	ZeroMemory(viewport.Get(), sizeof(D3D11_VIEWPORT));
	{
		viewport->Height = (float) m_bbDesc->Height;
		viewport->Width = (float)m_bbDesc->Width;
		viewport->MinDepth = 0;
		viewport->MaxDepth = 1;
	}

	context->RSSetViewports(1, viewport.Get());
	

	//------------------------------------------------------------------------


	//Like clear color in opengl
	float color[] = { 0, 0.5f, 1, 1 };
	context->ClearRenderTargetView(targetView.Get(), color);
	swapChain->Present(1, 0);




	// SHADERS
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> vertexBlob;
	{
		using namespace Microsoft::WRL;

		D3DReadFileToBlob(L"bin/debug/VertexShader.cso", &vertexBlob);
		device->CreateVertexShader(
			vertexBlob->GetBufferPointer(), //pointer to the compiled shader.
			vertexBlob->GetBufferSize(), // Size of the compiled vertex shader
			nullptr,
			&vertexShader // Address of a pointer to the vertex shader
		);

	}
	

	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	D3D11_INPUT_ELEMENT_DESC layout[] = {
	{
		"POSITION",
		0,
		DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT,
		0,
        0,
		D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,
		0
	} };

	device->CreateInputLayout(
		layout,
		1,
		vertexBlob->GetBufferPointer(),
		vertexBlob->GetBufferSize(),
		inputLayout.GetAddressOf()
	);

	context->IASetInputLayout(inputLayout.Get());


	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelBlob;
	{
		using namespace Microsoft::WRL;

		D3DReadFileToBlob(L"bin/debug/FragmentShader.cso", &pixelBlob);
		device->CreatePixelShader(
			pixelBlob->GetBufferPointer(), //pointer to the compiled shader.
			pixelBlob->GetBufferSize(), // Size of the compiled vertex shader
			nullptr,
			&pixelShader // Address of a pointer to the vertex shader
		);

	}

	const UINT vertexCount = 6;
	const UINT stride = sizeof(float) * vertexCount;
	const UINT offset = 0;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	
	// Pass triangle to the GPU buffers
	{
		// define the triangle
		const float vertices[vertexCount] =
		{
			 0.0f,  0.5f,
			 0.5f, -0.5f,
			-0.5f, -0.5f,
		};

		using namespace Microsoft::WRL;
		
		ComPtr<ID3D11Buffer> vertexBuffer;
		
		// Describes a buffer resource
		D3D11_BUFFER_DESC bufferDescription;
		ZeroMemory(&bufferDescription, sizeof(D3D11_BUFFER_DESC));

		bufferDescription.Usage = D3D11_USAGE_DEFAULT;
		bufferDescription.ByteWidth = stride;
		bufferDescription.StructureByteStride = vertexCount;
		bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDescription.CPUAccessFlags = 0; // 0 if no CPU access is necessary
		bufferDescription.MiscFlags = 0; // 0 if unused

		// initializing a subresource (in this case the vertex buffer)
		D3D11_SUBRESOURCE_DATA bufferSourceData;
		ZeroMemory(&bufferSourceData, sizeof(D3D11_SUBRESOURCE_DATA));
		bufferSourceData.pSysMem = vertices;

		device->CreateBuffer(&bufferDescription, &bufferSourceData, &vertexBuffer);
		context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	}


	// MAIN LOOP
	while (!glfwWindowShouldClose(window))
	{
		// bind the vertex shader
		context->VSSetShader(vertexShader.Get(), nullptr, 0);
		//Drawing!!
		context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->Draw(6, 0);

		// Like swap buffers in opengl
		swapChain->Present(1, 0);
		// get the events
		glfwPollEvents();
		processInput(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	
}

void processInput(GLFWwindow* window)
{
	/*
	Ask glfw is key was pressed and do stuff acorrding
	*/
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

}