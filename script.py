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
        kk = 4

    y= "ay 7aga"


def returnFunctionCall():
    print("ay 7aga")

def myFunction( x, y=10):
    """
    A function docstring
    """
    "hello world"
    x = 5

    inttt,floaaattt,stringgg = 10, 11.5, "String"
    myList = [1, 2, 3]
    for i in myList:
        print(i)
    mySet = {4, 5, 6}
    myDict = {7: "seven", 8: "eight", 9: "nine"}
    myTuple = (0, 1, 2)
    int_brackets = (12-6)
    float_brackets = (12+14)/2 # not handled as a tuple
    ay7agaSet=mySet
    vVvHexa = 0xABC
    VVVVVVVHexa = vVvHexa
    namesList = ["hey", "hello", "hi"]
    newSet = {":", ":(", ":D"} # should be a set not a dictionary
    # do something
    MyClass.greet(vVvHexa, vVvHexa)
    totalInt = x + y
    returnFunctionCall()
    if totalInt > 100:
        return True
    elif totalInt < 100:
        return False
    else:
        return "Result is {}".format(totalInt)
    
