"""
A multi-line docstring at the top of the file
"""
import math

class MyClass:
    # This is a class-level comment
    def __init__(self, name):
        """Constructor docstring"""
        self.name = name

    def greet(self):
        # Greet method
        print(f"Hello, {self.name}!")
        x = 10

def myFunction(x, y=10):
    """
    A function docstring
    """
    "hello world"
    x = 5


    myList = [1, 2, 3]
    for i in myList:
        print(i)
    mySet = {4, 5, 6}
    myDict = {7: "seven", 8: "eight", 9: "nine"}
    myTuple = (0, 1, 2)
    myTuple2 = (12+14)/2 # not handled as a tuple
    ay7aga=mySet
    vVv = 0xABC
    VVVVVVV = vVv
    names = ["hey", "hello", "hi"]
    newSet = {":", ":(", ":D"} # should be a set not a dictionary
    # do something
    total = x + y
    if total > 100:
        return True
    elif total < 100:
        return False
    else:
        return "Result is {}".format(total)
