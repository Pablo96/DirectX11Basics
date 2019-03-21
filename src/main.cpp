#include "main.hpp"


#define GLFW_STATIC
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>



void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);


int main(int in_varc, char** in_varv)
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

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, TITLE, NULL, NULL);
	if (!window)
	{
		std::cerr << "ERROR: couldnt create window." << std::endl;
		glfwTerminate();
		return 1;
	}

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetCursorPosCallback(window, mouse_callback);

	// Win32 window handler (yeeey glfw did it for us)
	HWND win32_window = glfwGetWin32Window(window);

	// Swap chain descriptor
	DXGI_SWAP_CHAIN_DESC sd = {0};
	// Buffer filled with 0 size then guess it set to the one we want
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	// Basiclly we say use 8 bit per color channel with the "UNORM" thing at the end
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	// Not a clue
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator= 0;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	//  Here we say this is the  buffer target to output da picture
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	//  Use 1 buffer for front and 1 for back (weird)
	sd.BufferCount = 1;
	// Here we say which window get the rendered picture (opengl kinda context)
	sd.OutputWindow = win32_window;
	// Windowed Mode
	sd.Windowed = TRUE;
	// Not a clue
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	const char* win32_adapter = glfwGetWin32Adapter(glfwGetPrimaryMonitor());
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	UINT creationFlags = D3D11_CREATE_DEVICE_DEBUG;

	D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		creationFlags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&swapChain,
		&device,
		nullptr,
		&context
	);


	Microsoft::WRL::ComPtr<ID3D11Resource> backBuffer = nullptr;
	swapChain->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
	
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> targetView;
	device->CreateRenderTargetView(backBuffer.Get(), nullptr, &targetView);

	context->OMSetRenderTargets(1, targetView.GetAddressOf(), NULL);


	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.Height = HEIGHT;
	viewport.Width = WIDTH;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	context->RSSetViewports(1, &viewport);
	
	//Like clear color in opengl
	float color[] = { 0, 0.5f, 1, 1 };
	context->ClearRenderTargetView(targetView.Get(), color);
	swapChain->Present(1, NULL);

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



	while (!glfwWindowShouldClose(window))
	{
		// bind the vertex shader
		context->VSSetShader(vertexShader.Get(), nullptr, 0);
		//Drawing!!
		context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->Draw(3, 0);

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