#include "caffeine_vector.h"
#include "caffeine_container.h"
#include "caffeine_memory.h"
#include "caffeine_assertions.h"


cff_err_e caffeine_vector_create(cff_vector* vector, uint64_t data_size, uint64_t lenght, AllocatorInterface* allocator) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_zero(lenght);
	cff_assert_param_not_zero(data_size);

	vector->count = 0;
	vector->lenght = lenght;

	return caffeine_container_create((cff_container*)vector, data_size, lenght, allocator);
}

cff_err_e caffeine_vector_resize(cff_vector* vector, uint64_t lenght, AllocatorInterface* allocator) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_zero(lenght);

	vector->lenght = lenght;

	return caffeine_container_resize((cff_container*)vector, lenght, allocator);
}

void caffeine_vector_get(cff_vector* vector, uint64_t index, uintptr_t out) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(out);
	cff_assert_param_less(index, vector->count);

	caffeine_container_get((cff_container*)vector, index, out, vector->lenght);
}

void caffeine_vector_set(cff_vector* vector, uintptr_t data_ptr, uint64_t index) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(data_ptr);
	cff_assert_param_less(index, vector->lenght);

	caffeine_container_set((cff_container*)vector, index, data_ptr, vector->lenght);
	if (vector->count < index + 1) vector->count = index + 1;
}

void caffeine_vector_insert(cff_vector* vector, uintptr_t data_ptr, uint64_t index) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(data_ptr);
	cff_assert_param_less_eq(index, vector->count);

	if (vector->count == 0 && index == 0)
		caffeine_vector_set(vector, data_ptr, 0);
	else {
		caffeine_container_insert((cff_container*)vector, index, data_ptr, vector->count + 1);
		vector->count++;
	}
}

cff_err_e caffeine_vector_remove(cff_vector* vector, uint64_t index, AllocatorInterface* allocator) {
	cff_assert_param_not_null(vector);
	cff_assert_param_less(index, vector->count);

	if (index != vector->count - 1) caffeine_container_remove((cff_container*)vector, index, vector->count);
	vector->count--;
	if (vector->count < vector->lenght / 2) return caffeine_vector_resize(vector, vector->lenght / 2, allocator);

	return CFF_NONE_ERR;
}

cff_err_e caffeine_vector_copy(cff_vector* vector, cff_vector* to, uint64_t start, uint64_t end, AllocatorInterface* allocator) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(to);
	cff_assert_param_less(end, vector->count);
	cff_assert_param_less(start, end);

	cff_err_e err = caffeine_container_copy((cff_container*)vector, (cff_container*)to, start, end, to->lenght, &to->lenght, allocator);
	to->count = to->lenght;

	return err;
}

cff_err_e caffeine_vector_clone(cff_vector* vector, cff_vector* to, AllocatorInterface* allocator) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(to);

	cff_err_e err = caffeine_container_clone((cff_container*)vector, (cff_container*)to, to->lenght, allocator);

	if (err == CFF_NONE_ERR)to->count = vector->count;

	return err;
}

void caffeine_vector_fill(cff_vector* vector, uintptr_t data_ptr) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(data_ptr);

	caffeine_container_fill((cff_container*)vector, data_ptr, vector->lenght);
	vector->count = vector->lenght;
}

cff_err_e caffeine_vector_join(cff_vector* vector, cff_vector* from, AllocatorInterface* allocator) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(from);

	if (vector->lenght < vector->count + from->count) {
		cff_err_e err = caffeine_vector_resize(vector, vector->count + from->count, allocator);
		if (err != CFF_NONE_ERR) return err;
	}
	uint64_t size = from->count * from->data_size;
	cff_memcpy((const void* const)from->buffer, (void* const)resolve_ptr(vector->buffer + (vector->count * vector->data_size)), (size_t)size, (size_t)((vector->lenght * vector->data_size) - (vector->count * vector->data_size)));
	vector->count += from->count;

	return CFF_NONE_ERR;
}

cff_err_e caffeine_vector_reverse(cff_vector* vector) {
	cff_assert_param_not_null(vector);

	return caffeine_container_reverse((cff_container*)vector, vector->count);
}

cff_err_e caffeine_vector_filter(cff_vector* vector, filter_fn func, cff_vector* filter_result, AllocatorInterface* allocator) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(func);
	cff_assert_param_not_null(filter_result);

	cff_err_e err = caffeine_container_filter((cff_container*)vector, func, (cff_container*)filter_result, vector->count, &filter_result->lenght, allocator);
	if (err == CFF_NONE_ERR) filter_result->count = filter_result->lenght;

	return err;
}

cff_err_e caffeine_vector_push_back(cff_vector* vector, uintptr_t data_ptr, AllocatorInterface* allocator) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(data_ptr);

	if (vector->count == vector->lenght) {
		cff_err_e err = caffeine_vector_resize(vector, vector->lenght * 2, allocator);
		if (err != CFF_NONE_ERR) return err;
	}

	caffeine_vector_set(vector, data_ptr, vector->count);
	return CFF_NONE_ERR;
}

cff_err_e caffeine_vector_push_front(cff_vector* vector, uintptr_t data_ptr, AllocatorInterface* allocator) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(data_ptr);

	if (vector->count == vector->lenght) {
		cff_err_e err = caffeine_vector_resize(vector, vector->lenght * 2, allocator);
		if (err != CFF_NONE_ERR) return err;
	}

	caffeine_vector_insert(vector, data_ptr, 0);
	return CFF_NONE_ERR;
}

cff_err_e caffeine_vector_pop_back(cff_vector* vector, uintptr_t data_ptr, AllocatorInterface* allocator) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(data_ptr);

	caffeine_vector_get(vector, vector->count - 1, data_ptr);
	vector->count--;

	if (vector->count < vector->lenght / 2) return caffeine_vector_resize(vector, vector->lenght / 2, allocator);
	return CFF_NONE_ERR;
}

cff_err_e caffeine_vector_pop_front(cff_vector* vector, uintptr_t data_ptr, AllocatorInterface* allocator) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(data_ptr);

	caffeine_vector_get(vector, 0, data_ptr);
	return caffeine_vector_remove(vector, 0, allocator);
}

cff_err_e caffeine_vector_map(cff_vector* vector, map_fn func, cff_vector* map_result, uint64_t result_data_size, AllocatorInterface* allocator) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(func);
	cff_assert_param_not_null(map_result);
	cff_assert_param_not_zero(result_data_size);

	return caffeine_container_map((cff_container*)vector, func, (cff_container*)map_result, result_data_size, vector->count, allocator);
}

void caffeine_vector_foreach(cff_vector* vector, foreach_fn func) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(func);

	caffeine_container_foreach((cff_container*)vector, func, vector->count);
}

void caffeine_vector_sort(cff_vector* vector, comparer_fn predicate) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(predicate);

	caffeine_container_sort((cff_container*)vector, predicate, vector->count);
}

void caffeine_vector_free(cff_vector* vector, AllocatorInterface* allocator) {
	cff_assert_param_not_null(vector);

	caffeine_container_free((cff_container*)vector, allocator);
	vector->count = 0;
	vector->lenght = 0;
}

void caffeine_vector_clear(cff_vector* vector) {
	cff_assert_param_not_null(vector);

	vector->count = 0;
}

uint8_t caffeine_vector_equal(cff_vector* vector, cff_vector* other) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(other);

	return caffeine_container_equal((cff_container*)vector, (cff_container*)other, vector->count);
}

uint8_t caffeine_vector_find(cff_vector* vector, uintptr_t data_ptr, uint64_t* found) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(data_ptr);
	cff_assert_param_not_null(found);

	return caffeine_container_find((cff_container*)vector, data_ptr, found, vector->count);
}

uint8_t caffeine_vector_find_cmp(cff_vector* vector, uintptr_t data_ptr, uint64_t* found, comparer_fn predicate) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(data_ptr);
	cff_assert_param_not_null(found);
	cff_assert_param_not_null(predicate);

	return caffeine_container_find_cmp((cff_container*)vector, data_ptr, found, predicate, vector->count);
}

uint64_t caffeine_vector_count(cff_vector* vector, uintptr_t data_ptr) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(data_ptr);

	return caffeine_container_count((cff_container*)vector, data_ptr, vector->count);
}

uint64_t caffeine_vector_count_cmp(cff_vector* vector, uintptr_t data_ptr, comparer_fn predicate) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(data_ptr);
	cff_assert_param_not_null(predicate);

	return caffeine_container_count_cmp((cff_container*)vector, data_ptr, predicate, vector->count);
}

uint8_t caffeine_vector_any(cff_vector* vector, uintptr_t data_ptr) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(data_ptr);

	return caffeine_container_any((cff_container*)vector, data_ptr, vector->count);
}

uint8_t caffeine_vector_any_cmp(cff_vector* vector, uintptr_t data_ptr, comparer_fn predicate) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(data_ptr);
	cff_assert_param_not_null(predicate);

	return caffeine_container_any_cmp((cff_container*)vector, data_ptr, predicate, vector->count);
}

uint8_t caffeine_vector_all(cff_vector* vector, uintptr_t data_ptr) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(data_ptr);

	return caffeine_container_all((cff_container*)vector, data_ptr, vector->count);
}

uint8_t caffeine_vector_all_cmp(cff_vector* vector, uintptr_t data_ptr, comparer_fn predicate) {
	cff_assert_param_not_null(vector);
	cff_assert_param_not_null(data_ptr);
	cff_assert_param_not_null(predicate);

	return caffeine_container_all_cmp((cff_container*)vector, data_ptr, predicate, vector->count);
}

