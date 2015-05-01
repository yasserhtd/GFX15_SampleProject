﻿#include "Renderer.h"

#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtx/transform.hpp>
#include "OBJLoader/objloader.hpp"

Renderer::Renderer()
{
    
}

Renderer::~Renderer()
{
    Cleanup();
}

void Renderer::Initialize()
{
	//myCamera = std::unique_ptr<FPCamera>(new FPCamera());
	myCamera = std::unique_ptr<EulerCamera>(new EulerCamera());
	collisionManager = std::unique_ptr<CollisionManager>( new CollisionManager());
	//////////////////////////////////////////////////////////////////////////
	//drawing a square.
	floor = std::unique_ptr<Model>(new Model());

	floor->VertexData.push_back(glm::vec3(-1.0f, -1.0f, 0.0f));
	floor->UVData.push_back(glm::vec2(0.0f,0.0f));
	floor->VertexData.push_back(glm::vec3(1.0f, -1.0f, 0.0f));
	floor->UVData.push_back(glm::vec2(10.0f,0.0f));
	floor->VertexData.push_back(glm::vec3( 1.0f,  1.0f, 0.0f));
	floor->UVData.push_back(glm::vec2(10.0f,10.0f));
	floor->VertexData.push_back(glm::vec3( -1.0f,  1.0f, 0.0f));
	floor->UVData.push_back(glm::vec2(0.0f,10.0f));
	
	//first triangle.
	floor->IndicesData.push_back(0);
	floor->IndicesData.push_back(1);
	floor->IndicesData.push_back(3);

	//second triangle.
	floor->IndicesData.push_back(1);
	floor->IndicesData.push_back(2);
	floor->IndicesData.push_back(3);
	glm::vec3 squareNormal = glm::vec3(0.0,0.0,1.0);
	floor->NormalsData.push_back(squareNormal);
	floor->NormalsData.push_back(squareNormal);
	floor->NormalsData.push_back(squareNormal);
	floor->NormalsData.push_back(squareNormal);
	floor->Initialize();

	floorTexture = std::unique_ptr<Texture>(new Texture("data/textures/rock.jpg",0));
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	blade.LoadModel("data/models/blade/Blade.md2");
	//blade.LoadModel("data/models/samourai/Samourai.md2");
	bladeAnimationState = blade.StartAnimation(animType_t::STAND);
	//calculate AABB
	blade.SetBoundingBox(CollidableModel::CalculateBoundingBox(blade.GetVertices()));
	collisionManager->AddCollidableModel((CollidableModel*) &blade);
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	//load model.
	mySpider = std::unique_ptr<Model3D>(new Model3D());
	//read model and it's textures from HDD.
	mySpider->LoadFromFile("data/models/Spider/spider.obj",true);
	//send the meshes to the GPU.
	mySpider->Initialize();
	mySpider->SetBoundingBox(CollidableModel::CalculateBoundingBox(mySpider->GetVertices()));
	collisionManager->AddCollidableModel((CollidableModel*) mySpider.get());
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	house = std::unique_ptr<Model3D>(new Model3D());
	house->LoadFromFile("data/models/House/house.3ds");
	house->Initialize();
	house->SetBoundingBox(CollidableModel::CalculateBoundingBox(house->GetVertices(),-2.0f));
	collisionManager->AddCollidableModel((CollidableModel*) house.get());
	//////////////////////////////////////////////////////////////////////////



	//////////////////////////////////////////////////////////////////////////
	// Create and compile our GLSL program from the shaders
	animatedModelShader.LoadProgram();
	staticModelShader.LoadProgram();
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// Projection matrix : 
	myCamera->SetPerspectiveProjection(45.0f,4.0f/3.0f,0.1f,100.0f);

	// View matrix : 
	myCamera->Reset(0.0,1.0,2.0,
					0,0,0,
					0,1,0);
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Configure the light.
	//setup the light position.
	staticModelShader.UseProgram();
	LightPositionID = glGetUniformLocation(staticModelShader.programID,"LightPosition_worldspace");
	lightPosition = glm::vec3(5.0,5.0,0.0);
	glUniform3fv(LightPositionID,1, &lightPosition[0]);
	//setup the ambient light component.
	AmbientLightID = glGetUniformLocation(staticModelShader.programID,"ambientLight");
	ambientLight = glm::vec3(0.1,0.1,0.1);
	glUniform3fv(AmbientLightID,1, &ambientLight[0]);
	//setup the eye position.
	EyePositionID = glGetUniformLocation(staticModelShader.programID,"EyePosition_worldspace");
	//send the eye position to the shaders.
	glUniform3fv(EyePositionID,1, &myCamera->GetEyePosition()[0]);

	///////////////////////////////////////////////////
	//repeat the process for the animated models shader.
	animatedModelShader.UseProgram();
	LightPositionID = glGetUniformLocation(animatedModelShader.programID,"LightPosition_worldspace");
	lightPosition = glm::vec3(1.0,0.25,0.0);
	glUniform3fv(LightPositionID,1, &lightPosition[0]);
	//setup the ambient light component.
	AmbientLightID = glGetUniformLocation(animatedModelShader.programID,"ambientLight");
	ambientLight = glm::vec3(0.5,0.5,0.5);
	glUniform3fv(AmbientLightID,1, &ambientLight[0]);
	//setup the eye position.
	EyePositionID = glGetUniformLocation(animatedModelShader.programID,"EyePosition_worldspace");
	//send the eye position to the shaders.
	glUniform3fv(EyePositionID,1, &myCamera->GetEyePosition()[0]);
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Setting the initaial transformations
	floorM =  glm::scale(2.0f,2.0f,2.0f)*glm::rotate(-90.0f,glm::vec3(1.0f,0.0f,0.0f));
	spiderM = glm::translate(-1.0f,0.25f,0.0f) * glm::scale(0.005f,0.005f,0.005f);
	//we have to scale and translate the spider's bounding box to match the new position and size.
	auto tempBox = mySpider->GetBoundingBox();
	tempBox.Scale(0.005f,0.005f,0.005f);
	tempBox.Translate(-1.0f,0.25f,0.0f);
	mySpider->SetBoundingBox(tempBox);
	
	bladeM = glm::rotate(-90.0f,1.0f,0.0f,0.0f) * glm::scale(0.01f,0.01f,0.01f);
	tempBox = blade.GetBoundingBox();
	tempBox.Scale(0.01f,0.01f,0.01f);
	tempBox.Rotate(-90.0f,1.0f,0.0f,0.0f);
	blade.SetBoundingBox(tempBox);

	houseM = glm::rotate(90.0f,1.0f,0.0f,0.0f) *glm::scale(0.1f,0.1f,0.1f);
	tempBox = house->GetBoundingBox();
	tempBox.Scale(0.1f,0.1f,0.1f);
	tempBox.Rotate(90.0f,1.0f,0.0f,0.0f);
	house->SetBoundingBox(tempBox);


	//////////////////////////////////////////////////////////////////////////

}

void Renderer::Draw()
{		

		//Bind the VP matrix (Camera matrices) to the current shader.
		glm::mat4 VP = myCamera->GetProjectionMatrix() * myCamera->GetViewMatrix();


		staticModelShader.UseProgram();
		staticModelShader.BindVPMatrix(&VP[0][0]);
		staticModelShader.BindModelMatrix(&floorM[0][0]);
		floorTexture->Bind();
		floor->Draw();

		mySpider->Render(&staticModelShader,spiderM);
		house->Render(&staticModelShader,houseM);
		
		animatedModelShader.UseProgram();
		animatedModelShader.BindVPMatrix(&VP[0][0]);
		animatedModelShader.BindModelMatrix(&bladeM[0][0]);
		blade.RenderModel(&bladeAnimationState,&animatedModelShader);
}

void Renderer::Cleanup()
{
}

void Renderer::Update(double deltaTime)
{
	blade.UpdateAnimation(&bladeAnimationState,deltaTime/1000);
	collisionManager->UpdateCollisions();
}

void Renderer::HandleKeyboardInput(int key)
{
	float step = 0.1f;
	auto tempbox = blade.GetBoundingBox();

	//NOTE!!! 
	//this is the worst way possible for moving an object with the camera.
	// but it'll serve our purpose for now.

	switch (key)
	{
		//Moving forward
	case GLFW_KEY_UP:
	case GLFW_KEY_W:
		myCamera->Walk(step);
		bladeM = glm::translate(0.0f,0.0f,-step)*bladeM;
		tempbox.Translate(0.0f,0.0f,-step);
		break;

		//Moving backword
	case GLFW_KEY_DOWN:
	case GLFW_KEY_S:
		myCamera->Walk(-step);
		bladeM = glm::translate(0.0f,0.0f,step)*bladeM;
		tempbox.Translate(0.0f,0.0f,step);
		break;

		// Moving right
	case GLFW_KEY_RIGHT:
	case GLFW_KEY_D:
		myCamera->Strafe(step);
		bladeM = glm::translate(step,0.0f,0.0f)*bladeM;
		tempbox.Translate(step,0.0f,0.0f);
		break;

		// Moving left
	case GLFW_KEY_LEFT:
	case GLFW_KEY_A:
		myCamera->Strafe(-step);
		bladeM = glm::translate(-step,0.0f,0.0f)*bladeM;
		tempbox.Translate(-step,0.0f,0.0f);
		break;

		// Moving up
	case GLFW_KEY_SPACE:
	case GLFW_KEY_R:
		myCamera->Fly(step);
		bladeM = glm::translate(0.0f,step,0.0f)*bladeM;
		tempbox.Translate(0.0f,step,0.0f);
		break;

		// Moving down
	case GLFW_KEY_LEFT_CONTROL:
	case GLFW_KEY_F:
		myCamera->Fly(-step);
		bladeM = glm::translate(0.0f,-step,0.0f)*bladeM;
		tempbox.Translate(0.0f,-step,0.0f);
		break;
	default:
		break;
	}

	//Set the transformed bounding box again.
	blade.SetBoundingBox(tempbox);

	//continue the remaining movements.
	myCamera->UpdateViewMatrix();

	//update the eye position uniform.
	staticModelShader.UseProgram();
	EyePositionID = glGetUniformLocation(staticModelShader.programID,"EyePosition_worldspace");
	glUniform3fv(EyePositionID,1, &myCamera->GetEyePosition()[0]);

	animatedModelShader.UseProgram();
	EyePositionID = glGetUniformLocation(animatedModelShader.programID,"EyePosition_worldspace");
	glUniform3fv(EyePositionID,1, &myCamera->GetEyePosition()[0]);
}

void Renderer::HandleMouse(double deltaX,double deltaY)
{	
	myCamera->Yaw(deltaX);
	myCamera->Pitch(deltaY);
	myCamera->UpdateViewMatrix();
}

