#include<Windows.h>
#include<tchar.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include <vector>
#include<d3dcompiler.h>
#ifdef _DEBUG
#include<iostream>
#endif

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

using namespace std;
using namespace DirectX;

void DebugOutputFormatString(const char* format, ...) 
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif 
}

//�����Ȃ���΂Ȃ�Ȃ��֐�
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//�E�B���h�E���j�����ꂽ��Ă΂��
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);//OS�ɑ΂��ăA�v�����I��邱�Ƃ�`����
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);//����̏������s��
}

#ifdef _DEBUG
void EnableDebugLayer()
{
	ID3D12Debug* debugLayer = nullptr;
	HRESULT result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
	if (!SUCCEEDED(result)) return;

	debugLayer->EnableDebugLayer();//�f�o�b�O���C���[��L��������
	debugLayer->Release();//�L����������C���^�[�t�F�[�X�����
}
#endif//_DEBUG

#ifdef _DEBUG

int main()
{
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#endif
	const unsigned int window_width = 1280;
	const unsigned int window_height = 720;

	ID3D12Device* _dev = nullptr;
	IDXGIFactory6* _dxgiFactory = nullptr;
	ID3D12CommandAllocator* _cmdAllocator = nullptr;
	ID3D12GraphicsCommandList* _cmdList= nullptr;
	ID3D12CommandQueue* _cmdQueue = nullptr;
	IDXGISwapChain4* _swapchain = nullptr;

	//�E�B���h�E�N���X�̐������o�^
	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;//�R�[���o�b�N�֐��̎w��
	w.lpszClassName = _T("DX12Sample");//�A�v���P�[�V�����N���X��
	w.hInstance = GetModuleHandle(nullptr);//�n���h���̎擾

	RegisterClassEx(&w);//�A�v���P�[�V�����N���X�i�E�B���h�E�N���X�̎w���OS�ɓ`����j

	RECT wrc = { 0,0,window_width,window_height };//�E�B���h�E�T�C�Y�����߂�

	//�֐����g���ăE�B���h�E�T�C�Y��␳����
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
	//�E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow(w.lpszClassName,//�N���X���w��
		_T("DX12 �e�X�g"),//�^�C�g���o�[�̕���
		WS_OVERLAPPEDWINDOW,//�^�C�g���o�[�Ƌ��E��������E�B���h�E
		CW_USEDEFAULT,//�\��x���W��OS�ɂ��C��
		CW_USEDEFAULT,//�\��y���W��OS�ɂ��C��
		wrc.right - wrc.left,//�E�B���h�E��
		wrc.bottom - wrc.top,//�E�B���h�E��
		nullptr,//�e�E�B���h�E�n���h��
		nullptr,//���j���[�n���h��
		w.hInstance,//�Ăяo���A�v���P�[�V�����n���h��
		nullptr);//�ǉ��p�����[�^�[

	HRESULT D3D12CreateDevie(
		IUnknown * pAdapter,//�ЂƂ܂�nullptr��OK
		D3D_FEATURE_LEVEL MinimumFeatureLevel,//�Œ���K�v�ȃt�B�[�`���[���x��
		REFIID riid,//��q
		void** ppDevice//��q
	);

#ifdef _DEBUG
	EnableDebugLayer();//�f�o�b�O�v���C���[���I����
#endif // _DEBUG


	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

#ifdef _DEBUG
	auto result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory));
#else
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
#endif // _DEBUG

	//�A�_�v�^�[�̗񋓗p
	std::vector <IDXGIAdapter*> adapters;
	//�����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	IDXGIAdapter* tmpAdapter = nullptr;
	for (int i = 0;
		_dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		adapters.push_back(tmpAdapter);
	}
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);//�A�_�v�^�[�̐����I�u�W�F�N�g�擾
		std::wstring strDesc = adesc.Description;
		//�T�������A�_�v�^�[�̖��O���m�F
		if (strDesc.find(L"NVIDIA") != std::string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}

	//Direct3D�f�o�C�X�̏�����
	D3D_FEATURE_LEVEL featureLevel;
	for (auto lv : levels) 
	{
		if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			featureLevel = lv;
			break;//�����\�ȃo�[�W���������������烋�[�v�ł��؂�
		}
	}

	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&_cmdAllocator));
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		_cmdAllocator, nullptr,
		IID_PPV_ARGS(&_cmdList));

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	//�^�C���A�E�g�Ȃ�
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	//�A�_�v�^�[��������g��Ȃ��Ƃ���0�ł悢
	cmdQueueDesc.NodeMask = 0;
	//�v���C�I���e�B�͓��Ɏw��Ȃ�
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	//�R�}���h���X�g�ƍ��킹��
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	//�L���[����
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));

	HRESULT CreateSwapChainForHwnd(
		IUnknown * pDevice,//�R�}���h�L���[�I�u�W�F�N�g
		HWND hWnd,//�E�B���h�E�n���h��
		const DXGI_SWAP_CHAIN_DESC1 * pDesc,//�X���b�v�`�F�[���ݒ�
		const DXGI_SWAP_CHAIN_FULLSCREEN_DESC * pFullscreenDesc,//�ЂƂ܂�nullptr
		IDXGIOutput * pRestrictToOutput,//�ЂƂ܂�nullptr
		IDXGISwapChain1 * *ppSwapChain//�X���b�v�`�F�[���I�u�W�F�N�g�擾�p
	);

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = window_width;
	swapchainDesc.Height = window_height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	//�o�b�N�o�b�t�@�[�͐L�яk�݉\
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	//�t���b�v��͑��₩�ɔj��
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	//���Ɏw��Ȃ�
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	//�E�B���h�E�ƃt���X�N���[���؂�ւ��\
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue, hwnd,
		&swapchainDesc, nullptr, nullptr,
		(IDXGISwapChain1**)&_swapchain);
	//�{����QueryInterface����p����
	//IDXGISwapChain4*�ւ̕ϊ��`�F�b�N�����邪�A
	//�����ł͂킩��₷���d���̂��߃L���X�g�őΉ�

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//�����_�[�^�[�Q�b�g�r���[�Ȃ̂�RTV	
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;//�\���̂Q��
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//���Ɏw��Ȃ�

	ID3D12DescriptorHeap* rtvHeaps = nullptr;
	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = _swapchain->GetDesc(&swcDesc);

	std::vector<ID3D12Resource*> _backBuffers(swcDesc.BufferCount);
	for (int idx = 0; idx < swcDesc.BufferCount; ++idx)
	{
		result = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));
		D3D12_CPU_DESCRIPTOR_HANDLE handle
			= rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_dev->CreateRenderTargetView(_backBuffers[idx], nullptr, handle);
	}

	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;
	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	//�E�B���h�E�\��
	ShowWindow(hwnd, SW_SHOW);

	XMFLOAT3 vertices[] = {
		{-1.0f,-1.0f,0.0f},//����
		{-1.0f,+1.0f,0.0f},//����
		{+1.0f,-1.0f,0.0f}//�E��
	};

	D3D12_HEAP_PROPERTIES heapprop = {};
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resbesc = {};
	resbesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resbesc.Width = sizeof(vertices);//���_��񂪓����Ă��邾���̃T�C�Y
	resbesc.Height = 1;
	resbesc.DepthOrArraySize = 1;
	resbesc.MipLevels = 1;
	resbesc.Format = DXGI_FORMAT_UNKNOWN;
	resbesc.SampleDesc.Count = 1;
	resbesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resbesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* vertBuff = nullptr;

	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resbesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	ID3DBlob* _vsBlob = nullptr;
	ID3DBlob* _psBlob = nullptr;

	ID3DBlob* errorBlob = nullptr;
	result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",//�V�F�[�_�[��
		nullptr,//define�͂Ȃ�
		D3D_COMPILE_STANDARD_FILE_INCLUDE,//�C���N���[�h�̓f�t�H���g
		"BasicVS", "vs_5_0",//�֐���BasicVS�A�ΏۃV�F�[�_�[��vs_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,//�f�o�b�O�p�y�эœK���Ȃ�
		0,
		&_vsBlob, &errorBlob);//�G���[����errorBlob�Ƀ��b�Z�[�W������
	
	if (FALSE(result))//���̏ꍇ�̓G���[���b�Z�[�W�������Ă��Ȃ�
	{
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("�t�@�C������������܂���");
			
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);
	}

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
		D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
	};

	XMFLOAT3* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertBuff->Unmap(0, nullptr);

	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();//�o�b�t�@�[�̉��z�A�h���X
	vbView.SizeInBytes = sizeof(vertices);//�S�o�C�g��
	vbView.StrideInBytes = sizeof(vertices[0]);//�P���_������̃o�C�g��

	while (true)
	{
		MSG msg;
		if(PeekMessage(&msg,nullptr,0,0,PM_REMOVE)){}

		//DirectX����
		//�o�b�N�o�b�t�@�̃C���f�b�N�X�擾
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();

		//�����_�[�^�[�Q�b�g���w��
		D3D12_RESOURCE_BARRIER BarrierDesc = {};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;//�J��
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;//���Ɏw��Ȃ�
		BarrierDesc.Transition.pResource = _backBuffers[bbIdx];//�o�b�N�o�b�t�@�[���\�[�X
		BarrierDesc.Transition.Subresource = 0;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;//���O��PRESENT���
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;//������RT���
		_cmdList->ResourceBarrier(1, &BarrierDesc);//�o���A�w����s

		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

		//��ʃN���A
		float clearColor[] = { 1.0f,1.0f,0.0f,1.0f };//���F
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

		//�O�ゾ������ւ���
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		_cmdList->ResourceBarrier(1, &BarrierDesc);

		//���߂̃N���[�Y
		_cmdList->Close();

		//�R�}���h���X�g�̎��s
		ID3D12CommandList* cmdlists[] = { _cmdList };
		_cmdQueue->ExecuteCommandLists(1, cmdlists);

		//�҂�
		_cmdQueue->Signal(_fence, ++_fenceVal);
		if (_fence->GetCompletedValue() != _fenceVal)
		{
			auto event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);//�C�x���g�n���h���擾
			WaitForSingleObject(event, INFINITE);//�C�x���g����������܂Ŗ����҂�
			CloseHandle(event);//�C�x���g�n���h�������
		}

		_cmdAllocator->Reset();//�L���[���N���A
		_cmdList->Reset(_cmdAllocator, nullptr);//�ĂуR�}���h���X�g�����߂鏀��

		//�t���b�v
		_swapchain->Present(1, 0);
	}

	MSG msg = {};
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			//�A�v���P�[�V�������I���Ƃ���message��WM_QUIT�ɂȂ�
			if (msg.message == WM_QUIT)
		    {
			break;
		    }

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
				
	}

	//�����N���X�͎g��Ȃ��̂œo�^����
	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;
}

