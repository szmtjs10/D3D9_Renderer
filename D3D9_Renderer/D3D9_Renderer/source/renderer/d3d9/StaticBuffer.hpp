#pragma once

#include <d3d9.h>

#include "../../utils/ComHelpers.h"

namespace renderer
{
	constexpr int32_t FullBufferLock = 0;

    template<typename T, typename = typename std::enable_if_t<std::is_same<T, IDirect3DVertexBuffer9>::value || std::is_same<T, IDirect3DIndexBuffer9>::value>>
    class StaticBuffer
    {
    public:
		StaticBuffer()
			:m_buffer(nullptr)
		{
		}
		~StaticBuffer() 
		{
			ComSafeRelease(m_buffer);
		}
        
        T* GetRawPtr() { return m_buffer; }
        T** operator&() { return &m_buffer; }

        inline constexpr auto GetBufferDesc() const { return m_bufferDesc.GetBufferDesc(); }

		void AddDataToBuffer(void* data, DWORD lockFlags, UINT dataSize)
		{
            void* bufferData;
            Lock(FullBufferLock, dataSize, &bufferData, lockFlags);
            memcpy(bufferData, data, dataSize);
            Unlock();
		}

	private:
		
		template<typename U>
		struct StaticBufferDesc
		{
			template<typename U, typename = typename std::enable_if_t<std::is_same<U, IDirect3DVertexBuffer9>::value>>
			D3DVERTEXBUFFER_DESC GetBufferDesc()
			{
				D3DVERTEXBUFFER_DESC bufferDesc;
				return bufferDesc;
			}

			template<typename U, typename = typename std::enable_if_t<std::is_same<U, IDirect3DIndexBuffer9>::value>>
			D3DINDEXBUFFER_DESC GetBufferDesc()
			{
				D3DINDEXBUFFER_DESC bufferDesc;
				return bufferDesc;
			}
		};

        inline void Lock(UINT offsetToLock, UINT sizeToLock, void** ppbufferData, DWORD flags) 
        {
            m_buffer->Lock(offsetToLock, sizeToLock, ppbufferData, flags);
        }

		inline void Unlock() { m_buffer->Unlock(); }

        T* m_buffer;
        StaticBufferDesc<T> m_bufferDesc;
    };
}   