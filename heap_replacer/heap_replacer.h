#pragma once

#include "util.h"

#include "default_heap_manager.h"
#include "memory_pool_manager.h"
#include "scrap_heap_manager.h"

memory_pools_manager* mpm;
default_heap_manager* dhm;

void*	__fastcall	nvhr_alloc(size_t size, bool zero)
{
	if (size < 4) { size = 4; }
	void* address = nullptr;
	if (size <= 2 * KB)
	{
		if (address = zero ? mpm->calloc(size) : mpm->malloc(size)) { return address; }
	}
	if (address = zero ? dhm->calloc(size) : dhm->malloc(size)) { return address; }
	return nullptr;
}

void*	__fastcall	nvhr_malloc(size_t size)
{
	return nvhr_alloc(size, false);
}

void*	__fastcall	nvhr_calloc(size_t count, size_t size)
{
	return nvhr_alloc(count * size, true);
}

size_t	__fastcall	nvhr_mem_size(void* address)
{
	size_t size;
	if (size = mpm->mem_size(address)) { return size; }
	if (size = dhm->mem_size(address)) { return size; }
	return 0;
}

void	__fastcall	nvhr_free(void* address)
{
	if (!address) { return; }
	if (mpm->free(address)) { return; }
	if (dhm->free(address)) { return; }
}

void*	__fastcall	nvhr_realloc(void* address, size_t size)
{
	void* new_address = nvhr_malloc(size);
	size_t old_size = nvhr_mem_size(address);
	size = (size < old_size) ? size : old_size;
	memcpy(new_address, address, size);
	nvhr_free(address);
	return new_address;
}

void*	__fastcall	game_heap_allocate(TtFParam(void* self, size_t size))
{
	return nvhr_malloc(size);
}

void*	__fastcall	game_heap_reallocate(TtFParam(void* self, void* address, size_t size))
{
	return nvhr_realloc(address, size);
}

void	__fastcall	game_heap_free(TtFParam(void* self, void* address))
{
	nvhr_free(address);
}

void*	__cdecl		crt_malloc(size_t size)
{
	return nvhr_malloc(size);
}

void*	__cdecl		crt_calloc(size_t count, size_t size)
{
	return nvhr_calloc(count, size);
}

void*	__cdecl		crt_realloc(void* address, size_t size)
{
	return nvhr_realloc(address, size);
}

void	__cdecl		crt_free(void* address)
{
	nvhr_free(address);
}

size_t	__cdecl		crt_msize(void* address)
{
	return nvhr_mem_size(address);
}

/*void patch_old_NVSE()
{
	HMODULE base = GetModuleHandleA("nvse_1_4.dll");	
	if (!base) { return; }
	printf("NVHR - Found NVSE at %p\n", base);
	BYTE* address;
	address = (BYTE*)((DWORD)base + 0x38887);
	if (*address == 0x4)
	{
		printf("NVHR - Patched stable NVSE\n");
		patch_BYTE((DWORD)address, 0x8);
		return;
	}
	address = (BYTE*)((DWORD)base + 0x24FF7);
	if (*address == 0x4)
	{
		printf("NVHR - Patched unsupported releasefast NVSE\n");
		patch_BYTE((DWORD)address, 0x8);
		return;
	}
}*/

void apply_heap_hooks()
{

	mpm = new memory_pools_manager();
	dhm = new default_heap_manager();


	patch_jmp(0xECD1C7, &crt_malloc);
	patch_jmp(0xED0CDF, &crt_malloc);
	patch_jmp(0xEDDD7D, &crt_calloc);
	patch_jmp(0xED0D24, &crt_calloc);
	patch_jmp(0xECCF5D, &crt_realloc);
	patch_jmp(0xED0D70, &crt_realloc);
	patch_jmp(0xECD291, &crt_free);
	patch_jmp(0xECD31F, &crt_msize);


	patch_jmp(0xAA3E40, &game_heap_allocate);
	patch_jmp(0xAA4150, &game_heap_reallocate);
	patch_jmp(0xAA4060, &game_heap_free);

	patch_ret(0x866E00);
	patch_BYTE(0xAA39D9, 0);

	patch_ret(0x866770);
	patch_ret(0xAA7030);
	patch_ret(0xAA7290);


	patch_jmp(0xAA47E0, &shm_create_mt);
	patch_jmp(0xAA42E0, &shm_get_scrap_heap);

	patch_jmp(0x866D10, &shm_get_singleton);

	patch_jmp(0x40FBF0, &enter_light_critical_section);
	patch_jmp(0x40FBA0, &leave_light_critical_section);

	patch_jmp(0xAA5E30, &sh_grow);
	patch_jmp(0xAA5E90, &sh_shrink);

	patch_jmp(0xAA5860, &shm_ctor);
	patch_jmp(0xAA58D0, &shm_init);
	patch_jmp(0xAA5F50, &shm_swap_buffers);
	patch_jmp(0xAA5EC0, &shm_create_buffer);
	patch_jmp(0xAA59B0, &shm_request_buffer);
	patch_jmp(0xAA5F30, &shm_free_buffer);
	patch_jmp(0xAA5B70, &shm_release_buffer);
	patch_jmp(0xAA5C80, &shm_free_all_buffers);

	patch_jmp(0xAA57B0, &sh_init);
	patch_jmp(0xAA53F0, &sh_init_0x10000);
	patch_jmp(0xAA5410, &sh_init_var);

	patch_jmp(0xAA54A0, &sh_add_chunk);
	patch_jmp(0xAA5610, &sh_remove_chunk);


	patch_nops(0xAA3060, 5);

	patch_bytes(0xC42EA4, (BYTE*)"\xEB", 1);
	patch_bytes(0x86C563, (BYTE*)"\xEB\x12", 2);
	patch_bytes(0xEC16F8, (BYTE*)"\xEB\x0F", 2);

	/*patch_call(0x86CF64, &patch_old_NVSE);
	patch_nops(0x86CF69, 2);*/

	printf("NVHR - Heap hooks applied.\n");

}
