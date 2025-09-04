#pragma once
#include "core/System.hpp"
#include <unordered_map>
class btDiscreteDynamicsWorld; class btBroadphaseInterface; class btDefaultCollisionConfiguration; class btCollisionDispatcher; class btSequentialImpulseConstraintSolver; class btRigidBody; class btCollisionShape;

class PhysicsSystem : public System
{
public:
    bool Initialize();
    void Shutdown() override;
    void Update(float deltaTime) override;
    const char* GetName() const override { return "PhysicsSystem"; }

private:
    btBroadphaseInterface* m_broadphase{nullptr};
    btDefaultCollisionConfiguration* m_config{nullptr};
    btCollisionDispatcher* m_dispatcher{nullptr};
    btSequentialImpulseConstraintSolver* m_solver{nullptr};
    btDiscreteDynamicsWorld* m_world{nullptr};

    std::unordered_map<unsigned, btRigidBody*> m_bodies; // entity -> body
    std::unordered_map<unsigned, btCollisionShape*> m_shapes; // entity -> shape

    void EnsureBody(unsigned entity);
};

