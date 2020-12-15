#pragma once

#include <d3d11_1.h>
#include <directxmath.h>

using namespace DirectX;

class Camera
{
private:
	// Store the camera position and view volume
	XMFLOAT3 _eye;
	XMFLOAT3 _at;
	XMFLOAT3 _up;

	FLOAT _windowWidth;
	FLOAT _windowHeight;
	FLOAT _nearDepth;
	FLOAT _farDepth;

	// Hold the view and projection matrices 
	// to pass to the shader
	XMFLOAT4X4 _view;
	XMFLOAT4X4 _projection;

public:
	Camera();

	Camera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up,
		FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);

	~Camera();

	// Update function to make the current view and projection matrices
	void Update();

	// Set and return position, lookat and up attributes
	void setEye(XMFLOAT3 eye) { _eye = eye; }
	XMFLOAT3 getEye() const { return _eye; }

	void setAt(XMFLOAT3 at) { _at = at; }
	XMFLOAT3 getAt() const { return _at; }

	void setUp(XMFLOAT3 up) { _up = up; }
	XMFLOAT3 getUp() const { return _up; }

	// Get the view, projection and combined ViewProjection matrices
	XMFLOAT4X4 getViewMatrix() { return _view; }

	XMFLOAT4X4 getProjectionMatrix() { return _projection; }

	XMFLOAT4X4 getViewProjectionMatrix();

	// Reshape the camera volume if the window is resized
	void Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);
};

