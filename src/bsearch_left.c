/*
 * A generic implementation of binary search for the Linux kernel,
 * modified to return the index of the greatest element smaller than the key
 *
 * Copyright (C) 2008-2009 Ksplice, Inc.
 * Author: Tim Abbott <tabbott@ksplice.com>
 * Copyright (C) 2015 Davide Kirchner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2.
 */

#include <assert.h>

#include "bsearch_left.h"

/*
 * bsearch_left - binary search a sorted array of elements, returning the index
 * of the matched element, OR the rightmost index with an element less than
 * the given key, OR zero.
 * Note that zero could be returned in either the second or third case.
 *
 * @key: pointer to item being searched for
 * @base: pointer to first element to search
 * @num: number of elements
 * @size: size of each element
 * @cmp: pointer to comparison function
 *
 * This function does a binary search on the given array.  The
 * contents of the array should already be in ascending sorted order
 * under the provided comparison function.
 *
 * Note that the key need not have the same type as the elements in
 * the array, e.g. key could be a string and the comparison function
 * could compare the string with the struct's name field.  However, if
 * the key and elements in the array are of the same type, you can use
 * the same comparison function for both sort() and bsearch().
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-arith"
size_t bsearch_left(const void *key, const void *base, size_t num, size_t size,
               int (*cmp)(const void *key, const void *elt))
{
        size_t start = 0, end = num;
        int result;

        while (start < end) {
                size_t mid = start + (end - start) / 2;

                result = cmp(key, base + mid * size);
                if (result < 0)
                        end = mid;
                else if (result > 0)
                        start = mid + 1;
                else
                        return mid;
        }

        assert(start == end);
        if (start == 0) return 0;
        return start - 1;
}
#pragma GCC diagnostic pop

/*
int arr1[] = {0, 1, 2, 3, 4, 5};
int arr2[] = {0, 1, 2, 3, 4};

int mycmp (const void *vkey, const void *velt) {
  const float *key = vkey;
  const int *elt = velt;
  if (*key < *elt) return -1;
  if (*key > *elt) return 1;
  return 0;
}

int main() {
  float k;
  
  k = 1.4;
  assert(bsearch_left(&k, arr1, 6, sizeof(int), mycmp) == 1);
  assert(bsearch_left(&k, arr2, 5, sizeof(int), mycmp) == 1);

  k = -2;
  assert(bsearch_left(&k, arr1, 6, sizeof(int), mycmp) == 0);
  assert(bsearch_left(&k, arr2, 5, sizeof(int), mycmp) == 0);

  k = .5;
  assert(bsearch_left(&k, arr1, 6, sizeof(int), mycmp) == 0);
  assert(bsearch_left(&k, arr2, 5, sizeof(int), mycmp) == 0);

  k = 2.5;
  assert(bsearch_left(&k, arr1, 6, sizeof(int), mycmp) == 2);
  assert(bsearch_left(&k, arr2, 5, sizeof(int), mycmp) == 2);

  k = 3;
  assert(bsearch_left(&k, arr1, 6, sizeof(int), mycmp) == 3);
  assert(bsearch_left(&k, arr2, 5, sizeof(int), mycmp) == 3);

  k = 3.9;
  assert(bsearch_left(&k, arr1, 6, sizeof(int), mycmp) == 3);
  assert(bsearch_left(&k, arr2, 5, sizeof(int), mycmp) == 3);

  k = 4.1;
  assert(bsearch_left(&k, arr1, 6, sizeof(int), mycmp) == 4);
  assert(bsearch_left(&k, arr2, 5, sizeof(int), mycmp) == 4);

  k = 5.1;
  assert(bsearch_left(&k, arr1, 6, sizeof(int), mycmp) == 5);
  assert(bsearch_left(&k, arr2, 5, sizeof(int), mycmp) == 4);

  k = 10;
  assert(bsearch_left(&k, arr1, 6, sizeof(int), mycmp) == 5);
  assert(bsearch_left(&k, arr2, 5, sizeof(int), mycmp) == 4);

  return 0;
}
*/
