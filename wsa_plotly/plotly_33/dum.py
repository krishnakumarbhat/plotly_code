class SetListComposite:
    def __init__(self):
        self.ls = []  # List to store duplicate
        self.s = set()  # Set to ensure accessing in O(1)

    def __getitem__(self, key):
        return self.ls[key]
    
    def __contains__(self, item):
        return item in self.s

    def append(self, val):
        if val not in self.s:  # Only add if it's not already in the set
            self.ls.append(val)
            self.s.add(val)

    def __len__(self):
        return len(self.ls)

    def __str__(self):
        return str(self.ls)

# Example usage
if __name__ == "__main__":
    composite = SetListComposite()

    # Appending values
    composite.append(1)
    composite.append(2)
    composite.append(3)
    composite.append(2)  # This will not be added again since it's a duplicate

    # Accessing items by index
    print("Item at index 1:", composite[1])  # Output: Item at index 1: 2

    # Checking for membership
    print("Is 2 in composite?", 2 in composite)  # Output: Is 2 in composite? True
    print("Is 4 in composite?", 4 in composite)  # Output: Is 4 in composite? False

    # Printing the composite
    print("Composite list:", composite)  # Output: Composite list: [1, 2, 3]

    # Length of the composite
    print("Length of composite:", len(composite))  # Output: Length of composite: 3
