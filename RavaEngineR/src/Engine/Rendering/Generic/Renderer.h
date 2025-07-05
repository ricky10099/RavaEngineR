#pragma once

#include "EnTT/entt.hpp"

namespace RV {
class Scene;
class Camera;
class Renderer {
   public:
	virtual ~Renderer() = default;

	virtual void BeginFrame()                                                = 0;
	//virtual void UpdateEditor(Scene* scene)                                  = 0;
	//virtual void UpdateAnimations(entt::registry& registry)                  = 0;
	virtual void RenderpassEntities(/*entt::registry& registry, Camera camera*/) = 0;
	//virtual void RenderEntities(Scene* scene)                                = 0;
	//virtual void RenderEnv(entt::registry& registry)                         = 0;
	virtual void RenderpassGUI()                                             = 0;
	virtual void EndScene()                                                  = 0;

	static Unique<Renderer> Create(void* window);
};
}  // namespace RV