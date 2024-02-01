import os
import shutil
import glob
import codecs
from generate_common import get_attr_or_param_name_type_value, get_docstring_properties, header_marker, name_marker, desc_marker, return_marker, attr_marker, param_marker


def create_fragment_markdown_file(name, desc, ret, attrs, params):
    file = open('build/' + name + '.md', 'w')

    file.write("**" + name + "(")
    if params != None and len(params) != 0:
        for i in range(len(params)):
            name, value, type = get_attr_or_param_name_type_value(params[i])

            if name == -1:
                return -1

            if i != len(params)-1:
                file.write(name + ", ")
            else:
                file.write(name)
    file.write(")**\r\n")

    file.write('&nbsp;&nbsp;&nbsp;' + desc + "\r\n")
    
    if params != None and len(params) != 0:
        file.write("**Parameters**:" + "\r\n")
        for param in params:
            name, value, type = get_attr_or_param_name_type_value(param)

            if name == -1:
                return -1

            file.write("&nbsp;&nbsp;&nbsp;" + name + " [values: " + value + "]" + "\r\n")

    if attrs != None and len(attrs) != 0:
        file.write("**Attributes**:" + "\r\n")
        for attr in attrs:
            name, value, type = get_attr_or_param_name_type_value(attr)

            if name == -1:
                return -1

            if type.find("function") != -1:
                file.write("&nbsp;&nbsp;&nbsp;" + name + "\r\n")
            else:
                file.write("&nbsp;&nbsp;&nbsp;" + name + " [values: " + value + "]" + "\r\n")
    
    if ret != None: file.write('**Return Value**: ' + ret + "\r\n")

    file.close()


def extract_markdown(file_contents:str):
    # Look for any docstrings that may exist
    last_index = 0
    last_index = file_contents.find(header_marker)

    # While docstrings are being found, make markdown files
    while last_index != -1:
        # Skip past the header that was just found
        last_index += len(header_marker)

        # Find the end of the docstring C comment
        docstring_end = file_contents.find('*/', last_index)
        docstring = file_contents[last_index:docstring_end]

        name = get_docstring_properties(docstring, name_marker, True)[0]
        desc = get_docstring_properties(docstring, desc_marker, False)
        ret = get_docstring_properties(docstring, return_marker, False)
        attrs = get_docstring_properties(docstring, attr_marker, False)
        params = get_docstring_properties(docstring, param_marker, False)
        
        if desc != None:
            desc = desc[0]
        
        if ret != None:
            ret = ret[0]

        if name == -1:
            print("ERROR: No name in docstring (all docstrings are required to have a name): " + docstring)
            return -1
        
        if create_fragment_markdown_file(name, desc, ret, attrs, params) == -1:
            return -1

        # Find the next docstring to make a markdown file out of
        last_index = file_contents.find(header_marker, last_index)
    
    return 0




# Check if build directory exists, if it does, remove it
if os.path.isdir('build'):
    shutil.rmtree('build')

# Create directory that all artifacts will be copied to
os.mkdir('build')

# Get paths from this work directory to each file in 'src'
paths = glob.glob('..\src\**\*', recursive=True)

for path in paths:
    if os.path.isfile(path):
        # Open each file and get contents. Use codecs to ignore decode errors for non ASCII files
        file = codecs.open(path, 'r', 'utf-8', errors='replace')
        file_contents = file.read()
        file.close()

        if extract_markdown(file_contents) == -1:
            print("ERROR occurred in file: '" + path + "'")
            exit()

import generate_resolve_ref_links
