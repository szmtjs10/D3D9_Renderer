#pragma once

#include <windows.h>

namespace renderer
{
	class GfxRendererBase
	{
	public:
		GfxRendererBase();
		virtual ~GfxRendererBase();

		//prevent copies of the renderer
		GfxRendererBase(const GfxRendererBase&) = delete;
		GfxRendererBase& operator=(const GfxRendererBase&) = delete;

		virtual void Init(HWND hWindow) = 0;
		virtual void UnInit() = 0;
		virtual void PrepareForRendering() = 0;
		virtual void PreRender() = 0;
		virtual void RenderFrame() = 0;
		virtual void PostRender() = 0;

		//>Events
		virtual void OnDeviceLost() = 0;
		virtual void OnDeviceAvailable() = 0;

		//>Device Handler | long <-> HRESULT
		[[nodiscard]] virtual long CheckDeviceStatus() = 0;
	};
}
