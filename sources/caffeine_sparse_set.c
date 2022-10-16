#include "caffeine_sparse_set.h"
#include "caffeine_assertions.h"
#include "caffeine_default_allocator.h"
#include "caffeine_memory.h"
#include <stdio.h>

void caffeine_sparseset_resize(cff_sparseset* set, uint64_t lenght, AllocatorInterface* allocator) {
	cff_assert_param_not_null(set);
	cff_assert_param_not_zero(lenght);
	
	if (allocator == NULL) allocator = cff_default_allocator_get();

	uint64_t* sparse = NULL;
	uint64_t* dense_index = NULL;
	void* dense = 0;

	if (
		cff_allocator_realloc(allocator, (void*)set->sparse, (size_t)(sizeof(uint64_t) * lenght), 8, (void**) & sparse) &&
		cff_allocator_realloc(allocator, (void*)set->dense_index, (size_t)(sizeof(uint64_t) * lenght), 8, (void**)&dense_index) &&
		cff_allocator_realloc(allocator, (void*)set->dense, (size_t)(set->data_size * lenght), 8, &dense)
		) {
		set->sparse = sparse;
		set->dense_index = dense_index;
		set->dense = (uintptr_t)dense;
		set->lenght = lenght;
	}
	else {
		cff_allocator_free(allocator, sparse, 8);
		cff_allocator_free(allocator, dense_index, 8);
		cff_allocator_free(allocator, dense, 8);
		cff_assert_msg(set != NULL, "[SPARSE SET]: Failed to resize sparse set\n");
	}
}

void caffeine_sparseset_create(cff_sparseset* set, uint64_t lenght, uint64_t data_size, AllocatorInterface* allocator) {
	cff_assert_param_not_null(set);
	cff_assert_param_not_zero(lenght);
	cff_assert_param_not_zero(data_size);
	
	if (allocator == NULL) allocator = cff_default_allocator_get();

	set->count = 0;
	set->lenght = lenght;
	set->data_size = data_size;

	set->sparse = (uint64_t*)cff_allocator_alloc(allocator, (size_t)(sizeof(uint64_t) * lenght), 8);
	set->dense_index = (uint64_t*)cff_allocator_alloc(allocator, (size_t)(sizeof(uint64_t) * lenght), 8);
	set->dense = (uintptr_t)cff_allocator_alloc(allocator, (size_t)(data_size * lenght), 8);

	cff_assert_msg((void*)set->dense != (uintptr_t)NULL, "[SPARSE SET]: Failed to allocate buffer\n");
	cff_assert_msg((void*)set->sparse != (uintptr_t)NULL, "[SPARSE SET]: Failed to allocate buffer\n");
	cff_assert_msg((void*)set->dense_index != (uintptr_t)NULL, "[SPARSE SET]: Failed to allocate buffer\n");
}

void caffeine_sparseset_add(cff_sparseset* set, uint64_t index, uintptr_t data, AllocatorInterface* allocator) {
	cff_assert_param_not_null(set);
	cff_assert_param_not_null(data);
	

	cff_assert_msg((void*)set->dense != NULL, "[SPARSE SET]: Set->Dense can not be NULL\n");
	cff_assert_msg((void*)set->sparse != NULL, "[SPARSE SET]: Set->Sparse can not be NULL\n");
	cff_assert_msg((void*)set->dense_index != NULL, "[SPARSE SET]: Set->DenseIndex can not be NULL\n");
	
	
	if (set->count == set->lenght) caffeine_sparseset_resize(set, set->lenght * 2,allocator);

	cff_assert_param_less(index, set->lenght);

	set->sparse[index] = set->count++;
	set->dense_index[set->sparse[index]] = index;
	cff_memcpy((const void* const)data,(void* const) resolve_ptr(set->dense + (set->sparse[index] * set->data_size)), (size_t)set->data_size, (size_t)set->data_size);
}

void caffeine_sparseset_get(cff_sparseset* set, uint64_t index, uintptr_t data) {
	cff_assert_param_not_null(set);
	cff_assert_param_not_null(data);
	cff_assert_param_less(index, set->lenght);

	cff_assert_msg((void*)set->dense != NULL, "[SPARSE SET]: Set->Dense can not be NULL\n");
	cff_assert_msg((void*)set->sparse != NULL, "[SPARSE SET]: Set->Sparse can not be NULL\n");
	cff_assert_msg((void*)set->dense_index != NULL, "[SPARSE SET]: Set->DenseIndex can not be NULL\n");
	
	cff_memcpy((const void* const)resolve_ptr(set->dense + (set->sparse[index] * set->data_size)), (void* const)data, (size_t)set->data_size, (size_t)set->data_size);
}

void caffeine_sparseset_remove(cff_sparseset* set, uint64_t index) {
	cff_assert_param_not_null(set);
	cff_assert_param_less(index, set->lenght);

	cff_assert_msg((void*)set->dense != NULL, "[SPARSE SET]: Set->Dense can not be NULL\n");
	cff_assert_msg((void*)set->sparse != NULL, "[SPARSE SET]: Set->Sparse can not be NULL\n");
	cff_assert_msg((void*)set->dense_index != NULL, "[SPARSE SET]: Set->DenseIndex can not be NULL\n");
	

	uint64_t last_item = set->count - 1;
	uint64_t to_rem = set->sparse[index];
	set->dense_index[to_rem] = set->dense_index[last_item];
	cff_memcpy((const void* const)resolve_ptr(set->dense + (last_item * set->data_size)), (void* const)resolve_ptr(set->dense + (to_rem * set->data_size)), (size_t)set->data_size, (size_t)set->data_size);
	set->sparse[index] = set->dense_index[last_item];
	set->count--;
}

void caffeine_sparseset_clear(cff_sparseset* set) {
	cff_assert_param_not_null(set);

	set->count = 0;
}

void caffeine_sparseset_free(cff_sparseset* set, AllocatorInterface* allocator) {
	cff_assert_param_not_null(set);

	cff_assert_msg((void*)set->dense != NULL, "[SPARSE SET]: Set->Dense can not be NULL\n");
	cff_assert_msg((void*)set->sparse != NULL, "[SPARSE SET]: Set->Sparse can not be NULL\n");
	cff_assert_msg((void*)set->dense_index != NULL, "[SPARSE SET]: Set->DenseIndex can not be NULL\n");

	if (allocator == NULL) allocator = cff_default_allocator_get();

	cff_allocator_free(allocator, (void*)set->dense, 8);
	cff_allocator_free(allocator, (void*)set->sparse, 8);
	cff_allocator_free(allocator, (void*)set->dense_index, 8);
	cff_memset((void*)resolve_ptr(set), 0, sizeof(cff_sparseset));
	set->count = 0;
	set->data_size = 0;
	set->lenght = 0;
}

uintptr_t caffeine_sparseset_get_dense(cff_sparseset* set) {
	cff_assert_param_not_null(set);

	cff_assert_msg((void*)set->dense != NULL, "[SPARSE SET]: Set->Dense can not be NULL\n");

	return set->dense;
}