#include <iostream>

#include <glm/glm.hpp>

using vec3 = glm::vec3;

#define NUM_OF_PARTICLES 100
#define TIMESTAMP 0.00001f

class ParticleSystem
{
public:
	ParticleSystem();
	void step();
private:
	void verlet();
	void satisfyContraints();
	void accumulateForces();
private:
	vec3 m_currentPos[NUM_OF_PARTICLES]{};
	vec3 m_previousPos[NUM_OF_PARTICLES]{};
	vec3 m_forceAccumulations[NUM_OF_PARTICLES]{};
	vec3 m_gravity = {0,0,1};
	float m_timestamp = TIMESTAMP;
};

ParticleSystem::ParticleSystem()
{

}

void ParticleSystem::step()
{
	accumulateForces();
	verlet();
	satisfyContraints();
}

void ParticleSystem::verlet()
{
	for (int i = 0; i < NUM_OF_PARTICLES; i++)
	{
		vec3& x = m_currentPos[i];
		vec3 temp_x = x;
		vec3& old_x = m_previousPos[i];
		vec3& a = m_forceAccumulations[i];

		x += x - old_x + a * m_timestamp * m_timestamp;

		old_x = temp_x;
	}
}

void ParticleSystem::satisfyContraints()
{
}

void ParticleSystem::accumulateForces()
{
	for (int i = 0; i < NUM_OF_PARTICLES; i++)
	{
		m_forceAccumulations[i] = m_gravity;
	}
}

int main()
{
	ParticleSystem ps;
	
	while (true)
	{
		ps.step();
	}
}