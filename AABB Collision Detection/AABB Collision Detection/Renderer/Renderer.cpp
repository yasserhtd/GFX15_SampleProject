#include "Renderer.h"

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

	hero = std::unique_ptr<Hero>(new Hero(&animatedModelShader));

	collisionManager->AddCollidableModel((CollidableModel*)hero.get());
	


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
	glUniform3fv(EyePositionID,1, &hero->Camera->GetEyePosition()[0]);

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
	glUniform3fv(EyePositionID,1, &hero->Camera->GetEyePosition()[0]);
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
		glm::mat4 VP = hero->Camera->GetProjectionMatrix() * hero->Camera->GetViewMatrix();


		staticModelShader.UseProgram();
		staticModelShader.BindVPMatrix(&VP[0][0]);
		staticModelShader.BindModelMatrix(&floorM[0][0]);
		floorTexture->Bind();
		floor->Draw();

		mySpider->Render(&staticModelShader,spiderM);
		house->Render(&staticModelShader,houseM);
		
	

		hero->Render(VP);
}

void Renderer::Cleanup()
{
}

void Renderer::Update(double deltaTime)
{
	
	collisionManager->UpdateCollisions();
	hero->UpdateAnimation(deltaTime/1000);
}

void Renderer::HandleKeyboardInput(int key)
{
	float step = 0.1f;


	//NOTE!!! 
	//this is the worst way possible for moving an object with the camera.
	// but it'll serve our purpose for now.

	switch (key)
	{
		//Moving forward
	case GLFW_KEY_UP:
	case GLFW_KEY_W:
		hero->Walk(0.05f);
		break;

		//Moving backword
	case GLFW_KEY_DOWN:
	case GLFW_KEY_S:
		hero->Walk(-0.05f);
		break;

		// Moving right
	case GLFW_KEY_RIGHT:
	case GLFW_KEY_D:
		hero->Strafe(0.05f);
		break;

		// Moving left
	case GLFW_KEY_LEFT:
	case GLFW_KEY_A:
		hero->Strafe(-0.05f);
		break;

		// Moving up
	case GLFW_KEY_SPACE:
	case GLFW_KEY_R:
		
		break;

		// Moving down
	case GLFW_KEY_LEFT_CONTROL:
	case GLFW_KEY_F:
		
		break;
	default:
		break;
	}

	


	//update the eye position uniform.
	staticModelShader.UseProgram();
	EyePositionID = glGetUniformLocation(staticModelShader.programID,"EyePosition_worldspace");
	glUniform3fv(EyePositionID,1, &hero->Camera->GetEyePosition()[0]);

	animatedModelShader.UseProgram();
	EyePositionID = glGetUniformLocation(animatedModelShader.programID,"EyePosition_worldspace");
	glUniform3fv(EyePositionID,1, &hero->Camera->GetEyePosition()[0]);
}

void Renderer::HandleMouse(double deltaX,double deltaY)
{	
	double scaleAngle = 10.0;
	hero->Camera->Yaw(deltaX*scaleAngle);
	hero->Camera->Pitch(deltaY*scaleAngle);
	hero->Camera->UpdateViewMatrix();
}

