// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "stdafx.h"
#include "AudioImplCVars.h"
#include <CrySystem/IConsole.h>

using namespace CryAudio::Impl::PortAudio;

//////////////////////////////////////////////////////////////////////////
void CAudioImplCVars::RegisterVariables()
{
}

//////////////////////////////////////////////////////////////////////////
void CAudioImplCVars::UnregisterVariables()
{
	IConsole* const pConsole = gEnv->pConsole;

	if (pConsole != nullptr)
	{
	}
}
