#include "Transform.h"

std::unordered_map<const GLTransform *, bool> GLTransform::transformChange;

GLTransform::GLTransform(glm::vec3 _pos,
	glm::vec3 angles) {
	position = _pos;
	scale = Vector3::one;
	Rotation(angles);
	transformation = translate(Matrix::identity4, position) *
		mat4_cast(rotation) *
		glm::scale(Matrix::identity4, scale);
}

GLTransform::GLTransform()
{
	position = Vector3::zero;
	scale = Vector3::one;
	rotation = glm::quat(Vector3::zero);
	forward = Vector3::forward;
	up = Vector3::up;
	right = Vector3::right;
	transformation = translate(Matrix::identity4, position) *
		mat4_cast(rotation) *
		glm::scale(Matrix::identity4, scale);
}

GLTransform::~GLTransform()
{
}

void GLTransform::Position(const glm::vec3 &val)
{
	if (position != val)
	{
		position = val;
		UpdateTransformMatrix();
		transformChange[this] = true;
	}
}

void GLTransform::Rotation(const glm::quat &val)
{
	if (rotation != val)
	{
		this->angles = eulerAngles(val);
		rotation = val;
		UpdateCoordinates();
		UpdateTransformMatrix();
		transformChange[this] = true;
	}
}

void GLTransform::Rotation(const glm::vec3 &angles)
{
	if (this->angles != angles)
	{
		this->angles = angles;
		auto rotationX = angleAxis(angles.x, Vector3::right);
		auto rotationY = angleAxis(angles.y, Vector3::up);
		auto rotationZ = angleAxis(angles.z, Vector3::forward);
		// final composite rotation
		rotation = normalize(rotationZ * rotationX * rotationY);
		// rotate direction vectors
		UpdateCoordinates();
		UpdateTransformMatrix();
		transformChange[this] = true;
	}
}

void GLTransform::Scale(const glm::vec3 &val)
{
	if (scale != val)
	{
		scale = val;
		UpdateTransformMatrix();
		transformChange[this] = true;
	}
}

const glm::vec3 &GLTransform::Position() const
{
	return position;
}

const glm::quat &GLTransform::Rotation() const
{
	return rotation;
}

void GLTransform::UpdateCoordinates()
{
	up = normalize(Vector3::up * rotation);
	right = normalize(Vector3::right * rotation);
	forward = normalize(Vector3::forward * rotation);
}

void GLTransform::UpdateTransformMatrix()
{
	transformation = translate(Matrix::identity4, position) *
		mat4_cast(rotation) *
		glm::scale(Matrix::identity4, scale);
}

bool GLTransform::TransformChanged() const
{
	auto it = transformChange.find(this);

	if (it != transformChange.end()) return it->second;

	return false;
}

const std::unordered_map<const GLTransform *, bool>
&GLTransform::TransformChangedMap()
{
	return transformChange;
}

void GLTransform::CleanEventMap()
{
	transformChange.clear();
}

void GLTransform::RegisterChange(bool val) const
{
	transformChange[this] = val;
}

const glm::vec3 &GLTransform::Scale() const
{
	return scale;
}

const glm::vec3 &GLTransform::Forward() const
{
	return forward;
}

const glm::vec3 &GLTransform::Right() const
{
	return right;
}

const glm::vec3 &GLTransform::Up() const
{
	return up;
}

void GLTransform::Forward(const glm::vec3 &val)
{
	if (forward != val)
	{
		forward = val;
		UpdateTransformMatrix();
		transformChange[this] = true;
	}
}

void GLTransform::Right(const glm::vec3 &val)
{
	if (right != val)
	{
		right = val;
		UpdateTransformMatrix();
		transformChange[this] = true;
	}
}

void GLTransform::Up(const glm::vec3 &val)
{
	if (up != val)
	{
		up = val;
		UpdateTransformMatrix();
		transformChange[this] = true;
	}
}

const glm::vec3 &GLTransform::Angles() const
{
	return angles;
}

const glm::mat4x4 &GLTransform::Matrix() const
{
	return transformation;
}