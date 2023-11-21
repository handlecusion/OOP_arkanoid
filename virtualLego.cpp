////////////////////////////////////////////////////////////////////////////////
//
// File: virtualLego.cpp
//
// Original Author: 박창현 Chang-hyeon Park, 
// Modified by Bong-Soo Sohn and Dong-Jun Kim
// 
// Originally programmed for Virtual LEGO. 
// Modified later to program for Virtual Billiard.
//        
////////////////////////////////////////////////////////////////////////////////

#include "d3dUtility.h"
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cassert>

#define brickCount 52
#define wallCount 3
#define COR_VAL 0.01f

IDirect3DDevice9* Device = NULL;

// initialize the position (coordinate) of each ball (ball0 ~ ball3)
float spherePos[brickCount][2] = {};
// initialize the color of each ball (ball0 ~ ball3)
D3DXCOLOR sphereColor[brickCount] = {};

// window size
const int Width = 1024;
const int Height = 768;

// game start
bool game_start = false;

// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
D3DXMATRIX g_mWorld;
D3DXMATRIX g_mView;
D3DXMATRIX g_mProj;

#define M_RADIUS 0.21   // ball radius
#define PI 3.14159265
#define M_HEIGHT 0.01
#define DECREASE_RATE 0.9982

// -----------------------------------------------------------------------------
// CSphere class definition
// -----------------------------------------------------------------------------

class CSphere {
private :
	float					center_x, center_y, center_z;
    float                   m_radius;
	float					m_velocity_x;
	float					m_velocity_z;
	bool					isControlball = false;

public:
    CSphere(void)
    {
        D3DXMatrixIdentity(&m_mLocal);
        ZeroMemory(&m_mtrl, sizeof(m_mtrl));
        m_radius = 0;
		m_velocity_x = 0;
		m_velocity_z = 0;
        m_pSphereMesh = NULL;
    }
    ~CSphere(void) {}

public:
    bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE)
    {
        if (NULL == pDevice)
            return false;
		
        m_mtrl.Ambient  = color;
        m_mtrl.Diffuse  = color;
        m_mtrl.Specular = color;
        m_mtrl.Emissive = d3d::BLACK;
        m_mtrl.Power    = 5.0f;
		
        if (FAILED(D3DXCreateSphere(pDevice, getRadius(), 50, 50, &m_pSphereMesh, NULL)))
            return false;
        return true;
    }
	
    void destroy(void)
    {
        if (m_pSphereMesh != NULL) {
            m_pSphereMesh->Release();
            m_pSphereMesh = NULL;
        }
    }

    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice)
            return;
        pDevice->SetTransform(D3DTS_WORLD, &mWorld);
        pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
        pDevice->SetMaterial(&m_mtrl);
		m_pSphereMesh->DrawSubset(0);
    }
	
    bool hasIntersected(CSphere& ball) 
	{
		// Insert your code here.
		D3DXVECTOR3 cord = this->getCenter();
		D3DXVECTOR3 ball_cord = ball.getCenter();
		double xDistance = abs((cord.x - ball_cord.x) * (cord.x - ball_cord.x));
		double zDistance = abs((cord.z - ball_cord.z) * (cord.z - ball_cord.z));
		double totalDistance = sqrt(xDistance + zDistance);

		if (totalDistance < (this->getRadius() + ball.getRadius()))
		{
			std::cout << "hasInters" << std::endl;
			return true;
		}

		return false;
	}
	
	void hitBy(CSphere& ball) 
	{ 
		// Insert your code here.
		if (hasIntersected(ball)) {
			//충돌 구현

			//moveball을 튕기기(맞은 속도가 중요한게 아니라, 맞은 방향이 중요하다, 벡터 차만큼 이동하도록 한다)

			float delta_x = ball.getCenter().x - this->getCenter().x;
			float delta_z = ball.getCenter().z - this->getCenter().z;
			float multiple;

			float velocity_vector_scala = sqrt(ball.getVelocity_X() * ball.getVelocity_X() + ball.getVelocity_Z() * ball.getVelocity_Z());
			float distance_vector_scala = sqrt(delta_x * delta_x + delta_z * delta_z); //방향 벡터
			multiple = velocity_vector_scala / distance_vector_scala;

			float new_velocity_x = multiple * delta_x;
			float new_velocity_z = multiple * delta_z;

			ball.setPower(new_velocity_x, new_velocity_z);

			// this가 controlball이면, ball을 사라지게 하지 않기
			if (!this->isControlBall()) {
				this->setCenter(-10.0f, -10.0f, 0.0f);
			}
		}
			
	}

	void ballUpdate(float timeDiff) 
	{
		const float TIME_SCALE = 3.3;
		D3DXVECTOR3 cord = this->getCenter();
		double vx = abs(this->getVelocity_X());
		double vz = abs(this->getVelocity_Z());

		if(vx > 0.01 || vz > 0.01)
		{
			float tX = cord.x + TIME_SCALE*timeDiff*m_velocity_x;
			float tZ = cord.z + TIME_SCALE*timeDiff*m_velocity_z;

			//correction of position of ball
			// Please uncomment this part because this correction of ball position is necessary when a ball collides with a wall
			/*if(tX >= (4.5 - M_RADIUS))
				tX = 4.5 - M_RADIUS;
			else if(tX <=(-4.5 + M_RADIUS))
				tX = -4.5 + M_RADIUS;
			else if(tZ <= (-3 + M_RADIUS))
				tZ = -3 + M_RADIUS;
			else if(tZ >= (3 - M_RADIUS))
				tZ = 3 - M_RADIUS;*/
			
			this->setCenter(tX, cord.y, tZ);
		}
		else { this->setPower(0,0);}
	}

	double getVelocity_X() { return this->m_velocity_x;	}
	double getVelocity_Z() { return this->m_velocity_z; }

	void setPower(double vx, double vz)
	{
		this->m_velocity_x = vx;
		this->m_velocity_z = vz;
	}

	void setCenter(float x, float y, float z)
	{
		D3DXMATRIX m;
		center_x=x;	center_y=y;	center_z=z;
		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}
	
	float getRadius(void)  const { return (float)(M_RADIUS);  }
    const D3DXMATRIX& getLocalTransform(void) const { return m_mLocal; }
    void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
	void setControlBall(bool l_isControlball) { isControlball = l_isControlball; }
	bool isControlBall(void) { return isControlball; }

    D3DXVECTOR3 getCenter(void) const
    {
        D3DXVECTOR3 org(center_x, center_y, center_z);
        return org;
    }
	
private:
    D3DXMATRIX              m_mLocal;
    D3DMATERIAL9            m_mtrl;
    ID3DXMesh*              m_pSphereMesh;
	
};



// -----------------------------------------------------------------------------
// CWall class definition
// -----------------------------------------------------------------------------

class CWall {

private:
	
    float					m_x;
	float					m_z;
	float                   m_width;
    float                   m_depth;
	float					m_height;
	
public:
    CWall(void)
    {
        D3DXMatrixIdentity(&m_mLocal);
        ZeroMemory(&m_mtrl, sizeof(m_mtrl));
        m_width = 0;
        m_depth = 0;
        m_pBoundMesh = NULL;
    }
    ~CWall(void) {}
public:
    bool create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE)
    {
        if (NULL == pDevice)
            return false;
		
        m_mtrl.Ambient  = color;
        m_mtrl.Diffuse  = color;
        m_mtrl.Specular = color;
        m_mtrl.Emissive = d3d::BLACK;
        m_mtrl.Power    = 5.0f;
		
        m_width = iwidth;
        m_depth = idepth;
		
        if (FAILED(D3DXCreateBox(pDevice, iwidth, iheight, idepth, &m_pBoundMesh, NULL)))
            return false;
        return true;
    }
    void destroy(void)
    {
        if (m_pBoundMesh != NULL) {
            m_pBoundMesh->Release();
            m_pBoundMesh = NULL;
        }
    }
    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice)
            return;
        pDevice->SetTransform(D3DTS_WORLD, &mWorld);
        pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
        pDevice->SetMaterial(&m_mtrl);
		m_pBoundMesh->DrawSubset(0);
    }
	
	bool hasIntersected(CSphere& ball) 
	{
		// Insert your code here.
		float cord_x = ball.getCenter().x;
		float cord_z = ball.getCenter().z;

		float hit_boundary_min_x = this->m_x - this->getWidth() / 2 - ball.getRadius();
		float hit_boundary_max_x = this->m_x + this->getWidth() / 2 + ball.getRadius();
		float hit_boundary_min_z = this->m_z - this->getDepth() / 2 - ball.getRadius();
		float hit_boundary_max_z = this->m_z + this->getDepth() / 2 + ball.getRadius();

		if ((hit_boundary_min_x <= cord_x && cord_x <= hit_boundary_max_x) && (hit_boundary_min_z <= cord_z && cord_z <= hit_boundary_max_z)) {
			return true;
		}
		else return false;

		return false;
	}

	void hitBy(CSphere& ball) 
	{
		// Insert your code here.
		if (hasIntersected(ball)) {

			float cord_x = ball.getCenter().x;
			float cord_z = ball.getCenter().z;

			float boundary_min_x = this->m_x - this->getWidth() / 2;
			float boundary_max_x = this->m_x + this->getWidth() / 2;
			float boundary_min_z = this->m_z - this->getDepth() / 2;
			float boundary_max_z = this->m_z + this->getDepth() / 2;

			if ((boundary_min_x <= cord_x && cord_x <= boundary_max_x) && !(boundary_min_z <= cord_z && cord_z <= boundary_max_z)) {
				if (boundary_min_z - ball.getRadius() <= cord_z && cord_z <= this->m_z) {
					cord_z = boundary_min_z - ball.getRadius() - COR_VAL;
				}
				else {
					cord_z = boundary_max_z + ball.getRadius() + COR_VAL;
				}
				ball.setPower(ball.getVelocity_X(), -ball.getVelocity_Z());
			}

			if (!(boundary_min_x <= cord_x && cord_x <= boundary_max_x) && (boundary_min_z <= cord_z && cord_z <= boundary_max_z)) {
				if (boundary_min_x - ball.getRadius() <= cord_x && cord_x <= this->m_x) {
					cord_x = boundary_min_x - ball.getRadius() - COR_VAL;
				}
				else {
					cord_x = boundary_max_x + ball.getRadius() + COR_VAL;
				}
				ball.setPower(-ball.getVelocity_X(), ball.getVelocity_Z());
			}

			if (ball.isControlBall()) {
				ball.setPower(0.0, 0.0);
			}

			ball.setCenter(cord_x, ball.getCenter().y, cord_z);
		}
	}    
	
	void setPosition(float x, float y, float z)
	{
		D3DXMATRIX m;
		this->m_x = x;
		this->m_z = z;

		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}

	D3DXVECTOR3 CWall::getCenter(void) const
	{
		D3DXVECTOR3 org(m_x, 0, m_z);
		return org;
	}
	
    float getHeight(void) const { return M_HEIGHT; }
	float getDepth(void) const { return m_depth; }
	float getWidth(void) const { return m_width; }
	
	
	
private :
    void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
	
	D3DXMATRIX              m_mLocal;
    D3DMATERIAL9            m_mtrl;
    ID3DXMesh*              m_pBoundMesh;
};

// -----------------------------------------------------------------------------
// CLight class definition
// -----------------------------------------------------------------------------

class CLight {
public:
    CLight(void)
    {
        static DWORD i = 0;
        m_index = i++;
        D3DXMatrixIdentity(&m_mLocal);
        ::ZeroMemory(&m_lit, sizeof(m_lit));
        m_pMesh = NULL;
        m_bound._center = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        m_bound._radius = 0.0f;
    }
    ~CLight(void) {}
public:
    bool create(IDirect3DDevice9* pDevice, const D3DLIGHT9& lit, float radius = 0.1f)
    {
        if (NULL == pDevice)
            return false;
        if (FAILED(D3DXCreateSphere(pDevice, radius, 10, 10, &m_pMesh, NULL)))
            return false;
		
        m_bound._center = lit.Position;
        m_bound._radius = radius;
		
        m_lit.Type          = lit.Type;
        m_lit.Diffuse       = lit.Diffuse;
        m_lit.Specular      = lit.Specular;
        m_lit.Ambient       = lit.Ambient;
        m_lit.Position      = lit.Position;
        m_lit.Direction     = lit.Direction;
        m_lit.Range         = lit.Range;
        m_lit.Falloff       = lit.Falloff;
        m_lit.Attenuation0  = lit.Attenuation0;
        m_lit.Attenuation1  = lit.Attenuation1;
        m_lit.Attenuation2  = lit.Attenuation2;
        m_lit.Theta         = lit.Theta;
        m_lit.Phi           = lit.Phi;
        return true;
    }
    void destroy(void)
    {
        if (m_pMesh != NULL) {
            m_pMesh->Release();
            m_pMesh = NULL;
        }
    }
    bool setLight(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice)
            return false;
		
        D3DXVECTOR3 pos(m_bound._center);
        D3DXVec3TransformCoord(&pos, &pos, &m_mLocal);
        D3DXVec3TransformCoord(&pos, &pos, &mWorld);
        m_lit.Position = pos;
		
        pDevice->SetLight(m_index, &m_lit);
        pDevice->LightEnable(m_index, TRUE);
        return true;
    }

    void draw(IDirect3DDevice9* pDevice)
    {
        if (NULL == pDevice)
            return;
        D3DXMATRIX m;
        D3DXMatrixTranslation(&m, m_lit.Position.x, m_lit.Position.y, m_lit.Position.z);
        pDevice->SetTransform(D3DTS_WORLD, &m);
        pDevice->SetMaterial(&d3d::WHITE_MTRL);
        m_pMesh->DrawSubset(0);
    }

    D3DXVECTOR3 getPosition(void) const { return D3DXVECTOR3(m_lit.Position); }

private:
    DWORD               m_index;
    D3DXMATRIX          m_mLocal;
    D3DLIGHT9           m_lit;
    ID3DXMesh*          m_pMesh;
    d3d::BoundingSphere m_bound;
};


// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------
CWall	g_legoPlane;
CWall	g_legowall[wallCount];
CSphere	g_sphere[brickCount];
CSphere g_controlball;
CSphere g_moveball;
CLight	g_light;

double g_camera_pos[3] = {0.0, 5.0, -8.0};

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------


void destroyAllLegoBlock(void)
{
}

// initialization
bool Setup()
{
	int i;

	D3DXMatrixIdentity(&g_mWorld);
	D3DXMatrixIdentity(&g_mView);
	D3DXMatrixIdentity(&g_mProj);

	// create plane and set the position
	if (false == g_legoPlane.create(Device, -1, -1, 9, 0.03f, 6, d3d::GREEN)) return false;
	g_legoPlane.setPosition(0.0f, -0.0006f / 5, 0.0f);

	// create walls and set the position. note that there are four walls
	if (false == g_legowall[0].create(Device, -1, -1, 9, 0.3f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[0].setPosition(0.0f, 0.12f, 3.06f);
	if (false == g_legowall[1].create(Device, -1, -1, 9, 0.3f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[1].setPosition(0.0f, 0.12f, -3.06f);
	/*if (false == g_legowall[2].create(Device, -1, -1, 0.12f, 0.3f, 6.24f, d3d::DARKRED)) return false;
	g_legowall[2].setPosition(4.56f, 0.12f, 0.0f);*/
	if (false == g_legowall[2].create(Device, -1, -1, 0.12f, 0.3f, 6.24f, d3d::DARKRED)) return false;
	g_legowall[2].setPosition(-4.56f, 0.12f, 0.0f);

	// set position and color for the bricks
	// set position
	for (int layer = 0; layer < 4; layer++) {
		for (int nth = 0; nth < 13; nth++) {
			spherePos[layer * 13 + nth][0] = 0.9f + (-0.9f * layer);
			spherePos[layer * 13 + nth][1] = 0.43f * (nth - 6);
		}
	}
	// set color
	for (int i = 0; i < brickCount; i++) {
		sphereColor[i] = d3d::YELLOW;
	}

	// create balls and set the position
	for (i = 0; i < brickCount; i++) {
		if (false == g_sphere[i].create(Device, sphereColor[i])) return false;
		g_sphere[i].setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
		g_sphere[i].setPower(0, 0);
	}

	// create controlball for control direction of moveball
	if (false == g_controlball.create(Device, d3d::WHITE)) return false;
	g_controlball.setCenter(4.5f - M_RADIUS, (float)M_RADIUS, .0f);
	g_controlball.setControlBall(true);

	// create moveball for destroy bricks
	if (false == g_moveball.create(Device, d3d::RED)) return false;
	g_moveball.setCenter(4.5f - 3 * M_RADIUS, (float)M_RADIUS, .0f);

	// light setting 
	D3DLIGHT9 lit;
	::ZeroMemory(&lit, sizeof(lit));
	lit.Type = D3DLIGHT_POINT;
	lit.Diffuse = d3d::WHITE;
	lit.Specular = d3d::WHITE * 0.9f;
	lit.Ambient = d3d::WHITE * 0.9f;
	lit.Position = D3DXVECTOR3(0.0f, 6.0f, 0.0f);
	lit.Range = 100.0f;
	lit.Attenuation0 = 0.0f;
	lit.Attenuation1 = 0.7f;
	lit.Attenuation2 = 0.0f;
	if (false == g_light.create(Device, lit))
		return false;

	// Position and aim the camera.
	D3DXVECTOR3 pos(9.0f, 9.0f, 0.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 2.0f, 0.0f);
	D3DXMatrixLookAtLH(&g_mView, &pos, &target, &up);
	Device->SetTransform(D3DTS_VIEW, &g_mView);

	// Set the projection matrix.
	D3DXMatrixPerspectiveFovLH(&g_mProj, D3DX_PI / 4,
		(float)Width / (float)Height, 1.0f, 100.0f);
	Device->SetTransform(D3DTS_PROJECTION, &g_mProj);

	// Set render states.
	Device->SetRenderState(D3DRS_LIGHTING, TRUE);
	Device->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
	Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

	g_light.setLight(Device, g_mWorld);
	return true;
}

void Cleanup(void)
{
    g_legoPlane.destroy();
	for(int i = 0 ; i < 4; i++) {
		g_legowall[i].destroy();
	}
    destroyAllLegoBlock();
    g_light.destroy();
}


// timeDelta represents the time between the current image frame and the last image frame.
// the distance of moving balls should be "velocity * timeDelta"
bool Display(float timeDelta)
{
	int i = 0;
	int j = 0;


	if (Device)
	{
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0);
		Device->BeginScene();

		// update the position of balls.
		g_moveball.ballUpdate(timeDelta);
		g_controlball.ballUpdate(timeDelta);
		for (i = 0; i < brickCount; i++) {
			g_sphere[i].ballUpdate(timeDelta);
		}

		// update the position of each ball. during update, check whether each ball hit by walls.
		for (i = 0; i < brickCount; i++) {
			for (j = 0; j < wallCount; j++) { g_legowall[j].hitBy(g_moveball); }
		}

		// update the position of moveball. Check whether any two balls hit together and update the direction of moveball.
		for (i = 0; i < brickCount; i++) {
			g_sphere[i].hitBy(g_moveball);
		}

		// update the position of controlball. Check whether legowall hit by controlball.
		for (i = 0; i < wallCount; i++) {
			g_legowall[i].hitBy(g_controlball);
		}

		// update the position of moveball. Check whether controlball hit by moveball.
		g_controlball.hitBy(g_moveball);

		// If game not started, moveball follows controlball
		if (!game_start) g_moveball.setCenter(g_controlball.getCenter().x - 2 * g_controlball.getRadius(), g_controlball.getCenter().y, g_controlball.getCenter().z);

		// If ball out of field, restart game
		if (g_moveball.getCenter().x >= 8.0f) {
			game_start = false;
			g_moveball.setCenter(g_controlball.getCenter().x - 2 * g_controlball.getRadius(), g_controlball.getCenter().y, g_controlball.getCenter().z);
			g_moveball.setPower(0, 0);

			// balls set the position
			for (i = 0; i < brickCount; i++) {
				g_sphere[i].setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
				g_sphere[i].setPower(0, 0);
			}
		}

		// draw plane, walls, and spheres
		g_legoPlane.draw(Device, g_mWorld);
		for (i = 0; i < wallCount; i++) {
			g_legowall[i].draw(Device, g_mWorld);
		}

		for (int i = 0; i < brickCount; i++) {
			g_sphere[i].draw(Device, g_mWorld);
		}
		g_controlball.draw(Device, g_mWorld);
		g_moveball.draw(Device, g_mWorld);
		g_light.draw(Device);

		Device->EndScene();
		Device->Present(0, 0, 0, 0);
		Device->SetTexture(0, NULL);
	}
	return true;
}

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool wire = false;
	static bool isReset = true;
	static int old_x = 0;
	static int old_y = 0;
	static enum { WORLD_MOVE, LIGHT_MOVE, BLOCK_MOVE } move = WORLD_MOVE;

	switch (msg) {
	case WM_DESTROY:
	{
		::PostQuitMessage(0);
		break;
	}
	case WM_KEYDOWN:
	{
		switch (wParam) {
		case VK_ESCAPE:
			::DestroyWindow(hwnd);
			break;
		case VK_RETURN:
			if (NULL != Device) {
				wire = !wire;
				Device->SetRenderState(D3DRS_FILLMODE,
					(wire ? D3DFILL_WIREFRAME : D3DFILL_SOLID));
			}
			break;
		case VK_LEFT:
		{
			float new_z;

			float boundary_max_z = g_legowall[0].getCenter().z - g_legowall[0].getDepth() / 2 - g_controlball.getRadius();
			float boundary_min_z = g_legowall[1].getCenter().z + g_legowall[1].getDepth() / 2 + g_controlball.getRadius();

			D3DXVECTOR3 coord3d = g_controlball.getCenter();

			new_z = coord3d.z + 10 * (-0.01f);

			if (new_z > boundary_max_z) {
				new_z = boundary_max_z;
			}

			if (new_z < boundary_min_z) {
				new_z = boundary_min_z;
			}

			g_controlball.setCenter(coord3d.x, coord3d.y, new_z);

			move = WORLD_MOVE;
			break;
		}
		case VK_RIGHT:
		{
			float new_z;

			float boundary_max_z = g_legowall[0].getCenter().z - g_legowall[0].getDepth() / 2 - g_controlball.getRadius();
			float boundary_min_z = g_legowall[1].getCenter().z + g_legowall[1].getDepth() / 2 + g_controlball.getRadius();

			D3DXVECTOR3 coord3d = g_controlball.getCenter();

			new_z = coord3d.z + 10 * (0.01f);

			if (new_z > boundary_max_z) {
				new_z = boundary_max_z;
			}

			if (new_z < boundary_min_z) {
				new_z = boundary_min_z;
			}

			g_controlball.setCenter(coord3d.x, coord3d.y, new_z);

			move = WORLD_MOVE;
			break;
		}
		case VK_SPACE:
			//D3DXVECTOR3 targetpos = g_controlball.getCenter();
			//D3DXVECTOR3	whitepos = g_moveball.getCenter();
			//double theta = acos(sqrt(pow(targetpos.x - whitepos.x, 2)) / sqrt(pow(targetpos.x - whitepos.x, 2) +
			//	pow(targetpos.z - whitepos.z, 2)));		// 기본 1 사분면
			//if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x >= 0) { theta = -theta; }	//4 사분면
			//if (targetpos.z - whitepos.z >= 0 && targetpos.x - whitepos.x <= 0) { theta = PI - theta; } //2 사분면
			//if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x <= 0) { theta = PI + theta; } // 3 사분면
			//double distance = sqrt(pow(targetpos.x - whitepos.x, 2) + pow(targetpos.z - whitepos.z, 2));
			//g_moveball.setPower(distance * cos(theta), distance * sin(theta));
			//break;
			if (!game_start) {
				game_start = true;

				g_moveball.setPower(-2.5f, 0.0);
			}
			break;
		}
		break;
	}

	case WM_MOUSEMOVE:
	{
		int new_x = LOWORD(lParam);
		int new_y = HIWORD(lParam);
		float dx;
		float dy;
		float new_z;

		float boundary_max_z = g_legowall[0].getCenter().z - g_legowall[0].getDepth() / 2 - g_controlball.getRadius();
		float boundary_min_z = g_legowall[1].getCenter().z + g_legowall[1].getDepth() / 2 + g_controlball.getRadius();

		if (LOWORD(wParam) & MK_RBUTTON) {

			dx = (old_x - new_x);// * 0.01f;
			dy = (old_y - new_y);// * 0.01f;

			D3DXVECTOR3 coord3d = g_controlball.getCenter();

			g_controlball.setCenter(4.5f - M_RADIUS, (float)M_RADIUS, .0f);
			g_controlball.setControlBall(true);

			new_z = coord3d.z + dx * (-0.01f);

			if (new_z > boundary_max_z) {
				new_z = boundary_max_z;
			}

			if (new_z < boundary_min_z) {
				new_z = boundary_min_z;
			}

			g_controlball.setCenter(coord3d.x, coord3d.y, new_z);
			old_x = new_x;
			old_y = new_y;

			move = WORLD_MOVE;
		}


		//if (LOWORD(wParam) & MK_LBUTTON) {

		//	if (isReset) {
		//		isReset = false;
		//	}
		//	else {
		//		D3DXVECTOR3 vDist;
		//		D3DXVECTOR3 vTrans;
		//		D3DXMATRIX mTrans;
		//		D3DXMATRIX mX;
		//		D3DXMATRIX mY;

		//		switch (move) {
		//		case WORLD_MOVE:
		//			/*dx = (old_x - new_x) * 0.01f;
		//			dy = (old_y - new_y) * 0.01f;
		//			D3DXMatrixRotationY(&mX, dx);
		//			D3DXMatrixRotationX(&mY, dy);
		//			g_mWorld = g_mWorld * mX * mY;*/
		//			break;
		//		}
		//	}

		//	old_x = new_x;
		//	old_y = new_y;

		//}
		//else {
		//	isReset = true;

		//	if (LOWORD(wParam) & MK_RBUTTON) {
		//		dx = (old_x - new_x);// * 0.01f;
		//		dy = (old_y - new_y);// * 0.01f;

		//		D3DXVECTOR3 coord3d = g_controlball.getCenter();
		//		g_controlball.setCenter(coord3d.x, coord3d.y, coord3d.z + dy * (0.1f));
		//	}
		//	old_x = new_x;
		//	old_y = new_y;

		//	move = WORLD_MOVE;
		//}

		break;
	}
	}

	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hinstance,
				   HINSTANCE prevInstance, 
				   PSTR cmdLine,
				   int showCmd)
{
    srand(static_cast<unsigned int>(time(NULL)));
	
	if(!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}
	
	if(!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}
	
	d3d::EnterMsgLoop( Display );
	
	Cleanup();
	
	Device->Release();
	
	return 0;
}