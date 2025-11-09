#linerserach 

a = [14, 2, 3, 7, 8, 34, 23]  # Search for 23

def linear_search(arr, target):
    for i in range(len(arr)):
        if arr[i] == target:
            print(f"Found at index {i}")
            return  # Exit the function once found
    print("Not found")

linear_search(a, 23)

    
# binary search
def binary_search(arr, target):
    arr.sort()  # Ensure the list is sorted
    left, right = 0, len(arr) - 1

    while left <= right:
        mid = (left + right) // 2  # Integer division to find the middle index

        if arr[mid] == target:
            print(f'Found {target} at index {mid}')
            return

        if arr[mid] < target:
            left = mid + 1  # Adjust left pointer
        else:
            right = mid - 1  # Adjust right pointer

    print("Not found")


a = [14, 2, 3, 7, 8, 34, 23]
binary_search(a, 23)


def quick_sort(arr, low, high):
    if low < high:
        pivot_index = partition(arr, low, high)
        quick_sort(arr, low, pivot_index - 1)
        quick_sort(arr, pivot_index + 1, high)


def partition(arr, low, high):
    pivot = arr[low]
    i = low + 1
    j = high

    while True:
        while i <= j and arr[i] <= pivot:
            i += 1
        while i <= j and arr[j] >= pivot:
            j -= 1

        if i <= j:
            arr[i], arr[j] = arr[j], arr[i]
        else:
            break

    arr[low], arr[j] = arr[j], arr[low]
    return j


# Example usage:
arr = [2, 1, 6, 3,0, 9, 8]
quick_sort(arr, 0, len(arr) - 1)
print(arr)  # Output: [1, 2, 3, 6, 8, 9]


def merge_sort(arr):
    if len(arr) <= 1:
        return arr

    mid = len(arr) // 2
    l_half = arr[:mid]
    r_half = arr[mid:]

    l_half = merge_sort(l_half)
    r_half = merge_sort(r_half)

    return merge(l_half, r_half)


def merge(left, right):
    new = []
    i, j = 0, 0

    while i < len(left) and j < len(right):
        if left[i] < right[j]:
            new.append(left[i])
            i += 1
        else:
            new.append(right[j])
            j += 1

    new.extend(left[i:])
    new.extend(right[j:])
    return new


# Example usage:
arr = [12, 11, 13, 5, 6, 7]
sorted_arr = merge_sort(arr)
print(sorted_arr)  # Output: [5, 6, 7, 11, 12, 13]

def selection_sort(arr1):
    n = len(arr1)
    for i in range(n - 1):
        mini = i
        for j in range(i + 1, n):
            if arr1[j] < arr1[mini]:
                mini = j
        arr1[i], arr1[mini] = arr1[mini], arr1[i]


# Example Usage
arr1 = [24, 41, 32, 42, 17]
selection_sort(arr1)
print(arr1)  # Output: [17, 24, 32, 41, 42]

def insertion_sort(arr1):
    n = len(arr1)
    for i in range(1, n):
        key = arr1[i]
        j = i - 1

        while j >= 0 and key < arr1[j]:
            arr1[j + 1] = arr1[j]
            j -= 1

        arr1[j + 1] = key


# Example Usage
arr1 = [40, 30, 20, 10]
insertion_sort(arr1)
print("Sorted array is:", arr1)  # Output: Sorted array is: [10, 20, 30, 40]



def heapify(arr, n, i):
    """
    Heapify a subtree rooted at index 'i' in the array 'arr'.

    Parameters:
    arr (list): The list representing the heap.
    n (int): The size of the heap.
    i (int): The index of the root of the subtree to heapify.
    """
    largest = i  # Initialize largest as root
    left = 2 * i + 1  # left = 2*i + 1
    right = 2 * i + 2  # right = 2*i + 2

    # See if left child of root exists and is greater than root
    if left < n and arr[i] < arr[left]:
        largest = left

    # See if right child of root exists and is greater than the largest so far
    if right < n and arr[largest] < arr[right]:
        largest = right

    # Change root, if needed
    if largest != i:
        arr[i], arr[largest] = arr[largest], arr[i]  # swap
        heapify(arr, n, largest)  # Recursively heapify the affected sub-tree


def build_heap(arr):
    """
    Build a max heap from an unordered array.

    Parameters:
    arr (list): The list to be converted into a heap.
    """
    n = len(arr)

    # Index of the last non-leaf node
    start_node = n // 2 - 1

    # Perform reverse level order traversal from last non-leaf node and heapify each node
    for i in range(start_node, -1, -1):
        heapify(arr, n, i)


# Example usage:
if __name__ == '__main__':
    arr = [12, 11, 13, 5, 6, 7]
    build_heap(arr)
    print(arr)  # Output will be the heapified array
