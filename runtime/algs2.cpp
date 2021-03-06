#include "lib.h"

#include <queue>


class obj_tag
{
public:
  obj_tag(Obj obj, int tag) : obj(obj), tag(tag)
  {

  }

  Obj obj;
  int tag;
};


bool operator < (const obj_tag &lhs, const obj_tag &rhs)
{
  return comp_objs(lhs.obj, rhs.obj) < 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Obj merge_sets_impl(Obj set_of_sets)
{
  if (set_of_sets == empty_set)
    return empty_set;

  Set *set_of_sets_ptr = get_set_ptr(set_of_sets);
  int size = set_of_sets_ptr->size;

  assert(size > 0);
  
  Obj *sets = set_of_sets_ptr->elems;

  for (int i=1 ; i < size ; i++)
    assert(sets[i] != empty_set);

  // Skipping the empty set (if any), which must be the first one in the array
  if (sets[0] == empty_set)
  {
    sets++;
    size--;
  }

  //## HERE I SHOULD CHECK THAT ALL THE SETS THAT ARE LEFT ARE NOT EMPTY

  if (size == 0)
    return empty_set;

  if (size == 1)
  {
    Obj res = sets[0];
    add_ref(res);
    return res;
  }

  // If we are here, it means that there are at least two non-empty sets to merge

  int elem_count = 0;                               // Total number of elements in all the sets
  int *sizes = new_int_array(size);                 // Sizes of sets
  Obj **elem_arrays = (Obj **) new_ptr_array(size); // Pointers to the elements in each of the sets
  for (int i=0 ; i < size ; i++)
  {
    Set *set_ptr = get_set_ptr(sets[i]);
    assert(set_ptr != 0);
    int array_size = set_ptr->size;
    assert(array_size > 0);
    sizes[i] = array_size;
    elem_arrays[i] = set_ptr->elems;
    elem_count += array_size;
  }

  // Creating and initializing the priority queue
  // by inserting the first element of each set/array
  std::priority_queue<obj_tag> pq;
  for (int i=0 ; i < size ; i++)
  {
    assert(sizes[i] > 0);
    obj_tag ot(elem_arrays[i][0], i);
    pq.push(ot);
  }

  // Cursors for each of the <elem_arrays> defined above.
  // We immediately initialize them to 1, as we've already
  // taken the first object from each of the arrays
  int *idxs = new_int_array(size);
  for (int i=0 ; i < size ; i++)
    idxs[i] = 1;

  // Tentatively allocating the output object (it may have
  // to be reallocated if there are duplicates elements
  // in the input, so that the size of the union is lower
  // then the sums of the sizes of the inputs) and
  // initializing its cursor
  Set *res_set = new_set(elem_count);
  Obj *dest_array = res_set->elems;
  int dest_idx = 0;

  // Main loop: popping elements from the priority queue,
  // storing them in the output array, and inserting the
  // next element (if there is one) from the same array
  // the popped value was from
  for (int done=0 ; done < elem_count ; done++)
  {
    // Popping the value (obj) and the index of its array (array_idx)
    obj_tag ot = pq.top();
    pq.pop();
    int array_idx = ot.tag;
    // If the current element is different from the previous one
    // we store it in the output array and update its counter
    if (dest_idx == 0 or not are_eq(ot.obj, dest_array[dest_idx-1]))
      dest_array[dest_idx++] = ot.obj;

    int src_idx = idxs[array_idx];
    int src_size = sizes[array_idx];
    Obj *src_array = elem_arrays[array_idx];
    // If the array that contained the value just popped
    // has more elements, we take the next one and store
    // it in the priority queue
    if (src_idx < src_size)
    {
      Obj new_obj = src_array[src_idx];
      idxs[array_idx] = src_idx + 1;
      obj_tag ot(new_obj, array_idx);
      pq.push(ot);
    }
  }

  // Reallocating the array if it turns out that its
  // final size is lower than the maximum one
  if (elem_count != dest_idx)
    res_set = shrink_set(res_set, dest_idx);

  // Releasing the temporary arrays
  delete_int_array(sizes, size);
  delete_int_array(idxs, size);
  delete_ptr_array((void **) elem_arrays, size);

  // Calling add_ref() for all the elements of the new array
  vec_add_ref(res_set->elems, dest_idx);

  return make_obj(res_set);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Obj merge_maps_impl(Obj set_of_maps)
{
  if (set_of_maps == empty_set)
    return empty_map;

  Set *set_of_maps_ptr = get_set_ptr(set_of_maps);
  int size = set_of_maps_ptr->size;

  assert(size > 0);

  Obj *maps = set_of_maps_ptr->elems;

  for (int i=1 ; i < size ; i++)
    assert(maps[i] != empty_map);

  // Skipping the empty map (if any), which can only be the first item in the set
  if (maps[0] == empty_map)
  {
    maps++;
    size--;
  }

  //## HERE I SHOULD CHECK THAT ALL REMAINING MAPS ARE NOT EMPTY

  if (size == 0)
    return empty_map;

  if (size == 1)
  {
    Obj res = maps[0];
    add_ref(res);
    return res;
  }

  // If we are here, it means that there are at least two non-empty maps to merge

  int pair_count = 0;                                 // Total number of key/value pairs in all the maps
  int *sizes = new_int_array(size);                   // Sizes of all maps
  Obj **key_arrays = (Obj **) new_ptr_array(size);    // Pointers to the key arrays in each of the maps
  Obj **value_arrays = (Obj **) new_ptr_array(size);  // Pointers to the value arrays in each of the maps
  for (int i=0 ; i < size ; i++)
  {
    Map *map_ptr = get_map_ptr(maps[i]);
    assert(map_ptr != 0);
    int map_size = map_ptr->size;
    assert(map_size > 0);
    sizes[i] = map_size;
    key_arrays[i] = get_key_array_ptr(map_ptr);
    value_arrays[i] = get_value_array_ptr(map_ptr);
    pair_count += map_size;
  }

  // Creating and initializing the priority queue
  // by inserting the first key of each map
  std::priority_queue<obj_tag> pq;
  for (int i=0 ; i < size ; i++)
  {
    assert(sizes[i] > 0);
    obj_tag ot(key_arrays[i][0], i);
    pq.push(ot);
  }

  // Cursors for each of the <key/value_arrays> defined above.
  // We immediately initialize them to 1, as we've already
  // taken the first object from each of the arrays
  int *idxs = new_int_array(size);
  for (int i=0 ; i < size ; i++)
    idxs[i] = 1;

  // Allocating the output object and initializing its cursor
  Map *res_map = new_map(pair_count);
  Obj *dest_key_array = get_key_array_ptr(res_map);
  Obj *dest_value_array = get_value_array_ptr(res_map);
  int dest_idx = 0;

  // Main loop: popping elements from the priority queue,
  // storing them in the output array, and inserting the
  // next element (if there is one) from the same array
  // the popped value was from
  for (int done=0 ; done < pair_count ; done++)
  {
    // Popping the key (obj) and the index of its array (array_idx)
    obj_tag ot = pq.top();
    pq.pop();
    int array_idx = ot.tag;
    int src_idx = idxs[array_idx];
    Obj key = ot.obj;
    Obj value = value_arrays[array_idx][src_idx-1];

    // Checking that this key is not a duplicate
    hard_fail_if(dest_idx > 0 and are_eq(key, dest_key_array[dest_idx-1]), "_merge_: Maps have common keys");

    // Storing key and value in the target arrays and updating the cursor
    dest_key_array[dest_idx] = key;
    dest_value_array[dest_idx] = value;
    dest_idx++;

    int src_size = sizes[array_idx];
    Obj *src_key_array = key_arrays[array_idx];
    // If the map that contained the key just popped has more elements,
    // we take the key of the next one and store it in the priority queue
    if (src_idx < src_size)
    {
      Obj new_key = src_key_array[src_idx];
      idxs[array_idx] = src_idx + 1;
      obj_tag ot(new_key, array_idx);
      pq.push(ot);
    }
  }

  // Releasing the temporary arrays
  delete_int_array(sizes, size);
  delete_int_array(idxs, size);
  delete_ptr_array((void **) key_arrays, size);
  delete_ptr_array((void **) value_arrays, size);

  // Calling add_ref() for all the elements of the new array
  vec_add_ref(dest_key_array, pair_count);
  vec_add_ref(dest_value_array, pair_count);

  return make_obj(res_map);
}
