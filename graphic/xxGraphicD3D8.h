#pragma once

#include "xxGraphic.h"

xxGL_API uint64_t xxCreateInstanceD3D8();
xxGL_API void xxDestroyInstanceD3D8(uint64_t instance);

xxGL_API uint64_t xxCreateDeviceD3D8(uint64_t instance);
xxGL_API void xxDestroyDeviceD3D8(uint64_t device);
xxGL_API void xxResetDeviceD3D8(uint64_t device);
xxGL_API bool xxTestDeviceD3D8(uint64_t device);

xxGL_API uint64_t xxCreateSwapchainD3D8(uint64_t device, void* view, unsigned int width = 0, unsigned int height = 0);
xxGL_API void xxDestroySwapchainD3D8(uint64_t swapchain);
xxGL_API void xxPresentSwapchainD3D8(uint64_t swapchain, void* view);

xxGL_API uint64_t xxGetCommandBufferD3D8(uint64_t device, uint64_t swapchain);
xxGL_API bool xxBeginCommandBufferD3D8(uint64_t commandBuffer);
xxGL_API void xxEndCommandBufferD3D8(uint64_t commandBuffer);
xxGL_API void xxSubmitCommandBufferD3D8(uint64_t commandBuffer);

xxGL_API uint64_t xxCreateRenderPassD3D8(uint64_t device, float r, float g, float b, float a, float depth, unsigned char stencil);
xxGL_API void xxDestroyRenderPassD3D8(uint64_t renderPass);
xxGL_API bool xxBeginRenderPassD3D8(uint64_t commandBuffer, uint64_t renderPass);
xxGL_API void xxEndRenderPassD3D8(uint64_t commandBuffer, uint64_t renderPass);

xxGL_API uint64_t xxCreateBufferD3D8(uint64_t device, unsigned int size, bool indexBuffer = false);
xxGL_API void xxDestroyBufferD3D8(uint64_t buffer);
xxGL_API void* xxMapBufferD3D8(uint64_t buffer);
xxGL_API void xxUnmapBufferD3D8(uint64_t buffer);

xxGL_API uint64_t xxCreateTextureD3D8(uint64_t device, int format, unsigned int width, unsigned int height, unsigned int depth = 1, unsigned int mipmap = 1, unsigned int array = 1);
xxGL_API void xxDestroyTextureD3D8(uint64_t texture);
xxGL_API void* xxMapTextureD3D8(uint64_t texture, unsigned int& stride, unsigned int mipmap = 0, unsigned int array = 0);
xxGL_API void xxUnmapTextureD3D8(uint64_t texture, unsigned int mipmap = 0, unsigned int array = 0);

xxGL_API void xxSetViewportD3D8(uint64_t commandBuffer, int x, int y, int width, int height, float minZ, float maxZ);
xxGL_API void xxSetScissorD3D8(uint64_t commandBuffer, int x, int y, int width, int height);

xxGL_API void xxSetIndexBufferD3D8(uint64_t commandBuffer, uint64_t buffer);
xxGL_API void xxSetVertexBuffersD3D8(uint64_t commandBuffer, const uint64_t* buffers, const int* offsets, const int* strides, int count);
xxGL_API void xxSetFragmentBuffersD3D8(uint64_t commandBuffer, const uint64_t* buffers, const int* offsets, const int* strides, int count);

xxGL_API void xxSetVertexTexturesD3D8(uint64_t commandBuffer, const uint64_t* textures, int count);
xxGL_API void xxSetFragmentTexturesD3D8(uint64_t commandBuffer, const uint64_t* textures, int count);

xxGL_API void xxDrawIndexedD3D8(uint64_t commandBuffer, int indexCount, int instanceCount, int firstIndex, int vertexOffset, int firstInstance);

xxGL_API void xxSetOrthographicTransformD3D8(uint64_t commandBuffer, float left, float right, float top, float bottom);
