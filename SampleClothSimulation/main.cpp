#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include <SFML/Graphics.hpp>

using vec3 = glm::vec3;

#define NUM_OF_ITERATIONS 3
#define TIMESTAMP 0.1f
#define WIDTH 800
#define HEIGHT 600
#define MAX_FPS 60

static constexpr vec3 g_gravity = { 0, 9.81f, 0 };

#define logInfo(msg) std::cout << msg << std::endl;
#define logDebug(msg) std::cout << msg << std::endl;
#define logWarning(msg) std::cout << msg << std::endl;
#define logError(msg) std::cout << msg << std::endl;

class ParticleSystem;
class Particle;
class Constraint;
class Cloth;

class Particle
{
public:
	Particle();
	Particle(const vec3& pos);
	vec3 getCurrentPos() const;
	vec3 getPreviousPos() const;
	void setPos(const vec3& pos);
	void move(const vec3& pos);
	void addforce(const vec3& force);
	void resetForces();
	void update(float dt);
	void pin();

private:
	vec3 m_currentPos;
	vec3 m_previousPos;
	vec3 m_forceAccumulations;
	bool m_isPinned = false;
};

Particle::Particle()
	: m_currentPos({}), m_previousPos(m_currentPos), m_forceAccumulations()
{
}

Particle::Particle(const vec3& pos)
	: m_currentPos(pos), m_previousPos(m_currentPos), m_forceAccumulations()
{
}

vec3 Particle::getCurrentPos() const
{
	return m_currentPos;
}

vec3 Particle::getPreviousPos() const
{
	return m_previousPos;
}

void Particle::setPos(const vec3& pos)
{
	m_currentPos = pos;
}

void Particle::move(const vec3& pos)
{
	if (m_isPinned)
		return;

	m_currentPos += pos;
}

void Particle::addforce(const vec3& force)
{
	m_forceAccumulations += force;
}

void Particle::resetForces()
{
	m_forceAccumulations = {};
}

void Particle::update(float dt)
{
	if (m_isPinned)
		return;

	vec3& x = m_currentPos;
	vec3 temp_x = x;
	vec3& old_x = m_previousPos;
	vec3& a = m_forceAccumulations;
	x += (x - old_x) + a * (dt * dt);
	old_x = temp_x;
}

void Particle::pin()
{
	m_isPinned = true;
}

class Constraint
{
public:
	Constraint(Particle* particleA, Particle* particleB, float restLength);
	Particle* getParticleA() const;
	Particle* getParticleB() const;
	float getRestLength() const;
private:
	Particle* m_particleA = nullptr;
	Particle* m_particleB = nullptr;
	float m_restLength = 0;
};

Constraint::Constraint(Particle* particleA, Particle* particleB, float restLength)
	: m_particleA(particleA), m_particleB(particleB), m_restLength(restLength)
{}

Particle* Constraint::getParticleA() const
{
	return m_particleA;
}

Particle* Constraint::getParticleB() const
{
	return m_particleB;
}

float Constraint::getRestLength() const
{
	return m_restLength;
}

class Cloth
{
public:
	Cloth(ParticleSystem& ps, const vec3& pos, int sizeH, int sizeV, int gridSize);
};

class ParticleSystem
{
public:
	ParticleSystem();
	~ParticleSystem();
	void step(float dt);
	void addConstraint(const Constraint& c);
	void addParticle(Particle* p);
	void draw(std::function<void(const Particle* particle)> cb) const;
	void draw(std::function<void(const Constraint& constraint)> cb) const;
private:
	void verlet(float dt);
	void satisfyContraints();
	void accumulateForces();
private:
	vec3 m_gravity = g_gravity;
	std::vector<Particle*> m_particles;
	std::vector<Constraint> m_constraints;
};

ParticleSystem::ParticleSystem()
{}

ParticleSystem::~ParticleSystem()
{
	for (int i = m_particles.size() - 1; i >= 0; i--)
	{
		delete m_particles[i];
	}
}

void ParticleSystem::step(float dt)
{
	accumulateForces();
	verlet(dt);
	satisfyContraints();
}

void ParticleSystem::verlet(float dt)
{
	for (auto& p : m_particles)
	{
		p->update(dt);
	}
}

void ParticleSystem::satisfyContraints()
{
	for (int i = 0; i < NUM_OF_ITERATIONS; i++)
	{
		//satisfy bounds
		for (auto& p : m_particles)
		{
			vec3 v = p->getCurrentPos();
			p->setPos(glm::min(glm::max(v, { 0,0,0 }), { WIDTH,HEIGHT,HEIGHT }));
		}

		// satisfy contsraints
		for (const auto& c : m_constraints)
		{
			vec3 v1 = c.getParticleA()->getCurrentPos();
			vec3 v2 = c.getParticleB()->getCurrentPos();
			vec3 delta = v2 - v1;
			float deltaLength = glm::length(delta);
			float diff = (deltaLength - c.getRestLength()) / deltaLength;
			c.getParticleA()->move(delta * .5f * diff);
			c.getParticleB()->move(-(delta * .5f * diff));
		}
	}
}

void ParticleSystem::accumulateForces()
{
	for (auto& p : m_particles)
	{
		p->resetForces();
		p->addforce(m_gravity);
	}
}

void ParticleSystem::addConstraint(const Constraint& c)
{
	m_constraints.push_back(c);
}

void ParticleSystem::addParticle(Particle* p)
{
	m_particles.push_back(p);
}

void ParticleSystem::draw(std::function<void(const Particle* particle)> cb) const
{
	for (const auto& p : m_particles)
	{
		cb(p);
	}
}

void ParticleSystem::draw(std::function<void(const Constraint& constraint)> cb) const
{
	for (const auto& c : m_constraints)
	{
		cb(c);
	}
}

Cloth::Cloth(ParticleSystem& ps, const vec3& pos, int sizeH, int sizeV, int gridSize)
{
	const vec3& origin = pos;

	std::vector<Particle*> particles;
	particles.reserve(sizeH * sizeV);

	for (int i = 0; i < sizeV; i++)
	{
		for (int j = 0; j < sizeH; j++)
		{
			Particle* p = new Particle(origin + vec3{ j * gridSize, i * gridSize, 0});

			if (i != 0)
			{
				Constraint c(p, particles.at((i - 1) * sizeH + j), gridSize);
				ps.addConstraint(c);
			}

			if (j != 0)
			{
				Constraint c(p, particles.back(), gridSize);
				ps.addConstraint(c);
			}

			particles.push_back(p);
			ps.addParticle(p);
		}
	}

	// Constrain tops particle of the cloth
	for (int i = 0; i < sizeH; i += 2)
	{
		particles.at(i)->pin();
	}
}

int main()
{
	ParticleSystem ps;

	Cloth cloth(ps, vec3(100, 100, 0), 60, 50, 10);
	
	sf::RenderWindow window(sf::VideoMode({ WIDTH, HEIGHT }), "Cloth simulation", sf::Style::Close);

	window.setFramerateLimit(MAX_FPS);

	sf::Clock clock;
	while (window.isOpen())
	{
		window.clear();

		// Process events
		for (sf::Event event; window.pollEvent(event);)
		{
			// Close window: exit
			if (event.type == sf::Event::Closed)
				window.close();

			// Escape key: exit
			if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape))
				window.close();
		}

		ps.step(TIMESTAMP);

		ps.draw([&](const Constraint& constraint) {
			sf::Vertex line[2];
			line[0].position = sf::Vector2f(constraint.getParticleA()->getCurrentPos().x, constraint.getParticleA()->getCurrentPos().y);
			line[0].color = sf::Color::Red;
			line[1].position = sf::Vector2f(constraint.getParticleB()->getCurrentPos().x, constraint.getParticleB()->getCurrentPos().y);
			line[1].color = sf::Color::Red;
			window.draw(line, 2, sf::Lines);
			});

		// Finally, display the rendered frame on screen
		window.display();
	}
}

