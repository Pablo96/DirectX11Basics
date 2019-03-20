#include "main.hpp"


#define GLFW_STATIC
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <d3d11.h>


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
	IDXGISwapChain* swapChain;
	ID3D11Device* device;
	ID3D11DeviceContext* context;

	D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&swapChain,
		&device,
		nullptr,
		&context
	);


	ID3D11Resource* backBuffer = nullptr;
	swapChain->GetBuffer(0, __uuidof(ID3D11Resource), (void**)&backBuffer);
	
	ID3D11RenderTargetView* targetView;
	device->CreateRenderTargetView(backBuffer, nullptr, &targetView);
	backBuffer->Release();
	
	//Like clear color in opengl
	float color[] = { 0, 0.5f, 1, 1 };
	context->ClearRenderTargetView(targetView, color);

	while (!glfwWindowShouldClose(window))
	{

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