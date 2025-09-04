#include "PhysicsSystem.hpp"

#include <btBulletDynamicsCommon.h>

#include "core/Coordinator.hpp"
#include "components/TransformComponent.hpp"
#include "components/physics/RigidBodyComponent.hpp"
#include "components/physics/BoxColliderComponent.hpp"
#include "components/physics/SphereColliderComponent.hpp"

bool PhysicsSystem::Initialize()
{
    m_broadphase = new btDbvtBroadphase();
    m_config = new btDefaultCollisionConfiguration();
    m_dispatcher = new btCollisionDispatcher(m_config);
    m_solver = new btSequentialImpulseConstraintSolver();
    m_world = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_config);
    m_world->setGravity(btVector3(0, -9.81f, 0));
    return true;
}

void PhysicsSystem::Shutdown()
{
    for (auto& [e, body] : m_bodies)
    {
        m_world->removeRigidBody(body);
        delete body->getMotionState();
        delete body;
    }
    for (auto& [e, shape] : m_shapes) delete shape;
    m_bodies.clear(); m_shapes.clear();

    delete m_world; m_world=nullptr;
    delete m_solver; m_solver=nullptr;
    delete m_dispatcher; m_dispatcher=nullptr;
    delete m_config; m_config=nullptr;
    delete m_broadphase; m_broadphase=nullptr;
}

void PhysicsSystem::EnsureBody(unsigned entity)
{
    if (m_bodies.count(entity)) return;
    if (!gCoordinator.HasComponent<RigidBodyComponent>(entity) || !gCoordinator.HasComponent<TransformComponent>(entity)) return;

    auto& rb = gCoordinator.GetComponent<RigidBodyComponent>(entity);
    auto& tr = gCoordinator.GetComponent<TransformComponent>(entity);

    btCollisionShape* shape = nullptr;
    if (gCoordinator.HasComponent<BoxColliderComponent>(entity))
    {
        auto& box = gCoordinator.GetComponent<BoxColliderComponent>(entity);
        shape = new btBoxShape(btVector3(box.halfExtents[0], box.halfExtents[1], box.halfExtents[2]));
    }
    else if (gCoordinator.HasComponent<SphereColliderComponent>(entity))
    {
        auto& sph = gCoordinator.GetComponent<SphereColliderComponent>(entity);
        shape = new btSphereShape(btScalar(sph.radius));
    }
    else
    {
        shape = new btBoxShape(btVector3(0.5f,0.5f,0.5f));
    }
    m_shapes[entity] = shape;

    btTransform start; start.setIdentity();
    start.setOrigin(btVector3(tr.position[0], tr.position[1], tr.position[2]));
    btDefaultMotionState* motion = new btDefaultMotionState(start);

    btScalar mass = (rb.type == RigidBodyType::Dynamic) ? btScalar(rb.mass) : btScalar(0.0f);
    btVector3 inertia(0,0,0);
    if (mass != 0.0f) shape->calculateLocalInertia(mass, inertia);
    btRigidBody::btRigidBodyConstructionInfo info(mass, motion, shape, inertia);
    info.m_friction = rb.friction; info.m_restitution = rb.restitution;
    btRigidBody* body = new btRigidBody(info);
    if (rb.type == RigidBodyType::Kinematic)
    {
        body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        body->setActivationState(DISABLE_DEACTIVATION);
    }
    m_world->addRigidBody(body);
    m_bodies[entity] = body;
    rb.native = reinterpret_cast<std::uintptr_t>(body);
}

void PhysicsSystem::Update(float deltaTime)
{
    // create bodies for new entities
    auto entities = gCoordinator.GetEntitiesWithComponents<RigidBodyComponent, TransformComponent>();
    for (auto e : entities) EnsureBody(e);

    // sync kinematics from transforms
    for (auto e : entities)
    {
        auto& rb = gCoordinator.GetComponent<RigidBodyComponent>(e);
        if (rb.type == RigidBodyType::Kinematic)
        {
            auto& tr = gCoordinator.GetComponent<TransformComponent>(e);
            if (auto it = m_bodies.find(e); it != m_bodies.end())
            {
                btTransform t; t.setIdentity(); t.setOrigin(btVector3(tr.position[0], tr.position[1], tr.position[2]));
                it->second->getMotionState()->setWorldTransform(t);
                it->second->setWorldTransform(t);
            }
        }
    }

    m_world->stepSimulation(deltaTime, 4);

    // write back dynamic transforms
    for (auto e : entities)
    {
        auto& rb = gCoordinator.GetComponent<RigidBodyComponent>(e);
        if (rb.type == RigidBodyType::Dynamic)
        {
            if (auto it = m_bodies.find(e); it != m_bodies.end())
            {
                btTransform t; it->second->getMotionState()->getWorldTransform(t);
                auto& tr = gCoordinator.GetComponent<TransformComponent>(e);
                auto o = t.getOrigin();
                tr.position = { (float)o.x(), (float)o.y(), (float)o.z() };
            }
        }
    }
}

