#pragma once

#include <variant>
#include "RenderCommand.h"
#include "Constants.h"

struct CmdUpdateActorConstant {
	long long id = 0;
	LocalConstant lc;
};

struct CmdAddActor {
	std::string name;
	int state;
};

// render thread 에서 main으
using GameCmd = std::variant<
	CmdUpdateActorConstant,
	CmdAddActor>;
