#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>
#include "../Common/const_def.h"

class GLTransform {
public:
	GLTransform(glm::vec3 _pos, glm::vec3 angles = glm::vec3(0));
	
	/// <summary>
	/// Clearns the transform changed map
	/// </summary>
	static void CleanEventMap();
	/// <summary>
	/// Registers a change in the transform parameters onto
	/// the transform changed map
	/// </summary>
	/// <param name="val">if set to <c>true</c> [value].</param>
	void RegisterChange(bool val = true) const;
	/// <summary>
	/// Determines whether this transform has changed
	/// </summary>
	/// <returns></returns>
	bool TransformChanged() const;
	/// <summary>
	/// Returns a map of bool associated with a transform that
	/// tells if the transform has changed
	/// </summary>
	/// <returns></returns>
	static const std::unordered_map<const GLTransform *, bool> &TransformChangedMap();

	/// <summary>
	/// Updates the transform matrix.
	/// </summary>
	virtual void UpdateTransformMatrix();
	/// <summary>
	/// Sets the transform position
	/// </summary>
	/// <param name="val">The value.</param>
	void Position(const glm::vec3 &val);
	/// <summary>
	/// Sets the transform rotation.
	/// </summary>
	/// <param name="val">The value.</param>
	void Rotation(const glm::quat &val);
	/// <summary>
	/// Sets the transform rotation using euler angles
	/// </summary>
	/// <param name="angles">The angles.</param>
	void Rotation(const glm::vec3 &angles);
	/// <summary>
	/// Sets the transform scale
	/// </summary>
	/// <param name="val">The value.</param>
	void Scale(const glm::vec3 &val);
	/// <summary>
	/// Sets the forward direction vector. Warning this does not update the
	/// rest of the direction vectors.
	/// </summary>
	/// <param name="val">The value.</param>
	void Forward(const glm::vec3 &val);
	/// <summary>
	/// Sets the right direction vector. Warning this does not update the
	/// rest of the direction vectors.
	/// </summary>
	/// <param name="val">The value.</param>
	void Right(const glm::vec3 &val);
	/// <summary>
	/// Sets the up direction vector. Warning this does not update the
	/// rest of the direction vectors.
	/// </summary>
	/// <param name="val">The value.</param>
	void Up(const glm::vec3 &val);

	const glm::vec3 &Position() const;
	const glm::quat &Rotation() const;
	const glm::vec3 &Scale() const;
	const glm::vec3 &Forward() const;
	const glm::vec3 &Right() const;
	const glm::vec3 &Up() const;
	const glm::vec3 &Angles() const;
	const glm::mat4x4 &Matrix() const;

	GLTransform();
	virtual ~GLTransform();

	void move(float frontDelta, float rightDelta, float upDelta = 0) {
		position += frontDelta * forward + rightDelta * right + upDelta * up;
		UpdateTransformMatrix();
	}

	void rollUp(float r) {
		glm::vec3 nangles = glm::vec3(this->angles.x + r, this->angles.y, this->angles.z);
		Rotation(nangles);
	}
	void rollLeft(float r) {
		glm::vec3 nangles = glm::vec3(this->angles.x, this->angles.y + r, this->angles.z);
		Rotation(nangles);
	}

private:
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;
	glm::vec3 angles;
	glm::mat4x4 transformation;
	void UpdateCoordinates();
	static std::unordered_map<const GLTransform *, bool> transformChange;
};