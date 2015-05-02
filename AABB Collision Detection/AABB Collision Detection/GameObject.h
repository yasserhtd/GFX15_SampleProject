
#ifndef GameObject_h__
#define GameObject_h__
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

class GameObject
{
#define  PI 3.14159265359
private:
	glm::vec3 mDirection;

	float mDirectionAngle;
	glm::vec3 mPosition;

	bool mIsDestroid;
protected:
	glm::mat4 mModelMatrix;
	glm::mat4 mInitialTransformations;
public:
	GameObject(void);
	GameObject(glm::vec3 fPosition,glm::vec3 fDirection,bool fIsDestroid = false);
	void SetDirection(glm::vec3 fDirection);
	glm::vec3 GetDirection();
	void SetPosition(glm::vec3 fPosition);
	glm::vec3 GetPosition();
	void UpdateModelMatrix();
	~GameObject(void);

};
#endif // GameObject_h__

