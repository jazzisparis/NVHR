#pragma once

#include "memory_pool.h"

#include "util.h"

#define POOL_COUNT 10
#define POOL_ARRAY_SIZE 0xFFFFFFFF / (POOL_ALIGNMENT) + 1

class memory_pools_manager
{

private:

	memory_pool* pools_by_size[POOL_ARRAY_SIZE];
	memory_pool* pools_by_addr[POOL_ARRAY_SIZE];

public:

	memory_pools_manager()
	{
		memset(this->pools_by_size, 0, (POOL_ARRAY_SIZE) * sizeof(memory_pool*));
		memset(this->pools_by_addr, 0, (POOL_ARRAY_SIZE) * sizeof(memory_pool*));
		this->init_all_pools();
	}

	~memory_pools_manager()
	{

	}

private:

	void init_all_pools()
	{
		struct pool_data { size_t item_size; size_t pool_size; };

		pool_data pools[POOL_COUNT] =
		{
			{ 4		, 0x01000000u },
			{ 8		, 0x04000000u },
			{ 16	, 0x04000000u },
			{ 32	, 0x10000000u },
			{ 64	, 0x04000000u },
			{ 128	, 0x10000000u },
			{ 256	, 0x08000000u },
			{ 512	, 0x04000000u },
			{ 1024	, 0x08000000u },
			{ 2048	, 0x08000000u },
		};

		for (size_t i = 0; i < POOL_COUNT; i++)
		{
			pool_data* pd = &pools[i];
			memory_pool* pool = new memory_pool(pd->item_size, pd->pool_size);
			void* address = pool->memory_pool_init();
			this->pools_by_size[(pd->item_size >> 2) - 1] = pool;
			for (size_t j = 0; j < ((pd->pool_size + (POOL_ALIGNMENT - 1)) / POOL_ALIGNMENT); j++)
			{
				this->pools_by_addr[((uintptr_t)address / POOL_ALIGNMENT) + j] = pool;
			}
		}
	}

	size_t next_power_of_2(size_t size)
	{
		if (size & (size - 1))
		{
			size--;
			size |= size >> 1;
			size |= size >> 2;
			size |= size >> 4;
			size |= size >> 8;
			size |= size >> 16;
			size++;
		}
		return size;
	}
	
	memory_pool* pool_from_size(size_t size)
	{
		size = this->next_power_of_2(size);
		return this->pools_by_size[(size >> 2) - 1];
	}

	memory_pool* pool_from_addr(void* address)
	{
		return this->pools_by_addr[(uintptr_t)address / POOL_ALIGNMENT];
	}

public:

	void* malloc(size_t size)
	{
		void* address = nullptr;
		while (size <= 2048)
		{
			memory_pool* pool = this->pool_from_size(size);
			if (!pool) { return nullptr; }
			if (address = pool->malloc()) {	return address; }
			size <<= 1;
		}
		return address;
	}

	void* calloc(size_t size)
	{
		void* address = nullptr;
		while (size <= 2048)
		{
			memory_pool* pool = this->pool_from_size(size);
			if (!pool) { return nullptr; }
			if (address = pool->calloc()) { return address; }
			size <<= 1;
		}
		return address;
	}

	size_t mem_size(void* address)
	{
		memory_pool* pool = this->pool_from_addr(address);
		if (!pool) { return 0; }
		return pool->mem_size(address);
	}

	bool free(void* address)
	{	
		memory_pool* pool = pool_from_addr(address);
		if (!pool) { return false; }
		return pool->free(address);
	}

};
