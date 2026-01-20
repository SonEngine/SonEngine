#pragma once

#include <nlohmann/json.hpp>
#include "PhysXMode.h"
#include "directxtk12\SimpleMath.h"

enum  ActorType {
    AT_Actor,
    AT_ATriggerBox,
    AT_AMovingPlatform,
    AT_PointCloud,
    AT_CubeMap,
    AT_Dot,
    AT_Light,
    AT_Door,
    AT_SkinnedMesh,
};

NLOHMANN_JSON_SERIALIZE_ENUM(ActorType, {
    {ActorType::AT_Actor, "Actor"},
    {ActorType::AT_ATriggerBox, "ATriggerBox"},
    {ActorType::AT_AMovingPlatform, "AMovingPlatform"},
    {ActorType::AT_PointCloud, "APointCloud"},
    {ActorType::AT_CubeMap, "ACubeMap"},
    {ActorType::AT_Dot, "ADot"},
    {ActorType::AT_Light, "ALight"},
    {ActorType::AT_Door, "ADoor"},
    {ActorType::AT_SkinnedMesh, "ASkinnedMesh"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(PhysXMode, {
    {PhysXMode::PM_Default,    "PM_Default"},
    {PhysXMode::PM_Dynamic,   "PM_Dynamic"},
    {PhysXMode::PM_Kinematic, "PM_Kinematic"},
    {PhysXMode::PM_Trigger, "PM_Trigger"},
    })

static DirectX::SimpleMath::Vector3 ParseVec3(const nlohmann::json & j)
{
    DirectX::SimpleMath::Vector3 v;
    if (j.is_array() && j.size() == 3)
    {
        v.x = j[0].get<float>();
        v.y = j[1].get<float>();
        v.z = j[2].get<float>();
    }

    return v;
}

static DirectX::SimpleMath::Vector4 ParseVec4(const nlohmann::json& j)
{
    DirectX::SimpleMath::Vector4 v;
    if (j.is_array() && j.size() == 4)
    {
        v.x = j[0].get<float>();
        v.y = j[1].get<float>();
        v.z = j[2].get<float>();
        v.w = j[3].get<float>();
    }

    return v;
}