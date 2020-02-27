#include "../Common/InstancePool.h"
#include "../Scene/scene.h"

class Behavior : public InstancePool<Behavior>
{
public:
	Behavior() {}
	virtual ~Behavior() {}
protected:
	/// <summary>
	/// Called per frame, contains the behavior's logic
	/// </summary>
	virtual void Update() = 0;
private:
	friend void Scene::run();
	static void UpdateAll() {
		for (auto &behavior : instances)
		{
			behavior->Update();
		}
	}
};

class FreeMove : public Behavior
{
protected:
	void Update() override {
		
	}
public:
	FreeMove() = default;
	~FreeMove() = default;
};


