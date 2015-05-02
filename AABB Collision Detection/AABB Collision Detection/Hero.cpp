#include "Hero.h"


Hero::Hero(KeyFrameAnimationShader* fShader) : GameObject()
{
	mShader = fShader;
	Initialize();
}

Hero::Hero(KeyFrameAnimationShader* fShader,glm::vec3 fPostion,glm::vec3 fDirection, bool fIsDestroid /*= false*/): GameObject(fPostion,fDirection,fIsDestroid)
{
	mShader = fShader;
	Initialize();
	auto tempBoundingBox = CMD2Model::GetBoundingBox();
	tempBoundingBox.Translate(fPostion);
}


Hero::~Hero(void)
{
}

void Hero::UpdateAnimation(float deltaTime)
{	
	if (CMD2Model::IsAnimationFinished == true && mAnimationState.type != animType_t::STAND)
	{
		mAnimationState = CMD2Model::StartAnimation(animType_t::STAND);
	}

	CMD2Model::UpdateAnimation(&mAnimationState,deltaTime);

}

void Hero::Initialize()
{
	CMD2Model::LoadModel("data/models/blade/Blade.md2");
	mAnimationState = this->StartAnimation(animType_t::STAND);
	CMD2Model::SetBoundingBox(CollidableModel::CalculateBoundingBox(CMD2Model::GetVertices()));
	mInitialTransformations =  glm::rotate(-90.0f,1.0f,0.0f,0.0f) * glm::scale(0.01f,0.01f,0.01f);
	GameObject::UpdateModelMatrix();

	auto tempBoundingBox = CMD2Model::GetBoundingBox();
	tempBoundingBox.Scale(0.01f,0.01f,0.01f);
	tempBoundingBox.Rotate(-90.0f,1.0f,0.0f,0.0f);
	CMD2Model::SetBoundingBox(tempBoundingBox);

	mInitialCameraOffset = glm::vec3(0.0f,1.0f,2.0f);
	Camera = std::unique_ptr<ThirdPersonCamera>(new ThirdPersonCamera());
	auto position = GameObject::GetPosition();
	Camera->Reset(position+mInitialCameraOffset,position,glm::vec3(0.0f,1.0f,0.0f));
	Camera->SetPerspectiveProjection(45.0f,4.0f/3.0f,0.1f,100.0f);
}

void Hero::Render(glm::mat4 VP)
{
	mShader->UseProgram();
	mShader->BindVPMatrix(&VP[0][0]);
	mShader->BindModelMatrix(&mModelMatrix[0][0]);
	CMD2Model::RenderModel(&mAnimationState,mShader);
}

void Hero::Collided(ObjectType fObject)
{
	printf("I'm the HERO Collided()!!!\n");
}

void Hero::Move(glm::vec3 fDirection)
{
	auto oldDirection = GameObject::GetDirection();
	
	auto newPosition = GameObject::GetPosition();
	newPosition = newPosition + fDirection;
	GameObject::SetPosition(newPosition);
	fDirection.z *= -1;
	fDirection = glm::normalize(fDirection);
	GameObject::SetDirection(fDirection);
	GameObject::UpdateModelMatrix();

	auto newBoundingBox = CollidableModel::GetBoundingBox();
	newBoundingBox.SetCenter(newPosition);
//	float rotationAngle = glm::acos(glm::dot(oldDirection,fDirection));
//	newBoundingBox.Rotate(rotationAngle,0.0f,1.0f,0.0f);
	if (CMD2Model::IsAnimationFinished == true || mAnimationState.type != animType_t::RUN)
	{
		mAnimationState = CMD2Model::StartAnimation(animType_t::RUN);
	}
	
}

void Hero::Walk(float dist)
{
	auto cameraDirection = Camera->GetDirection();
	auto direction =  glm::vec3(cameraDirection.x,0,cameraDirection.z);
	Hero::Move(dist*direction);
	Camera->Walk(dist);
	Camera->UpdateViewMatrix();
}

void Hero::Strafe(float dist)
{
	auto cameraRight = Camera->GetRight();
	auto direction = glm::vec3(cameraRight.x,0,cameraRight.z);
	Hero::Move(dist*direction);
	Camera->Strafe(dist);
	Camera->UpdateViewMatrix();
}
