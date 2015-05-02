#include "GameObject.h"


GameObject::GameObject(void)
{
	mPosition = glm::vec3(0);
	mDirection = glm::vec3(1.0f,0.0f,0.0f);
	mDirectionAngle = 0;
	mModelMatrix = glm::mat4(1);
	mIsDestroid = false;
}

GameObject::GameObject(glm::vec3 fPosition,glm::vec3 fDirection,bool fIsDestroid /*= false*/)
{
	mPosition = fPosition;
	mDirection = fDirection;
	mIsDestroid = fIsDestroid;
	//To be tested!!!
	mDirectionAngle = atan2(mDirection.z,mDirection.x)* 180.0/PI;
	UpdateModelMatrix();
}


GameObject::~GameObject(void)
{
}

void GameObject::SetDirection(glm::vec3 fDirection)
{
	mDirection  = fDirection;
	mDirectionAngle = atan2(mDirection.z,mDirection.x) * 180.0/PI;
}

glm::vec3 GameObject::GetDirection()
{
	return mDirection;
}

void GameObject::SetPosition(glm::vec3 fPosition)
{
	mPosition = fPosition;
}

glm::vec3 GameObject::GetPosition()
{
	return mPosition;
}

void GameObject::UpdateModelMatrix()
{
	mModelMatrix =  glm::translate(mPosition) * glm::rotate(mDirectionAngle,0.0f,1.0f,0.0f)* mInitialTransformations;
}


