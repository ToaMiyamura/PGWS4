#include<Windows.h>
#include<tchar.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include <vector>
#include<d3dcompiler.h>
#include<DirectXTex.h>
#include<d3dx12.h>
#ifdef _DEBUG
#include<iostream>
#endif

#pragma comment(lib,"DirectXTex.lib")
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

// �A���C�����g�ɂ��낦���T�C�Y��Ԃ�
// @param size ���̃T�C�Y
// @param alignment �A���C�����g�T�C�Y
// @return �A���C�����g�����낦���T�C�Y
size_t AlignmentedSize(size_t size, size_t alignment)
{
	return size + alignment - size % alignment;
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
	ID3D12GraphicsCommandList* _cmdList = nullptr;
	ID3D12CommandQueue* _cmdQueue = nullptr;
	IDXGISwapChain4* _swapchain = nullptr;

	// �E�B���h�E�N���X�̐������o�^
	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure; // �R�[���o�b�N�֐��̎w��
	w.lpszClassName = _T("DX12Sample"); // �A�v���P�[�V�����N���X���i�K���ł悢�j
	w.hInstance = GetModuleHandle(nullptr); // �n���h���̎擾

	RegisterClassEx(&w); // �A�v���P�[�V�����N���X�i�E�B���h�E�N���X�̎w��� OS �ɓ`����j

	RECT wrc = { 0, 0, window_width, window_height }; // �E�B���h�E�T�C�Y�����߂�

	// �֐����g���ăE�B���h�E�̃T�C�Y��␳����
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
	// �E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow(w.lpszClassName, // �N���X���w��
		_T("DX12 �e�X�g "), // �^�C�g���o�[�̕���
		WS_OVERLAPPEDWINDOW, // �^�C�g���o�[�Ƌ��E��������E�B���h�E
		CW_USEDEFAULT, // �\�� x ���W�� OS �ɂ��C��
		CW_USEDEFAULT, // �\�� y ���W�� OS �ɂ��C��
		wrc.right - wrc.left, // �E�B���h�E��
		wrc.bottom - wrc.top, // �E�B���h�E��
		nullptr, // �e�E�B���h�E�n���h��
		nullptr, // ���j���[�n���h��
		w.hInstance, // �Ăяo���A�v���P�[�V�����n���h��
		nullptr); // �ǉ��p�����[�^�[

#ifdef _DEBUG
	//�f�o�b�O���C���[���I����
	EnableDebugLayer();
#endif

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
#endif

	// �A�_�v�^�[�̗񋓗p
	std::vector <IDXGIAdapter*> adapters;
	// �����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
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
		adpt->GetDesc(&adesc); // �A�_�v�^�[�̐����I�u�W�F�N�g�擾
		std::wstring strDesc = adesc.Description;
		// �T�������A�_�v�^�[�̖��O���m�F
		if (strDesc.find(L"NVIDIA") != std::string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
		else if (strDesc.find(L"Radeon") != std::string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}

	// Direct3D �f�o�C�X�̏�����
	D3D_FEATURE_LEVEL featureLevel;
	for (auto lv : levels)
	{
		if (D3D12CreateDevice(tmpAdapter, lv, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			featureLevel = lv;
			break; // �����\�ȃo�[�W���������������烋�[�v��ł��؂�
		}
	}

	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&_cmdAllocator));
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		_cmdAllocator, nullptr,
		IID_PPV_ARGS(&_cmdList));

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	// �^�C���A�E�g�Ȃ�
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	// �A�_�v�^�[�� 1 �����g��Ȃ��Ƃ��� 0 �ł悢
	cmdQueueDesc.NodeMask = 0;
	// �v���C�I���e�B�͓��Ɏw��Ȃ�
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	// �R�}���h���X�g�ƍ��킹��
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	// �L���[����
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = window_width;
	swapchainDesc.Height = window_height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	// �o�b�N�o�b�t�@�[�͐L�яk�݉\
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	// �t���b�v��͑��₩�ɔj��
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	// ���Ɏw��Ȃ�
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	// �E�B���h�E�̃t���X�N���[���؂�ւ��\
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue, hwnd,
		&swapchainDesc, nullptr, nullptr,
		(IDXGISwapChain1**)&_swapchain); // �{����QueryInterface �Ȃǂ�p����
	// IDXGISwapChain4* �ւ̕ϊ��`�F�b�N�����邪�A
	// �����ł͂킩��₷���d���̂��߃L���X�g�őΉ�

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // �����_�[�^�[�Q�b�g�r���[�Ȃ̂� RTV
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2; // �\���� 2 ��
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // ���Ɏw��Ȃ�

	ID3D12DescriptorHeap* rtvHeaps = nullptr;
	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = _swapchain->GetDesc(&swcDesc);

	// SRGB �����_�[�^�[�Q�b�g�r���[�ݒ�
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // �K���}�␳����isRGB�j
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	std::vector<ID3D12Resource*> _backBuffers(swcDesc.BufferCount);
	for (UINT idx = 0; idx < swcDesc.BufferCount; ++idx)
	{
		result = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));
		D3D12_CPU_DESCRIPTOR_HANDLE handle
			= rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_dev->CreateRenderTargetView(_backBuffers[idx], &rtvDesc, handle);
	}

	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;
	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	// �E�B���h�E�\��
	ShowWindow(hwnd, SW_SHOW);

	// ���_�f�[�^�\����
	struct Vertex {
		XMFLOAT3 pos; // xyz���W
		XMFLOAT2 uv; // uv���W
	};

	Vertex vertices[] = {
		{{-1.0f,-1.0f,0.0f},{0.0f,1.0f}},//����
		{{-1.0f,+1.0f,0.0f},{0.0f,0.0f}},//����
		{{+1.0f,-1.0f,0.0f},{1.0f,1.0f}},//�E��
		{{+1.0f,+1.0f,0.0f},{1.0f,0.0f}},//�E��
	};

	D3D12_HEAP_PROPERTIES heapprop = {};
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resdesc = {};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices);// ���_��񂪓��邾���̃T�C�Y
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* vertBuff = nullptr;

	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	Vertex* vertMap = nullptr; // �^��Vertex �ɕύX
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertBuff->Unmap(0, nullptr);

	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress(); // �o�b�t�@�[�̉��z�A�h���X
	vbView.SizeInBytes = sizeof(vertices); // �S�o�C�g��
	vbView.StrideInBytes = sizeof(vertices[0]); // 1 ���_������̃o�C�g��


	unsigned short indices[] = {
		0, 1, 2,
		2, 1, 3
	};

	ID3D12Resource* idxBuff = nullptr;
	// �ݒ�́A�o�b�t�@�[�̃T�C�Y�ȊO�A���_�o�b�t�@�[�̐ݒ���g���񂵂Ă悢
	resdesc.Width = sizeof(indices);
	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&idxBuff));

	// ������o�b�t�@�[�ɃC���f�b�N�X�f�[�^���R�s�[
	unsigned short* mappedIdx = nullptr;
	idxBuff->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(std::begin(indices), std::end(indices), mappedIdx);
	idxBuff->Unmap(0, nullptr);

	// �C���f�b�N�X�o�b�t�@�[�r���[���쐬
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(indices);

	ID3DBlob* _vsBlob = nullptr;
	ID3DBlob* _psBlob = nullptr;

	ID3DBlob* errorBlob = nullptr;
	result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl", // �V�F�[�_�[��
		nullptr, // define �͂Ȃ�
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // �C���N���[�h�̓f�t�H���g
		"BasicVS", "vs_5_0", // �֐���BasicVS�A�ΏۃV�F�[�_�[��vs_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // �f�o�b�O�p����эœK���Ȃ�
		0,
		&_vsBlob, &errorBlob); // �G���[����errorBlob �Ƀ��b�Z�[�W������
	if (FAILED(result)) {
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
		exit(1);//�s�V�������ȁc
	}

	result = D3DCompileFromFile(
		L"BasicPixelShader.hlsl", // �V�F�[�_�[��
		nullptr, // define �͂Ȃ�
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // �C���N���[�h�̓f�t�H���g
		"BasicPS", "ps_5_0", // �֐���BasicPS�A�ΏۃV�F�[�_�[��ps_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // �f�o�b�O�p����эœK���Ȃ�
		0,
		&_psBlob, &errorBlob); // �G���[����errorBlob �Ƀ��b�Z�[�W������
	if (FAILED(result)) {
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
		exit(1);//�s�V�������ȁc
	}

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
		{ // uv�i�ǉ��j
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
			0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};

	gpipeline.pRootSignature = nullptr;// ���ƂŐݒ肷��

	gpipeline.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = _vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = _psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = _psBlob->GetBufferSize();

	// �f�t�H���g�̃T���v���}�X�N��\���萔�i0xffffffff�j
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// �܂��A���`�G�C���A�X�͎g��Ȃ�����false
	gpipeline.RasterizerState.MultisampleEnable = false;

	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // �J�����O���Ȃ�
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; // ���g��h��Ԃ�
	gpipeline.RasterizerState.DepthClipEnable = true; // �[�x�����̃N���b�s���O�͗L����

	gpipeline.BlendState.AlphaToCoverageEnable = false;
	gpipeline.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	//�ЂƂ܂����Z���Z�⃿�u�����f�B���O�͎g�p���Ȃ�
	renderTargetBlendDesc.BlendEnable = false;
	//�ЂƂ܂��_�����Z�͎g�p���Ȃ�
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;

	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	gpipeline.InputLayout.pInputElementDescs = inputLayout; // ���C�A�E�g�擪�A�h���X
	gpipeline.InputLayout.NumElements = _countof(inputLayout); // ���C�A�E�g�z��̗v�f��

	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED; // �X�g���b�v���̃J�b�g�Ȃ�
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;// �O�p�`�ō\��

	gpipeline.NumRenderTargets = 1;//���͂P�̂�
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//0�`1�ɐ��K�����ꂽRGBA

	gpipeline.SampleDesc.Count = 1;//�T���v�����O��1�s�N�Z���ɂ��P
	gpipeline.SampleDesc.Quality = 0;//�N�I���e�B�͍Œ�

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


	D3D12_DESCRIPTOR_RANGE descTblRange[2] = {};//�e�N�X�`���ƒ萔��2��

	//�e�N�X�`���p���W�X�^�[0��
	descTblRange[0].NumDescriptors = 1; // �e�N�X�`��1 ��
	descTblRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // ��ʂ̓e�N�X�`��
	descTblRange[0].BaseShaderRegister = 0; // 0 �ԃX���b�g����
	descTblRange[0].OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//�萔�p���W�X�^�[0��
	descTblRange[1].NumDescriptors = 1; // �萔1 ��
	descTblRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; // ��ʂ͒萔
	descTblRange[1].BaseShaderRegister = 0; // 0 �ԃX���b�g����
	descTblRange[1].OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootparam = {};
	rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	// �z��擪�A�h���X
	rootparam.DescriptorTable.pDescriptorRanges = descTblRange;
	// �f�B�X�N���v�^�����W��
	rootparam.DescriptorTable.NumDescriptorRanges = 2;
	// ���ׂẴV�F�[�_�[���猩����
	rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootSignatureDesc.pParameters = &rootparam; // ���[�g�p�����[�^�[�̐擪�A�h���X
	rootSignatureDesc.NumParameters = 1; // ���[�g�p�����[�^�[��

	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // �������̌J��Ԃ�
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // �c�����̌J��Ԃ�
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // ���s���̌J��Ԃ�
	samplerDesc.BorderColor =
		D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK; // �{�[�_�[�͍�
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // ���`���
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX; // �~�b�v�}�b�v�ő�l
	samplerDesc.MinLOD = 0.0f; // �~�b�v�}�b�v�ŏ��l
	samplerDesc.ShaderVisibility =
		D3D12_SHADER_VISIBILITY_PIXEL; // �s�N�Z���V�F�[�_�[���猩����
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // ���T���v�����O���Ȃ�

	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;

	ID3DBlob* rootSigBlob = nullptr;
	result = D3D12SerializeRootSignature(
		&rootSignatureDesc, // ���[�g�V�O�l�`���ݒ�
		D3D_ROOT_SIGNATURE_VERSION_1_0, // ���[�g�V�O�l�`���o�[�W����
		&rootSigBlob, // �V�F�[�_�[��������Ƃ��Ɠ���
		&errorBlob); // �G���[����������

	ID3D12RootSignature* rootsignature = nullptr;

	result = _dev->CreateRootSignature(
		0, // nodemask�B0 �ł悢
		rootSigBlob->GetBufferPointer(), // �V�F�[�_�[�̂Ƃ��Ɠ��l
		rootSigBlob->GetBufferSize(), // �V�F�[�_�[�̂Ƃ��Ɠ��l
		IID_PPV_ARGS(&rootsignature));
	rootSigBlob->Release(); // �s�v�ɂȂ����̂ŉ��

	gpipeline.pRootSignature = rootsignature;

	ID3D12PipelineState* _pipelinestate = nullptr;
	result = _dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&_pipelinestate));

	D3D12_VIEWPORT viewport = {};
	viewport.Width = window_width; // �o�͐�̕��i�s�N�Z�����j
	viewport.Height = window_height; // �o�͐�̍����i�s�N�Z�����j
	viewport.TopLeftX = 0; // �o�͐�̍�����WX
	viewport.TopLeftY = 0; // �o�͐�̍�����WY
	viewport.MaxDepth = 1.0f; // �[�x�ő�l
	viewport.MinDepth = 0.0f; // �[�x�ŏ��l

	D3D12_RECT scissorrect = {};
	scissorrect.top = 0; // �؂蔲������W
	scissorrect.left = 0; // �؂蔲�������W
	scissorrect.right = scissorrect.left + window_width; // �؂蔲���E���W
	scissorrect.bottom = scissorrect.top + window_height; // �؂蔲�������W

	//WIC�e�N�X�`���̃��[�h
	TexMetadata metadata = {};
	ScratchImage scratchImg = {};

	result = LoadFromWICFile(
		L"img/textest.png", WIC_FLAGS_NONE,
		&metadata, scratchImg);

	auto img = scratchImg.GetImage(0, 0, 0);//���f�[�^���o

	// ���ԃo�b�t�@�[�Ƃ��ẴA�b�v���[�h�q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES uploadHeapProp = {};

	// �}�b�v�\�ɂ��邽�߁AUPLOAD �ɂ���
	uploadHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

	// �A�b�v���[�h�p�Ɏg�p���邱�ƑO��Ȃ̂�UNKNOWN �ł悢
	uploadHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	uploadHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	uploadHeapProp.CreationNodeMask = 0; // �P��A�_�v�^�[�̂���0
	uploadHeapProp.VisibleNodeMask = 0; // �P��A�_�v�^�[�̂���0

	D3D12_RESOURCE_DESC resDesc = {};

	resDesc.Format = DXGI_FORMAT_UNKNOWN; // �P�Ȃ�f�[�^�̉�Ȃ̂�UNKNOWN
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // �P�Ȃ�o�b�t�@�[�Ƃ��Ďw��

	resDesc.Width =
		AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)
		* img->height;

	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;

	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // �A�������f�[�^
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE; // ���Ƀt���O�Ȃ�

	resDesc.SampleDesc.Count = 1; // �ʏ�e�N�X�`���Ȃ̂ŃA���`�G�C���A�V���O���Ȃ�
	resDesc.SampleDesc.Quality = 0;

	// ���ԃo�b�t�@�[�쐬
	ID3D12Resource* uploadbuff = nullptr;

	result = _dev->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE, // ���Ɏw��Ȃ�
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadbuff)
	);

	// �e�N�X�`���̂��߂̃q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES texHeapProp = {};

	texHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT; // �e�N�X�`���p
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	texHeapProp.CreationNodeMask = 0; // �P��A�_�v�^�[�̂���0
	texHeapProp.VisibleNodeMask = 0; // �P��A�_�v�^�[�̂���0

	// ���\�[�X�ݒ�i�ϐ��͎g���܂킵�j
	resDesc.Format = metadata.format;
	resDesc.Width = static_cast<UINT>(metadata.width); // ��
	resDesc.Height = static_cast<UINT>(metadata.height); // ����
	resDesc.DepthOrArraySize = static_cast<uint16_t>(metadata.arraySize);
	resDesc.MipLevels = static_cast<uint16_t>(metadata.mipLevels);
	resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // ���C�A�E�g�͌��肵�Ȃ�

	ID3D12Resource* texbuff = nullptr;
	result = _dev->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE, // ���Ɏw��Ȃ�
		&resDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, // �R�s�[��
		nullptr,
		IID_PPV_ARGS(&texbuff));

	uint8_t* mapforImg = nullptr; // image->pixels �Ɠ����^�ɂ���
	result = uploadbuff->Map(0, nullptr, (void**)&mapforImg); // �}�b�v

	auto srcAddress = img->pixels;

	auto rowPitch = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

	for (int y = 0; y < img->height; ++y)
	{
		std::copy_n(srcAddress,
			rowPitch,
			mapforImg); // �R�s�[

		// 1 �s���Ƃ̂��܂����킹��
		srcAddress += img->rowPitch;
		mapforImg += rowPitch;
	}

	uploadbuff->Unmap(0, nullptr); // �A���}�b�v


	D3D12_TEXTURE_COPY_LOCATION src = {};

	// �R�s�[���i�A�b�v���[�h���j�ݒ�
	src.pResource = uploadbuff; // ���ԃo�b�t�@�[
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT; // �t�b�g�v�����g�w��
	src.PlacedFootprint.Offset = 0;
	src.PlacedFootprint.Footprint.Width = static_cast<UINT>(metadata.width);
	src.PlacedFootprint.Footprint.Height = static_cast<UINT>(metadata.height);
	src.PlacedFootprint.Footprint.Depth = static_cast<UINT>(metadata.depth);
	src.PlacedFootprint.Footprint.RowPitch =
		AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	src.PlacedFootprint.Footprint.Format = img->format;

	D3D12_TEXTURE_COPY_LOCATION dst = {};

	// �R�s�[��ݒ�
	dst.pResource = texbuff;
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	_cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = texbuff;
	BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	BarrierDesc.Transition.StateBefore =
		D3D12_RESOURCE_STATE_COPY_DEST;// �������d�v
	BarrierDesc.Transition.StateAfter =
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;// �������d�v

	_cmdList->ResourceBarrier(1, &BarrierDesc);
	_cmdList->Close();

	// �R�}���h���X�g�̎��s
	ID3D12CommandList* cmdlists[] = { _cmdList };
	_cmdQueue->ExecuteCommandLists(1, cmdlists);

	_cmdQueue->Signal(_fence, ++_fenceVal);

	if (_fence->GetCompletedValue() != _fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);
		_fence->SetEventOnCompletion(_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}
	_cmdAllocator->Reset();//�L���[���N���A
	_cmdList->Reset(_cmdAllocator, nullptr);

	//�萔�o�b�t�@�[�쐬
	XMMATRIX worldMat = XMMatrixRotationY(XM_PIDIV4);

	XMFLOAT3 eye(0, 0, -5);
	XMFLOAT3 target(0, 0, 0);
	XMFLOAT3 up(0, 1, 0);

	auto viewMat = XMMatrixLookAtLH(
		XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

	auto projMat = XMMatrixPerspectiveFovLH(
		XM_PIDIV2,//��p90��
		static_cast<float>(window_width)
		/ static_cast<float>(window_height),//�A�X�y�N�g��
		1.0f,//�߂���
		10.0f//������
	);

	ID3D12Resource* constBuff = nullptr;
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(XMMATRIX) + 0xff) & ~0xff);
	_dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff)
	);

	XMMATRIX* mapMatrix;//�}�b�v��������|�C���^�[
	result = constBuff->Map(0, nullptr, (void**)&mapMatrix);
	//*mapMatrix = matrix;//�s��̓��e���R�s�[�i�g��Ȃ��j

	ID3D12DescriptorHeap* basicDescHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	// �V�F�[�_�[���猩����悤��
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	// �}�X�N��0
	descHeapDesc.NodeMask = 0;
	// SRV�P��CBV�P��
	descHeapDesc.NumDescriptors = 2;
	// �V�F�[�_�[���\�[�X�r���[�p
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	// ����
	result = _dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // ��q
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2D �e�N�X�`��
	srvDesc.Texture2D.MipLevels = 1; // �~�b�v�}�b�v�͎g�p���Ȃ��̂�1
									 
	//�f�X�N���v�^�̐擪�n���h�����擾
	auto basicHeapHandle = basicDescHeap->GetCPUDescriptorHandleForHeapStart();

	_dev->CreateShaderResourceView(
		texbuff, // �r���[�Ɗ֘A�t����o�b�t�@�[
		&srvDesc, // ��قǐݒ肵���e�N�X�`���ݒ���
		basicHeapHandle//�擪�̏ꏊ�������n���h��
	);

	basicHeapHandle.ptr +=
		_dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = constBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = static_cast<UINT>(constBuff->GetDesc().Width);

	//�萔�o�b�t�@�[�r���[�̍쐬
	_dev->CreateConstantBufferView(&cbvDesc, basicHeapHandle);

	float angle = 0.0f;

	while (true)
	{
		MSG msg;
		if(PeekMessage(&msg,nullptr,0,0,PM_REMOVE)){
			// �A�v���P�[�V�������I���Ƃ��� message �� WM_QUIT �ɂȂ�
			if (msg.message == WM_QUIT)
			{
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		angle += 0.1f;
		worldMat = XMMatrixRotationY(angle);
		*mapMatrix = worldMat * viewMat * projMat;

		//DirectX����
		//�o�b�N�o�b�t�@�̃C���f�b�N�X�擾
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();

		auto BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
			_backBuffers[bbIdx], D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET); // ���ꂾ���ōς�
		_cmdList->ResourceBarrier(1, &BarrierDesc);// �o���A�w����s

		_cmdList->SetPipelineState(_pipelinestate);

		//�����_�[�^�[�Q�b�g���w��
		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

		//��ʃN���A
		//float clearColor[] = { 1.0f,1.0f,0.0f,1.0f };//���F
		float clearColor[] = { 0.0f,0.0f,0.3f,1.0f };//���F
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

		_cmdList->SetGraphicsRootSignature(rootsignature);
		//_cmdList->SetGraphicsRootSignature(rootsignature);
		_cmdList->SetDescriptorHeaps(1, &basicDescHeap);
		
		_cmdList->SetGraphicsRootDescriptorTable(0,
			basicDescHeap->GetGPUDescriptorHandleForHeapStart());

		_cmdList->RSSetViewports(1, &viewport);
		_cmdList->RSSetScissorRects(1, &scissorrect);

		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		
		_cmdList->IASetVertexBuffers(0, 1,&vbView);//�O�����_�r���[�̐��A�P���X���b�g�ԍ�
		_cmdList->IASetIndexBuffer(&ibView);
		//_cmdList->DrawInstanced(4, 1, 0, 0);//4�����_���A�P���C���X�^���X���A�O�����_�f�[�^�̃I�t�Z�b�g�A�O���C���X�^���X�̃I�t�Z�b�g
		_cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);

		////�O�ゾ������ւ���
		BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
			_backBuffers[bbIdx], D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT);
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

	//�����N���X�͎g��Ȃ��̂œo�^����
	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;
}

