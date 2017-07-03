//----------------------------------------------------------
//
//   SA:MP Multiplayer Modification For GTA:SA
//   Copyright 2004-2006 SA:MP team
//
//----------------------------------------------------------

#pragma once

namespace GUI
{

	class IRenderable
	{
	public:
		virtual void OnRender() = 0;
		virtual void OnDestroyDevice() = 0;
		virtual void OnRestoreDevice() = 0;
	};

}