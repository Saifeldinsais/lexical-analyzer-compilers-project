def find_word_in_string(text, word):
    text = text.lower()
    word = word.lower()
    return word in text


"""
ay 7aga ay 7aga ay 7aga ay 7aga ay 7aga'''
"""
'''hello hello hello hello'''; idk = 11
#a7a
def main():
    print("Welcome to the Word Finder!")

    text = input("Enter the string to search in: ")
    word = input("Enter the word you want to find: ")
    value = 88
    min = -99 + value
    value2 = 99.99
    hexadecimal = 0x0FAD
    expo = 3.33e10

    result = find_word_in_string(text, word)

    print(value)
    print('the minimum value:\n' + min)
    print(value2)
    print(hexadecimal)
    print(expo)
    
    if result:
        print(f"'{word}' was found in the given text!")
    else:
        print(f"'{word}' was not found in the given text.")

if __name__ == "__main__":
    main()
