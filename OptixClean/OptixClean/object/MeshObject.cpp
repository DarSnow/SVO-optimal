#include "MeshObject.h"

void ObjectManager::readConfig(string config) {
	cout << "building object" << endl;
	stringstream configStream;
	configStream.str(config);
	string operation, values1, values2, values3;
	int valuei;
	configStream >> operation;
	while (operation.compare("end") != 0) {
		if (operation.compare("model") == 0) {
			configStream >> values1 >> values2 >> valuei;
			MeshObject to(values1);
			to.transform.Scale(vec3(1, 1, -1));
			to.meshIndex = AssetManager::getInstance()->meshGroups[values1];
			objects[values1] = to;
			cout << "name: " << values1 << "  mesh index:" << to.meshIndex.first << " to " << to.meshIndex.second << endl;
		}		
		configStream >> operation;
	}
	
}
void ObjectManager::drawObject(string name,GLuint shader) {
	vector<GLMesh*> &meshes = AssetManager::getInstance()->meshes;
	MeshObject &to = objects[name];
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, &(to.transform.Matrix())[0][0]);
	for (int j = to.meshIndex.first; j < to.meshIndex.second; j++) {
		meshes[j]->meterial->bindMeterial(shader);
		meshes[j]->Draw();
	}
}

mat4 ObjectManager::getModel(string name)
{
	MeshObject to = objects[name];
	return to.transform.Matrix();
}