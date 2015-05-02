#ifndef Hero_h__
#define Hero_h__
#include "GameObject.h"
#include <memory>
#include "MD2Model\md2model.h"
#include "EulerCamera\EulerCamera.h"
#include "Third Person Camera\ThirdPersonCamera.h"

class Hero : public GameObject, public CMD2Model
{

	KeyFrameAnimationShader* mShader;
	animState_t mAnimationState;
	void Initialize();
	glm::vec3 mInitialCameraOffset;
	void Move(glm::vec3 fDirection);

public:
	std::unique_ptr<ThirdPersonCamera> Camera;
	Hero(KeyFrameAnimationShader* fShader);
	Hero(KeyFrameAnimationShader* fShader,glm::vec3 fPostion,glm::vec3 fDirection, bool fIsDestroid = false);
	~Hero(void);
	virtual void Collided(ObjectType fObject);
	void UpdateAnimation(float deltaTime);
	void Render(glm::mat4 VP);
	void Walk(float dist);
	void Strafe(float dist);
};
#endif // Hero_h__

