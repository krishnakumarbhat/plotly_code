# A simple decorator function
def decorator(func):
        print("Before calling the function.")
        func()
        print("After calling the function.")
        return decorator
# Applying the decorator to a function
@decorator

def greet():
    print("Hello, World!")

greet()