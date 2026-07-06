#pragma once

#include <vector>
#include <string>

struct ProjectShowcase
{
	std::string name;
	std::string path;
};

class ProjectHub
{
private:

	const std::string hubPath = "HubProjects.json";

	void Setup();

public:

	static ProjectHub* instance;

	bool projectLoaded;

	std::vector<ProjectShowcase> projects;

	void Render();

	ProjectHub();
	~ProjectHub();

	void SaveProjects();

	void CreateProject();
	void CreateProject(const std::string& path);

	void AddProject();
	void AddProject(const std::string& path);

	void LoadProject(const std::string& path);
};