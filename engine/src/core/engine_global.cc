/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Engine global holder class.
 */

#include "core/engine_global.h"

/** List of initialized globals in destruction order. */
std::list<EngineGlobalBase *> EngineGlobalBase::m_globals;

/** Destructor, verifies the object has been destroyed. */
EngineGlobalBase::~EngineGlobalBase() {
	orionAssert(!m_initialized);
}

/** Destroy all global objects in correct order. */
void EngineGlobalBase::destroyAll() {
	/* Destruction order is last first, iterate in reverse. */
	while(!m_globals.empty()) {
		EngineGlobalBase *global = m_globals.back();
		global->destroy();
		global->m_initialized = false;
		m_globals.pop_back();
	}
}

/** Register a global in the list. */
void EngineGlobalBase::init() {
	/* Check we haven't already been initialized. FIXME: thread safety. */
	orionAssert(!m_initialized);
	m_initialized = true;

	m_globals.push_back(this);
}
