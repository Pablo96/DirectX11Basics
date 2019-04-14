#include "main.hpp"


#define GLFW_STATIC
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

struct Vertex    //Overloaded Vertex Structure
{
	Vertex() {}
	Vertex(float x, float y, float z,
		float nx, float ny, float nz)
		: pos(x, y, z), normal(nx, ny, nz) {}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
};

struct Light
{
	Light()
	{
		ZeroMemory(this, sizeof(Light));
	}
	DirectX::XMFLOAT3 dir;
	float pad;
	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
};

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);

const char* getFeatureLevel(D3D_FEATURE_LEVEL* in_featureLevel)
{
	switch (*in_featureLevel)
	{
	case D3D_FEATURE_LEVEL_11_1:
		return "D3D_FEATURE_LEVEL_11_1";
	case D3D_FEATURE_LEVEL_11_0:
		return "D3D_FEATURE_LEVEL_11_0";
	case D3D_FEATURE_LEVEL_10_1:
		return "D3D_FEATURE_LEVEL_10_1";
	case D3D_FEATURE_LEVEL_10_0:
		return "D3D_FEATURE_LEVEL_10_0";
	default:
		return "UNKNOWN";
	}
}

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

	HRESULT hr;
	IDXGISwapChain* SwapChain;
	ID3D11Device* d3d11Device;
	ID3D11DeviceContext* d3d11DevCon;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11DepthStencilView* depthStencilView;
	ID3D11Texture2D* depthStencilBuffer;


	float red = 0.0f;
	float green = 0.0f;
	float blue = 0.0f;
	int colormodr = 1;
	int colormodg = 1;
	int colormodb = 1;

	//Describe our Buffer
	DXGI_MODE_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

	bufferDesc.Width = WIDTH;
	bufferDesc.Height = HEIGHT;
	bufferDesc.RefreshRate.Numerator = 60;
	bufferDesc.RefreshRate.Denominator = 1;
	bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	//Describe our SwapChain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	swapChainDesc.BufferDesc = bufferDesc;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = win32_window;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	D3D_FEATURE_LEVEL features[] = {
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
	};

	D3D_FEATURE_LEVEL features_level;

	//Create our SwapChain
	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, features, 4,
		D3D11_SDK_VERSION, &swapChainDesc, &SwapChain, &d3d11Device, &features_level, &d3d11DevCon);

	printf("Feature Level: %s", getFeatureLevel(&features_level));

	//Create our BackBuffer
	ID3D11Texture2D* BackBuffer;
	hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer);

	//Create our Render Target
	hr = d3d11Device->CreateRenderTargetView(BackBuffer, NULL, &renderTargetView);
	BackBuffer->Release();


	//Describe our Depth/Stencil Buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = WIDTH;
	depthStencilDesc.Height = HEIGHT;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	d3d11Device->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);
	d3d11Device->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);
	depthStencilBuffer->Release();

	//Set our Render Target with Depth target
	d3d11DevCon->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	// VIEWPORT
	//Create the Viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = WIDTH;
	viewport.Height = HEIGHT;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	//Set the Viewport
	d3d11DevCon->RSSetViewports(1, &viewport);

	// -------------------------------------------------------------------------------------------------
	ID3D11Buffer* squareIndexBuffer;
	ID3D11Buffer* squareVertBuffer;
	ID3D11VertexShader* VS;
	ID3D11PixelShader* PS;
	ID3D11PixelShader* PS2;
	ID3D10Blob* VS_Buffer;
	ID3D10Blob* PS_Buffer;
	ID3D11InputLayout* vertLayout;

	// SHADERS
	//Compile Shaders from shader file
	hr = D3DCompileFromFile(L"VertexShader.hlsl", 0, 0, "main", "vs_5_0", 0, 0, &VS_Buffer, 0);
	hr = D3DCompileFromFile(L"FragmentShader.hlsl", 0, 0, "PSMain", "ps_5_0", 0, 0, &PS_Buffer, 0);

	//Create the Shader Objects
	hr = d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);
	hr = d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);
	hr = D3DCompileFromFile(L"FragmentShader2.hlsl", 0, 0, "PSMain", "ps_5_0", 0, 0, &PS_Buffer, 0);
	hr = d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS2);
	VS_Buffer->Release();
	PS_Buffer->Release();

	//Set Vertex and Pixel Shaders
	d3d11DevCon->VSSetShader(VS, 0, 0);

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);

	//Create the Input Layout
	hr = d3d11Device->CreateInputLayout(layout, numElements, VS_Buffer->GetBufferPointer(),
		VS_Buffer->GetBufferSize(), &vertLayout);

	//Set the Input Layout
	d3d11DevCon->IASetInputLayout(vertLayout);


	// GEOMETRY
	//Vertices
	Vertex v[] =
	{
		// Front Face
		Vertex(-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f),
		Vertex(-1.0f, +1.0f, -1.0f, -1.0f,  1.0f, -1.0f),
		Vertex(+1.0f, +1.0f, -1.0f, 1.0f,  1.0f, -1.0f),
		Vertex(+1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f),

		// Back Face
		Vertex(-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f),
		Vertex(+1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f),
		Vertex(+1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f),
		Vertex(-1.0f,  1.0f, 1.0f, -1.0f,  1.0f, 1.0f),

		// Top Face
		Vertex(-1.0f, 1.0f, -1.0f,-1.0f, 1.0f, -1.0f),
		Vertex(-1.0f, 1.0f,  1.0f,-1.0f, 1.0f,  1.0f),
		Vertex(+1.0f, 1.0f,  1.0f, 1.0f, 1.0f,  1.0f),
		Vertex(+1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f),

		// Bottom Face
		Vertex(-1.0f, -1.0f, -1.0f,-1.0f, -1.0f, -1.0f),
		Vertex(+1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f),
		Vertex(+1.0f, -1.0f,  1.0f, 1.0f, -1.0f,  1.0f),
		Vertex(-1.0f, -1.0f,  1.0f,-1.0f, -1.0f,  1.0f),

		// Left Face
		Vertex(-1.0f, -1.0f,  1.0f,-1.0f, -1.0f,  1.0f),
		Vertex(-1.0f,  1.0f,  1.0f,-1.0f,  1.0f,  1.0f),
		Vertex(-1.0f,  1.0f, -1.0f,-1.0f,  1.0f, -1.0f),
		Vertex(-1.0f, -1.0f, -1.0f,-1.0f, -1.0f, -1.0f),

		// Right Face
		Vertex(1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f),
		Vertex(1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f),
		Vertex(1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f),
		Vertex(1.0f, -1.0f,  1.0f, 1.0f, -1.0f,  1.0f),
	};
	// Indices
	DWORD indices[] = {
		// Front Face
		0,  1,  2,
		0,  2,  3,

		// Back Face
		4,  6,  5,
		4,  7,  6,

		// Top Face
		8,  9, 10,
		8, 10, 11,

		// Bottom Face
		12, 13, 14,
		12, 14, 15,

		// Left Face
		16, 17, 18,
		16, 18, 19,

		// Right Face
		20, 21, 22,
		20, 22, 23
	};

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * 24;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = v;
	hr = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &squareVertBuffer);

	//Set the vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	d3d11DevCon->IASetVertexBuffers(0, 1, &squareVertBuffer, &stride, &offset);

	// Indices
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * 12 * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;
	d3d11Device->CreateBuffer(&indexBufferDesc, &iinitData, &squareIndexBuffer);

	d3d11DevCon->IASetIndexBuffer(squareIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	

	//Set Primitive Topology
	d3d11DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	
	float bgColor[] = { 0, 0, 0.3f, 1 };

	using namespace DirectX;

	XMMATRIX WVP;
	XMMATRIX World;
	XMMATRIX camView;
	XMMATRIX camProjection;

	XMVECTOR camPosition;
	XMVECTOR camTarget;
	XMVECTOR camUp;
	camPosition = XMVectorSet(0.0f, 3.0f, -8.0f, 0.0f);
	camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	
	camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);
	camProjection = XMMatrixPerspectiveFovLH(0.4f*3.14f, (float)WIDTH / HEIGHT, 0.01f, 1000.0f);
	World = XMMatrixIdentity();
	WVP = World * camView * camProjection;

	struct cbPerObject
	{
		XMMATRIX  WVP;
	} cbPerObj;

	D3D11_BUFFER_DESC cbbd;
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(cbPerObject);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;
	
	ID3D11Buffer* cbPerObjectBuffer;
	hr = d3d11Device->CreateBuffer(&cbbd, NULL, &cbPerObjectBuffer);

	cbPerObj.WVP = XMMatrixTranspose(WVP);
	d3d11DevCon->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	d3d11DevCon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

	Light light;
	light.dir = XMFLOAT3(0.25f, 0.5f, -1.0f);
	light.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	light.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	struct cbPerFrame
	{
		Light  light;
	} constbuffPerFrame;

	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(cbPerFrame);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;

	ID3D11Buffer* cbPerFrameBuffer;
	hr = d3d11Device->CreateBuffer(&cbbd, NULL, &cbPerFrameBuffer);

	constbuffPerFrame.light = light;
	d3d11DevCon->UpdateSubresource(cbPerFrameBuffer, 0, NULL, &constbuffPerFrame, 0, 0);
	d3d11DevCon->PSSetShader(PS, 0, 0);
	d3d11DevCon->PSSetConstantBuffers(0, 1, &cbPerFrameBuffer);
	d3d11DevCon->PSSetShader(PS2, 0, 0);
	d3d11DevCon->PSSetConstantBuffers(0, 1, &cbPerFrameBuffer);

	XMMATRIX cube1World;
	XMMATRIX cube2World;

	XMMATRIX Rotation;
	XMMATRIX Scale;
	XMMATRIX Translation;
	float rot = 0.01f;

	while (!glfwWindowShouldClose(window))
	{
		{
			//Keep the cubes rotating
			rot += .0005f;
			if (rot > 6.28f)
				rot = 0.0f;

			//Define cube1's world space matrix
			XMVECTOR rotaxis = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			Rotation = XMMatrixRotationAxis(rotaxis, rot);
			Translation = XMMatrixTranslation(0.0f, 0.0f, 4.0f);

			//Set cube1's world space using the transformations
			cube1World = Translation * Rotation;

			
			//Define cube2's world space matrix
			Rotation = XMMatrixRotationAxis(rotaxis, -rot);
			Scale = XMMatrixScaling(1.3f, 1.3f, 1.3f);

			//Set cube2's world space matrix
			cube2World = Rotation * Scale;
		}

		//Clear our backbuffer to the updated color
		d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);
		// Clear stencil and depth targets
		d3d11DevCon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


		d3d11DevCon->PSSetShader(PS, 0, 0);
		///////////////**************new**************////////////////////
		//Set the WVP matrix and send it to the constant buffer in effect file
		WVP = cube1World * camView * camProjection;
		cbPerObj.WVP = XMMatrixTranspose(WVP);
		d3d11DevCon->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
		d3d11DevCon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

		//Draw 1st cube the Square 36 indices
		d3d11DevCon->DrawIndexed(36, 0, 0);

		d3d11DevCon->PSSetShader(PS2, 0, 0);
		WVP = cube2World * camView * camProjection;
		cbPerObj.WVP = XMMatrixTranspose(WVP);
		d3d11DevCon->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
		d3d11DevCon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

		//Draw the second cube
		d3d11DevCon->DrawIndexed(36, 0, 0);

		//Present the backbuffer to the screen
		SwapChain->Present(0, 0);

		// get the events
		glfwPollEvents();
		processInput(window);
	}

	//Release the COM Objects we created
	SwapChain->Release();
	d3d11Device->Release();
	d3d11DevCon->Release();
	renderTargetView->Release();
	squareVertBuffer->Release();
	squareIndexBuffer->Release();
	VS->Release();
	PS->Release();
	
	vertLayout->Release();
	depthStencilView->Release();
	cbPerObjectBuffer->Release();
	cbPerFrameBuffer->Release();
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