#include <iostream>

#include <glm/glm.hpp>
#include <vector>

using vec3 = glm::vec3;

#define NUM_OF_PARTICLES 100
#define NUM_OF_ITERATIONS 5
#define TIMESTAMP 0.00001f
#define WIDTH 800
#define HEIGHT 600

struct Constraint
{
	int particleA;
	int particleB;
	float restLength;
};

class ParticleSystem
{
public:
	ParticleSystem();
	void step();
private:
	void verlet();
	void satisfyContraints();
	void accumulateForces();

	void applyConstraint(const Constraint& constraint);
private:
	vec3 m_currentPos[NUM_OF_PARTICLES]{};
	vec3 m_previousPos[NUM_OF_PARTICLES]{};
	vec3 m_forceAccumulations[NUM_OF_PARTICLES]{};
	vec3 m_gravity = {0,0,1};
	float m_timestamp = TIMESTAMP;

	std::vector<Constraint> m_constraints;
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
	for (int i = 0; i < NUM_OF_ITERATIONS; i++)
	{
		//satisfy bounds
		for (int j = 0; j < NUM_OF_PARTICLES; j++)
		{
			vec3& v = m_currentPos[j];
			glm::min(glm::max(v, { 0,0,0 }), {WIDTH,HEIGHT,HEIGHT});
		}

		// satisfy contsraints
		for (const auto& c : m_constraints)
		{
			vec3& v1 = m_currentPos[c.particleA];
			vec3& v2 = m_currentPos[c.particleB];
			vec3 delta = v2 - v1;
			float deltaLength = glm::length(delta);
			float diff = (deltaLength - c.restLength) / deltaLength;
			v1 += delta * .5f * diff;
			v2 -= delta * .5f * diff;
		}

	}
}

void ParticleSystem::accumulateForces()
{
	for (int i = 0; i < NUM_OF_PARTICLES; i++)
	{
		m_forceAccumulations[i] = m_gravity;
	}
}

void ParticleSystem::applyConstraint(const Constraint& constraint)
{
	m_constraints.push_back(constraint);
}

int main()
{
	ParticleSystem ps;
	
	while (true)
	{
		ps.step();
	}
}