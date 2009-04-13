
def is_blank_line(line):
    for c in line:
        if c != ' ':
            return False
    return True

def first_index(func, list):
    for i in range(len(list)):
        if func(list[i]):
            return i
    return -1

def last_index(func, list):
    for i in range(len(list)-1, 0, -1):
        if func(list[i]):
            return i
    return -1

def normalize_indent(s, indent=""):
    """
    Break up 's' into lines, determine the current indent level, and modify so
    that the minimum indentation is the provided string 'indent'.
    """

    lines = s.split('\n')

    # figure out minimum indent
    def match_count(s, char):
        result = 0
        while result < len(s) and s[result] == char:
            result += 1
        return result

    non_blank_lines = filter(lambda l: not is_blank_line(l), lines)

    minimum_indent = min([match_count(line, ' ') for line in non_blank_lines])

    # replace minimum ident
    def replace_indent(line):
        return indent + line[minimum_indent:]

    return "\n".join(map(replace_indent, lines))

def strip_surrounding_blank_lines(s):
    "Remove blank lines from the beginning and end of 's'"
    lines = s.split('\n')

    first_non_blank = first_index(lambda l: not is_blank_line(l), lines)
    last_non_blank = last_index(lambda l: not is_blank_line(l), lines)

    if last_non_blank == -1:
        last_non_blank = len(lines)

    return "\n".join(lines[first_non_blank:last_non_blank+1])
    
def strip_extra_blank_lines(s):
    "Whereever there are 2 or more consecutive blank lines in s, reduce them to 1"

    result = []

    consecutive_blanks = 0
    for line in s.split('\n'):

        if is_blank_line(line):
            consecutive_blanks += 1
        else:
            consecutive_blanks = 0

        if consecutive_blanks <= 1:
            result.append(line)

    return "\n".join(result)

def load_file(filename):
    f = open(filename)
    contents = f.read()
    f.close()
    return contents

def read_file_as_lines(filename):
    contents = load_file(filename)

    def myFilter(line):
        # remove blank lines
        if line == '': return False

        # remove commented lines
        if line[0] == '#': return False

        return True

    return filter(myFilter, contents.split('\n'))


if __name__ == '__main__':
    test_str1 = """
    This 
      is
         a
            test
    """

    test_str2 = """
         Another
            test
    """

    print normalize_indent(test_str1, '')
    print normalize_indent(test_str1, '   ')
    print normalize_indent(test_str2, '')
    print normalize_indent(test_str2, '  ')
