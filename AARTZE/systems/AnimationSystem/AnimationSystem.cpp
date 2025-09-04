#include "AnimationSystem.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>

#include "core/Coordinator.hpp"
#include "components/animation/AnimationBlendComponent.hpp"
#include "components/AnimationComponent.hpp"
#include "components/SkeletonComponent.hpp"

static glm::vec3 sampleVec3(const std::vector<Keyframe<glm::vec3>>& keys, float t, float duration)
{
    if (keys.empty()) return glm::vec3(0);
    if (t <= keys.front().time) return keys.front().value;
    if (t >= keys.back().time) return keys.back().value;
    for (size_t i = 0; i + 1 < keys.size(); ++i)
    {
        if (t >= keys[i].time && t <= keys[i+1].time)
        {
            float a = (t - keys[i].time) / (keys[i+1].time - keys[i].time);
            return glm::lerp(keys[i].value, keys[i+1].value, a);
        }
    }
    return keys.back().value;
}

static glm::quat sampleQuat(const std::vector<Keyframe<glm::quat>>& keys, float t, float duration)
{
    if (keys.empty()) return glm::quat(1,0,0,0);
    if (t <= keys.front().time) return keys.front().value;
    if (t >= keys.back().time) return keys.back().value;
    for (size_t i = 0; i + 1 < keys.size(); ++i)
    {
        if (t >= keys[i].time && t <= keys[i+1].time)
        {
            float a = (t - keys[i].time) / (keys[i+1].time - keys[i].time);
            return glm::slerp(keys[i].value, keys[i+1].value, a);
        }
    }
    return keys.back().value;
}

void AnimationSystem::Update(float dt)
{
    auto entities = gCoordinator.GetEntitiesWithComponents<AnimationComponent, SkeletonComponent, AnimationBlendComponent>();
    for (auto e : entities)
    {
        auto& anim = gCoordinator.GetComponent<AnimationComponent>(e);
        auto& blend = gCoordinator.GetComponent<AnimationBlendComponent>(e);
        auto& skel = gCoordinator.GetComponent<SkeletonComponent>(e);

        if (blend.clipA < 0 && blend.clipB < 0) continue;
        blend.time += dt * blend.speed;

        const AnimationClip* A = nullptr; const AnimationClip* B = nullptr;
        if (blend.clipA >= 0 && blend.clipA < (int)anim.clips.size()) A = &anim.clips[blend.clipA];
        if (blend.clipB >= 0 && blend.clipB < (int)anim.clips.size()) B = &anim.clips[blend.clipB];
        float tA = 0.0f, tB = 0.0f;
        if (A) tA = blend.loop && A->duration>0 ? fmodf(blend.time, A->duration) : glm::clamp(blend.time, 0.0f, A?A->duration:0.0f);
        if (B) tB = blend.loop && B->duration>0 ? fmodf(blend.time, B->duration) : glm::clamp(blend.time, 0.0f, B?B->duration:0.0f);

        size_t boneCount = skel.bones.size();
        if (skel.poseMatrices.size() != boneCount) skel.poseMatrices.assign(boneCount, glm::mat4(1.0f));

        for (size_t i = 0; i < boneCount; ++i)
        {
            glm::vec3 posA(0), posB(0), sclA(1), sclB(1);
            glm::quat rotA(1,0,0,0), rotB(1,0,0,0);
            if (A && i < A->channels.size())
            {
                const auto& ch = A->channels[i];
                posA = sampleVec3(ch.positions, tA, A->duration);
                rotA = sampleQuat(ch.rotations, tA, A->duration);
                sclA = sampleVec3(ch.scalings,  tA, A->duration);
            }
            if (B && i < B->channels.size())
            {
                const auto& ch = B->channels[i];
                posB = sampleVec3(ch.positions, tB, B->duration);
                rotB = sampleQuat(ch.rotations, tB, B->duration);
                sclB = sampleVec3(ch.scalings,  tB, B->duration);
            }
            glm::vec3 pos = glm::lerp(posA, posB, blend.alpha);
            glm::quat rot = glm::slerp(rotA, rotB, blend.alpha);
            glm::vec3 scl = glm::lerp(sclA, sclB, blend.alpha);

            glm::mat4 T(1.0f);
            T[3][0]=pos.x; T[3][1]=pos.y; T[3][2]=pos.z;
            glm::mat4 R = glm::mat4_cast(rot);
            glm::mat4 S(1.0f); S[0][0]=scl.x; S[1][1]=scl.y; S[2][2]=scl.z;
            skel.poseMatrices[i] = T * R * S;
        }
    }
}

