#include "Camera.h"

Camera::Camera()
{

}

Camera::Camera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up,
	FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth) 
{
	_eye = position;
	_at = at;
	_up = up;

	_windowWidth = windowWidth;
	_windowHeight = windowHeight;

	_nearDepth = nearDepth;
	_farDepth = farDepth;

	Reshape(windowWidth, windowHeight, nearDepth, farDepth);
}

Camera::~Camera() 
{
	
}

void Camera::Update()
{
	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(_eye.x, _eye.y, _eye.z, 0.0f);
	XMVECTOR At = XMVectorSet(_at.x, _at.y, _at.z, 0.0f);
	XMVECTOR Up = XMVectorSet(_up.x, _up.y, _at.z, 0.0f);

	//XMVECTOR eyeVector = XMLoadFloat4(&Eye);
	//XMVECTOR atVector = XMLoadFloat4(&At);
	//XMVECTOR upVector = XMLoadFloat4(&Up);

	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(Eye, At, Up));

	// Initialize the projection matrix
	//XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / _windowHeight, _nearDepth, _farDepth));
}

XMFLOAT4X4 Camera::getViewProjectionMatrix()
{
	XMMATRIX view = XMLoadFloat4x4(&_view);
	XMMATRIX projection = XMLoadFloat4x4(&_projection);

	XMFLOAT4X4 viewProjection;

	XMStoreFloat4x4(&viewProjection, view * projection);

	return viewProjection;
}

void Camera::Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
{
	_windowWidth = windowWidth;
	_windowHeight = windowHeight;
	_nearDepth = nearDepth;
	_farDepth = farDepth;

	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / (FLOAT)_windowHeight, 0.01f, 100.0f));
}