#include <iostream>

#include <glm/glm.hpp>
#include <vector>
#include <functional>

#include <SFML/Graphics.hpp>

using vec3 = glm::vec3;

#define NUM_OF_ITERATIONS 1
#define TIMESTAMP 0.01f
#define WIDTH 800
#define HEIGHT 600
#define drag 0.001f

#define logInfo(msg) std::cout << msg << std::endl;
#define logDebug(msg) std::cout << msg << std::endl;
#define logWarning(msg) std::cout << msg << std::endl;
#define logError(msg) std::cout << msg << std::endl;

struct Particle
{
	vec3 m_currentPos;
	vec3 m_previousPos;
	vec3 m_forceAccumulations;
};

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
	void step(float dt);
	void addConstraint(const Constraint& c);
	void addParticle(const Particle& p);
	void draw(std::function<void(const Particle&)> cb) const;
private:
	void verlet(float dt);
	void satisfyContraints();
	void accumulateForces();


private:
	vec3 m_gravity = {0, 9.81f, 0};
	float m_timestamp = TIMESTAMP;

	std::vector<Particle> m_particles;
	std::vector<Constraint> m_constraints;
};

ParticleSystem::ParticleSystem()
{

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
		vec3& x = p.m_currentPos;
		vec3 temp_x = x;
		vec3& old_x = p.m_previousPos;
		vec3& a = p.m_forceAccumulations;
		x += (x - old_x) + a * (dt * dt);
		old_x = temp_x;
	}
}

void ParticleSystem::satisfyContraints()
{
	for (int i = 0; i < NUM_OF_ITERATIONS; i++)
	{
		//satisfy bounds
		for (auto& p : m_particles)
		{
			vec3& v = p.m_currentPos;
			v = glm::min(glm::max(v, { 0,0,0 }), { WIDTH,HEIGHT,HEIGHT });
		}

		// satisfy contsraints
		for (const auto& c : m_constraints)
		{
			vec3& v1 = m_particles[c.particleA].m_currentPos;
			vec3& v2 = m_particles[c.particleB].m_currentPos;
			vec3 delta = v2 - v1;
			float deltaLength = glm::length(delta);
			float diff = (deltaLength - c.restLength) / deltaLength;
			v1 += delta * .5f * diff;
			v2 -= delta * .5f * diff;
		}

		// Constrain one particle of the cloth to orig
		m_particles[0].m_currentPos = { WIDTH / 2 , HEIGHT / 2, 0 };

	}
}

void ParticleSystem::accumulateForces()
{
	for (auto& p : m_particles)
	{
		p.m_forceAccumulations = m_gravity;
	}
}

void ParticleSystem::addConstraint(const Constraint& c)
{
	m_constraints.push_back(c);
}

void ParticleSystem::addParticle(const Particle& p)
{
	m_particles.push_back(p);
}

void ParticleSystem::draw(std::function<void(const Particle& particle)> cb) const
{
	for (const auto& p : m_particles)
	{
		cb(p);
	}
	
}

int main()
{
	ParticleSystem ps;

	vec3 origin = { WIDTH / 2, HEIGHT / 2, 0 };

	ps.addParticle({ origin, origin, vec3{ 0, 0, 0 } });
	ps.addParticle({ origin + vec3{100,0,0}, origin + vec3{ 100,0,0 }, vec3{ 0, 0, 0 } });

	ps.addConstraint({ 0, 1, 100 });
	
	sf::RenderWindow window(sf::VideoMode({ WIDTH, HEIGHT }), "Cloth simulation", sf::Style::Close);

	window.setFramerateLimit(60);

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

			// Escape key: exit
			if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::A))
				ps.step(1.f);
		}

		ps.step(.1f);

		//sf::Time time = clock.restart();
		//float dt = (float)time.asMilliseconds() / 1000;
		//ps.step(dt);

		ps.draw([&](const Particle& particle) {
			sf::Vertex point[1];
			point[0].position = sf::Vector2f(particle.m_currentPos.x, particle.m_currentPos.y);
			point[0].color = sf::Color::Red;
			window.draw(point, 1, sf::Points);
		});

		// Finally, display the rendered frame on screen
		window.display();
	}
}