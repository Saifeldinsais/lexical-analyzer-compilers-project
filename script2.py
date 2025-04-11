# This is a comment
def my_function(param1, param2):
    """This is a docstring"""
    if param1 > 10 and param2 < 20:
        result = param1 + param2
        return result
    else:
        return None

class MyClass:
    def _init_(self, name):
        self.name = name

x = 123
y = 45.67
z = 1_000_000
complex_number = 5j
invalid1 = @var 
invalid2 = _9value
weird_string = "This is a string"
unterminated_string = """Multi-line
but never ends...

multi_ops = a == b and c != d or e <= f
punctuation = (a, b; c: d)
arrow_func = lambda x: x + 1