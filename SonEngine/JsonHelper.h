#pragma once

#include <nlohmann/json.hpp>
#include "PhysXMode.h"
#include "directxtk12\SimpleMath.h"

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