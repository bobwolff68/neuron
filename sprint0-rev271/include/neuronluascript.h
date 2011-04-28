// Our Neuron version of OOLUA::Script.
// This version allows 
// - a pre-prepared lua_State* to be given to the script.
// - Return values from a Script::call() to be handed back without using pull2cpp()
//
// If an initial lua_State is given, then startup was handled outside Script...so cleanup
// must be left to the outside as well. 
#include <oolua.h>

class NeuronLuaScript : public OOLUA::Script {
public:
  lua_State* origL;
  
  NeuronLuaScript(lua_State* initL=NULL)
  {
	origL = NULL;
	
	if (initL)
	{
		origL = m_lua;
		m_lua = initL;
		
		call.bind_script(m_lua);
		OOLUA::setup_user_lua_state(m_lua);
	}
  }
  
  virtual ~NeuronLuaScript()
  {
	// Switch out our overridden initL from the constructor and allow the ~Script() to close up
	// shop on the m_lua pointer.
	if (origL)
		m_lua = origL;
  }
  
  // Each ReturnValue* returns true on success or false if there's a problem pulling the data off the stack
  bool ReturnValue(bool& value) { return OOLUA::pull2cpp(m_lua, value); };
  
  template <typename T>
  inline bool ReturnValue(T& value) { return OOLUA::pull2cpp(m_lua, value); };
  
  inline bool ReturnValue(std::string& value) { return OOLUA::pull2cpp(m_lua, value); };
  
};